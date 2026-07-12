// ESP32 VM Switcher v2 para el homelab de Chema.
// Hardware: LCDWIKI E32R40T (ESP32 + 4.0" 320x480 ST7796 + XPT2046 resistive touch).
// Backend:  http://<API_SERVER>:8088 (vm-switcher-api, see the backend/ folder), endpoint /machines.
//
// Diferencias vs v1:
//   * Lista DINÁMICA con todas las CTs+VMs visibles (excluye la del backend).
//   * Tap corto = seleccionar; ENC/APG en el footer encienden/apagan la seleccionada.
//   * Long-press (~900 ms) sobre una VM del grupo iGPU (102/103/105/107) = switch
//     (apaga la activa del grupo + arranca esta). Compatible con v1.
//   * Scroll de la lista con flechas ^/v del footer.
//   * OTA habilitado (ArduinoOTA), por si el panel está donde no llega el USB.

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include "secrets.h"

// ---------- TFT ----------
TFT_eSPI tft = TFT_eSPI();
uint16_t calData[5] = {354, 3526, 314, 3491, 2};   // v1 girada 180°: flag 4->2 (invierte X en vez de Y)

// ---------- Layout (rotation 0: 320 ancho × 480 alto) — mosaico 2×4 ----------
#define SCR_W      320
#define SCR_H      480
#define HEADER_H   55
#define FOOTER_H   65
#define LIST_X     8
#define LIST_Y     (HEADER_H + 5)
#define LIST_H     (SCR_H - HEADER_H - FOOTER_H - 10)
#define COLS       2
#define CELL_GAP   8
#define CELL_W     ((SCR_W - 2 * LIST_X - CELL_GAP) / COLS)   // 148
#define CELL_H     80
#define ROWS_VISIBLE (LIST_H / CELL_H)                         // 4
#define CELLS_PER_PAGE (COLS * ROWS_VISIBLE)                   // 8

// footer buttons (y=415..475)
#define FOOTER_Y   (SCR_H - FOOTER_H + 5)
#define BTN_H      55
struct FBtn { int16_t x, w; const char* label; };
FBtn btnUp   = { 10,  56, "^"   };
FBtn btnDown = { 72,  56, "v"   };
FBtn btnOn   = {136,  82, "ENC" };
FBtn btnOff  = {224,  86, "APG" };

// ---------- Data model ----------
#define MAX_MACHINES 24

struct Machine {
    int    id;
    char   kind[3];     // "CT" / "VM"
    char   label[16];   // opcional (LABELS del backend) o el name truncado
    char   name[24];
    char   status[12];  // "running" / "stopped" / "paused" / ...
    bool   igpu_group;
};

Machine machines[MAX_MACHINES];
int     nMachines   = 0;
int     activeIgpu  = -1;
int     selectedId  = -1;
int     scrollOffset = 0;

String  statusLine = "Iniciando...";
bool    needsRedraw = true;

// ---------- Forward decls ----------
void connectWiFi();
void setupOTA();
bool pollMachines();
bool doStart(int vmid);
bool doStop(int vmid);
bool doSwitch(int vmid);
void handleTouch();
void drawFullUI();
void drawHeader();
void drawList();
void drawCell(int col, int row, const Machine& m);
void drawFooter();
void drawStatus();
int  findIdx(int vmid);
void clampScroll();
const Machine* selectedMachine();

// ---------- WiFi / OTA ----------
void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // sin modem-sleep (rompe RTT/ARP en LAN)
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    statusLine = "WiFi conectando...";
    drawStatus();
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 60) {
        delay(500);
        retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        statusLine = String("WiFi OK ") + WiFi.localIP().toString();
    } else {
        statusLine = "WiFi FAIL";
    }
    drawStatus();
}

void setupOTA() {
    ArduinoOTA.setHostname("esp32-vm-switcher");
#ifdef OTA_PASSWORD
    ArduinoOTA.setPassword(OTA_PASSWORD);
#endif
    ArduinoOTA.onStart([]() { statusLine = "OTA inicio..."; drawStatus(); });
    ArduinoOTA.onEnd([]()   { statusLine = "OTA done";     drawStatus(); });
    ArduinoOTA.onError([](ota_error_t e) {
        statusLine = String("OTA err ") + (int)e;
        drawStatus();
    });
    ArduinoOTA.begin();
}

// ---------- HTTP ----------
String apiUrl(const String& path) {
    return String("http://") + API_HOST + ":" + API_PORT + path;
}

bool pollMachines() {
    if (WiFi.status() != WL_CONNECTED) { statusLine = "WiFi caida"; return false; }
    HTTPClient http;
    http.setTimeout(5000);
    http.begin(apiUrl("/machines"));
    http.addHeader("X-API-Key", API_TOKEN);
    int code = http.GET();
    if (code != 200) {
        statusLine = String("API err ") + code;
        http.end();
        return false;
    }
    String body = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) { statusLine = "JSON err"; return false; }

    int prevActive = activeIgpu;
    activeIgpu = doc["active_igpu"].is<int>() ? doc["active_igpu"].as<int>() : -1;

    int n = 0;
    JsonArray arr = doc["machines"].as<JsonArray>();
    for (JsonObject m : arr) {
        if (n >= MAX_MACHINES) break;
        machines[n].id = m["id"].as<int>();
        const char* k  = m["kind"].as<const char*>();   if (!k)  k  = "?";
        const char* nm = m["name"].as<const char*>();   if (!nm) nm = "?";
        const char* lb = m["label"].is<const char*>() ? m["label"].as<const char*>() : nullptr;
        const char* st = m["status"].as<const char*>(); if (!st) st = "?";
        strlcpy(machines[n].kind,   k,  sizeof(machines[n].kind));
        strlcpy(machines[n].name,   nm, sizeof(machines[n].name));
        strlcpy(machines[n].label,  lb ? lb : nm, sizeof(machines[n].label));
        strlcpy(machines[n].status, st, sizeof(machines[n].status));
        machines[n].igpu_group = m["igpu_group"].as<bool>();
        n++;
    }
    bool changed = (n != nMachines) || (activeIgpu != prevActive);
    nMachines = n;
    clampScroll();
    if (selectedId > 0 && findIdx(selectedId) < 0) selectedId = -1;
    needsRedraw = needsRedraw || changed;
    return true;
}

bool _postSimple(const String& path, const String& okLabel, int vmid) {
    statusLine = okLabel + " " + vmid + "...";
    drawStatus();
    HTTPClient http;
    http.setTimeout(10000);
    http.begin(apiUrl(path));
    http.addHeader("X-API-Key", API_TOKEN);
    http.addHeader("Content-Length", "0");
    int code = http.POST("");
    statusLine = (code == 200) ? (okLabel + " " + vmid + " OK")
                               : (okLabel + " err " + code);
    http.end();
    drawStatus();
    return code == 200;
}

bool doStart(int vmid)  { return _postSimple(String("/start/")  + vmid, "ENC",    vmid); }
bool doStop(int vmid)   { return _postSimple(String("/stop/")   + vmid, "APG",    vmid); }
bool doSwitch(int vmid) { return _postSimple(String("/switch/") + vmid, "Switch", vmid); }

// ---------- Helpers ----------
int findIdx(int vmid) {
    for (int i = 0; i < nMachines; i++) if (machines[i].id == vmid) return i;
    return -1;
}

const Machine* selectedMachine() {
    int i = findIdx(selectedId);
    return (i >= 0) ? &machines[i] : nullptr;
}

void clampScroll() {
    // scrollOffset cuenta FILAS de mosaico (0 = primera fila visible arriba)
    int totalRows = (nMachines + COLS - 1) / COLS;
    int maxOff = max(0, totalRows - ROWS_VISIBLE);
    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > maxOff) scrollOffset = maxOff;
}

// ---------- Touch ----------
unsigned long lastTouchMs = 0;

static bool inRect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

void handleTouch() {
    uint16_t sx, sy;
    if (!tft.getTouch(&sx, &sy, 600)) return;
    if (millis() - lastTouchMs < 250) return;
    lastTouchMs = millis();

    // ----- Footer buttons -----
    if (sy >= FOOTER_Y && sy < FOOTER_Y + BTN_H) {
        if (inRect(sx, sy, btnUp.x,   FOOTER_Y, btnUp.w,   BTN_H)) {
            scrollOffset--; clampScroll(); needsRedraw = true; drawList(); return;
        }
        if (inRect(sx, sy, btnDown.x, FOOTER_Y, btnDown.w, BTN_H)) {
            scrollOffset++; clampScroll(); needsRedraw = true; drawList(); return;
        }
        if (inRect(sx, sy, btnOn.x,   FOOTER_Y, btnOn.w,   BTN_H)) {
            const Machine* m = selectedMachine();
            if (!m) { statusLine = "Selecciona fila"; drawStatus(); return; }
            if (String(m->status) == "running") { statusLine = "Ya encendida"; drawStatus(); return; }
            doStart(m->id);
            uint16_t dx, dy; while (tft.getTouch(&dx, &dy, 100)) delay(30);
            return;
        }
        if (inRect(sx, sy, btnOff.x,  FOOTER_Y, btnOff.w,  BTN_H)) {
            const Machine* m = selectedMachine();
            if (!m) { statusLine = "Selecciona fila"; drawStatus(); return; }
            if (String(m->status) != "running") { statusLine = "Ya apagada"; drawStatus(); return; }
            doStop(m->id);
            uint16_t dx, dy; while (tft.getTouch(&dx, &dy, 100)) delay(30);
            return;
        }
    }

    // ----- Grid cells -----
    if (sy >= LIST_Y && sy < LIST_Y + LIST_H) {
        int row = (sy - LIST_Y) / CELL_H;
        // col: detectar zonas izquierda/derecha (con gap entre ellas, descartar el centro)
        int col = -1;
        if (sx >= LIST_X && sx < LIST_X + CELL_W) col = 0;
        else if (sx >= LIST_X + CELL_W + CELL_GAP && sx < LIST_X + 2 * CELL_W + CELL_GAP) col = 1;
        if (col < 0 || row < 0 || row >= ROWS_VISIBLE) return;
        int idx = (scrollOffset + row) * COLS + col;
        if (idx >= 0 && idx < nMachines) {
            unsigned long t0 = millis();
            uint16_t dx, dy;
            bool longPress = false;
            while (tft.getTouch(&dx, &dy, 100)) {
                if (millis() - t0 > 900) { longPress = true; break; }
                delay(20);
            }
            selectedId = machines[idx].id;
            if (longPress && machines[idx].igpu_group) {
                doSwitch(machines[idx].id);
                while (tft.getTouch(&dx, &dy, 100)) delay(30);
            } else {
                statusLine = String("Sel ") + machines[idx].kind + " " + machines[idx].id;
            }
            needsRedraw = true;
        }
    }
}

// ---------- Drawing ----------
#define COL_BG      TFT_BLACK
#define COL_FG      TFT_WHITE
#define COL_DIM     tft.color565(120, 130, 145)
#define COL_ROW_BG  tft.color565(20, 25, 35)
#define COL_ROW_SEL tft.color565(45, 60, 90)
#define COL_OK      TFT_GREEN
#define COL_ERR     TFT_RED
#define COL_ACCENT  tft.color565(90, 200, 250)
#define COL_IGPU    tft.color565(255, 180, 60)

void drawHeader() {
    tft.fillRect(0, 0, SCR_W, HEADER_H, COL_BG);
    tft.setTextColor(COL_FG, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(8, 6);
    tft.print("Homelab Switcher");

    tft.setTextSize(1);
    tft.setCursor(8, 30);
    tft.setTextColor(COL_DIM, COL_BG);
    if (WiFi.status() == WL_CONNECTED) {
        tft.printf("%s  RSSI %d", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        tft.setTextColor(COL_ERR, COL_BG);
        tft.print("WiFi off");
    }

    tft.setTextSize(2);
    tft.setTextColor(activeIgpu > 0 ? COL_IGPU : COL_DIM, COL_BG);
    char buf[24];
    if (activeIgpu > 0) snprintf(buf, sizeof(buf), "iGPU:%d", activeIgpu);
    else                snprintf(buf, sizeof(buf), "iGPU:-");
    int16_t tw = strlen(buf) * 12;
    tft.setCursor(SCR_W - tw - 8, 8);
    tft.print(buf);

    tft.drawFastHLine(0, HEADER_H - 1, SCR_W, COL_DIM);
}

void drawCell(int col, int row, const Machine& m) {
    int x = LIST_X + col * (CELL_W + CELL_GAP);
    int y = LIST_Y + row * CELL_H;
    int w = CELL_W;
    int h = CELL_H - 6;     // margen vertical entre filas
    bool selected = (m.id == selectedId);
    bool running  = (String(m.status) == "running");

    uint16_t bg = selected ? COL_ROW_SEL : COL_ROW_BG;
    uint16_t border = selected ? COL_ACCENT : (m.igpu_group ? COL_IGPU : tft.color565(45, 55, 75));

    tft.fillRoundRect(x, y + 2, w, h, 10, bg);
    tft.drawRoundRect(x, y + 2, w, h, 10, border);
    if (selected) tft.drawRoundRect(x + 1, y + 3, w - 2, h - 2, 9, border);

    // Línea 1: kind+id arriba a la izquierda; pip de estado arriba a la derecha
    tft.setTextColor(COL_ACCENT, bg);
    tft.setTextSize(2);
    tft.setCursor(x + 6, y + 10);
    tft.printf("%s %d", m.kind, m.id);

    uint16_t pip = running ? COL_OK : (String(m.status) == "stopped" ? COL_ERR : COL_DIM);
    tft.fillCircle(x + w - 14, y + 18, 7, pip);

    // Línea 2: label (más grande), centrado
    tft.setTextColor(COL_FG, bg);
    tft.setTextSize(2);
    // truncar a lo que cabe (aprox 11 chars con size 2 en cell de 148)
    char lbl[12];
    strncpy(lbl, m.label, sizeof(lbl) - 1);
    lbl[sizeof(lbl) - 1] = 0;
    int16_t lblW = strlen(lbl) * 12;
    int lx = x + max(6, (w - lblW) / 2);
    tft.setCursor(lx, y + 38);
    tft.print(lbl);

    // Línea 3: estado en texto pequeño bajo el label
    tft.setTextSize(1);
    tft.setTextColor(COL_DIM, bg);
    const char* st = running ? "ENCENDIDA" : (String(m.status) == "stopped" ? "apagada" : "?");
    int16_t stW = strlen(st) * 6;
    tft.setCursor(x + (w - stW) / 2, y + h - 14);
    tft.print(st);
}

void drawList() {
    tft.fillRect(0, LIST_Y, SCR_W, LIST_H, COL_BG);
    for (int row = 0; row < ROWS_VISIBLE; row++) {
        for (int col = 0; col < COLS; col++) {
            int idx = (scrollOffset + row) * COLS + col;
            if (idx >= nMachines) continue;
            drawCell(col, row, machines[idx]);
        }
    }
    // Indicador de scroll a la derecha (puntos = filas-pantalla disponibles)
    int totalRows = (nMachines + COLS - 1) / COLS;
    if (totalRows > ROWS_VISIBLE) {
        int pages = (totalRows + ROWS_VISIBLE - 1) / ROWS_VISIBLE;
        int curPage = scrollOffset / ROWS_VISIBLE;
        int dotsX = SCR_W - 4;
        int dotsY = LIST_Y + LIST_H/2 - (pages * 6)/2;
        for (int i = 0; i < pages; i++) {
            tft.fillCircle(dotsX, dotsY + i * 8, 2, (i == curPage) ? COL_ACCENT : COL_DIM);
        }
    }
}

void drawFooterBtn(const FBtn& b, uint16_t bg, uint16_t fg) {
    tft.fillRoundRect(b.x, FOOTER_Y, b.w, BTN_H, 10, bg);
    tft.drawRoundRect(b.x, FOOTER_Y, b.w, BTN_H, 10, fg);
    tft.setTextColor(fg, bg);
    tft.setTextSize(3);
    int16_t tw = strlen(b.label) * 18;
    tft.setCursor(b.x + (b.w - tw) / 2, FOOTER_Y + (BTN_H - 24) / 2);
    tft.print(b.label);
}

void drawFooter() {
    tft.fillRect(0, FOOTER_Y - 5, SCR_W, FOOTER_H, COL_BG);
    tft.drawFastHLine(0, FOOTER_Y - 5, SCR_W, COL_DIM);
    drawFooterBtn(btnUp,   tft.color565(40, 50, 70), COL_ACCENT);
    drawFooterBtn(btnDown, tft.color565(40, 50, 70), COL_ACCENT);
    drawFooterBtn(btnOn,   tft.color565(0,  70, 30), COL_OK);
    drawFooterBtn(btnOff,  tft.color565(80, 20, 20), COL_ERR);
}

void drawStatus() {
    int y = FOOTER_Y - 14;
    tft.fillRect(0, y, SCR_W, 12, COL_BG);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setTextSize(1);
    tft.setCursor(6, y);
    tft.print(statusLine);
}

void drawFullUI() {
    tft.fillScreen(COL_BG);
    drawHeader();
    drawList();
    drawStatus();
    drawFooter();
    needsRedraw = false;
}

// ---------- Setup / loop ----------
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== ESP32 VM Switcher v2 ===");

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(2);   // 180° respecto a v1 (antes 0)
    tft.fillScreen(TFT_BLACK);
    tft.setTouch(calData);

    drawFullUI();
    connectWiFi();
    setupOTA();
    pollMachines();
    if (selectedId < 0 && activeIgpu > 0) selectedId = activeIgpu;
    drawFullUI();
}

unsigned long lastPoll = 0;
void loop() {
    ArduinoOTA.handle();
    handleTouch();
    if (millis() - lastPoll > 3000) {
        bool ok = pollMachines();
        lastPoll = millis();
        if (needsRedraw) drawFullUI();
        else if (!ok)     drawStatus();
    }
    delay(20);
}
