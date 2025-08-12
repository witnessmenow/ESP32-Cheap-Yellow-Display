#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ===== WiFi Setup =====
struct WiFiCredentials {
  const char* ssid;
  const char* password;
};

WiFiCredentials wifiList[] = {
    {"HomeSSID", "password123"},
    {"MobileHotspot", "hotspotpw"},
    {"OfficeNet", "securepass"}
};

const int wifiCount = sizeof(wifiList) / sizeof(wifiList[0]);

// ===== TFT and Data =====
TFT_eSPI tft = TFT_eSPI();
String questionTitle = "Fetching...";
String difficulty = "Easy";
String description = "";
String questionId = "";

// ===== UI Constants =====
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 320;
const int cardX = 0;
const int cardY = 0;
int descStartY = 0;
const int cardWidth = SCREEN_WIDTH;
int cardHeight = 0;

const int buttonUpPin = 32;
const int buttonDownPin = 33;

unsigned long lastLeetCodeFetch = 0;
unsigned long fetchInterval = 3600000; // 1 hour

std::vector<String> wrappedLines;
int scrollIndex = 0;
const int visibleLines = 22;
unsigned long lastScroll = 0;
unsigned long scrollDelay = 1000;

// Scroll control
bool autoScrollEnabled = true;
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);


// ===== TFT Color Palette (Dark Theme) =====
const uint16_t COLOR_BACKGROUND      = TFT_BLACK;        // 0x0000  pure black backdrop
const uint16_t COLOR_CARD            = TFT_PURPLE;         // 0x000F  dark slate‑blue panels
const uint16_t COLOR_TEXT_PRIMARY    = TFT_WHITE;        // 0xFFFF  crisp primary text
const uint16_t COLOR_TEXT_SECONDARY  = TFT_SKYBLUE;      // 0x867D  soft cyan accent
const uint16_t COLOR_TEXT_MUTED      = TFT_PURPLE;       // 0xC618  subtle muted info
const uint16_t COLOR_EASY            = TFT_GREENYELLOW;  // 0xB7E0  friendly easy‑level green
const uint16_t COLOR_MEDIUM          = TFT_ORANGE;       // 0xFDA0  warm medium‑level orange
const uint16_t COLOR_HARD            = TFT_RED;          // 0xF800  bold hard‑level red



// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Booting...");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BACKGROUND);

  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);

  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);

  connectToWiFi();
  fetchLeetCodeQuestion();
  wrapDescription(description, cardWidth - 10);
  drawLeetCodeCard();
  drawPauseIcon();
}

// ===== Loop =====
void loop() {
  if (ts.tirqTouched() && ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.printf("Touch at x: %d, y: %d\n", p.x, p.y);
    autoScrollEnabled = !autoScrollEnabled;
    drawPauseIcon();
    Serial.print("Auto-scroll toggled: ");
    Serial.println(autoScrollEnabled ? "ON" : "OFF");
    delay(300);
  }

  if (autoScrollEnabled && (millis() - lastScroll > scrollDelay)) {
    if (scrollIndex + visibleLines < wrappedLines.size()) {
      scrollIndex++;
    } else {
      scrollIndex = 0;
    }
    drawDescriptionArea();
    lastScroll = millis();
  }

  if (millis() - lastLeetCodeFetch > fetchInterval) {
    Serial.println("Fetching new question due to interval...");
    fetchLeetCodeQuestion();
    wrapDescription(description, cardWidth - 10);
    drawLeetCodeCard();
    drawPauseIcon();
    lastLeetCodeFetch = millis();
  }
}

// ===== WiFi Connection =====
void connectToWiFi() {
  for (int i = 0; i < wifiCount; i++) {
    WiFi.begin(wifiList[i].ssid, wifiList[i].password);
    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      return;
    }
  }
}

// ===== LeetCode Fetch =====
void fetchLeetCodeQuestion() {
  HTTPClient http;
  http.begin("https://leetcode.com/graphql");
  http.addHeader("Content-Type", "application/json");

  String query = R"rawliteral({
    "query": "query questionOfToday { activeDailyCodingChallengeQuestion { question { questionId title difficulty content questionFrontendId } } }"
  })rawliteral";

  int httpCode = http.POST(query);

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      JsonObject q = doc["data"]["activeDailyCodingChallengeQuestion"]["question"];
      questionTitle = q["title"].as<String>();
      questionId = q["questionFrontendId"].as<String>();
      difficulty = q["difficulty"].as<String>();
      description = sanitizeHtml(q["content"].as<String>());
    } else {
      questionTitle = "JSON Error";
      description = "Parsing failed.";
    }
  } else {
    questionTitle = "API Error";
    description = "Fetch failed.";
  }

  http.end();
}

// ===== HTML Sanitizer =====
String sanitizeHtml(String input) {
  input.replace("&nbsp;", " ");
  input.replace("</p>", "");
  input.replace("<pre>", "");
  input.replace("</pre>", "");
  input.replace("<code>", "");
  input.replace("</code>", "");
  input.replace("&#39;", "'");
  input.replace("&quot;", "\"");

  String output = "";
  bool inTag = false;
  for (unsigned int i = 0; i < input.length(); i++) {
    if (input[i] == '<') {
      inTag = true;
      continue;
    }
    if (input[i] == '>') {
      inTag = false;
      continue;
    }
    if (!inTag) {
      output += input[i];
    }
  }

  output.replace("&lt;", "<");
  output.replace("&gt;", ">");
  output.replace("\n\n", "\n");
  output = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" + output;
  return output;
}

// ===== Word Wrapping =====
void wrapDescription(String text, int maxWidth) {
  wrappedLines.clear();
  String currentLine = "";
  String word = "";

  for (int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == ' ' || c == '\n') {
      int lineWidth = (currentLine + word).length() * 6;
      if (lineWidth > maxWidth) {
        wrappedLines.push_back(currentLine);
        currentLine = word + " ";
      } else {
        currentLine += word + " ";
      }
      word = "";
      if (c == '\n') {
        wrappedLines.push_back(currentLine);
        currentLine = "";
      }
    } else {
      word += c;
    }
  }

  if (word.length() > 0) currentLine += word;
  if (currentLine.length() > 0) wrappedLines.push_back(currentLine);
}

// ===== Card Renderer =====
void drawLeetCodeCard() {
  tft.fillScreen(COLOR_BACKGROUND);

  int titleLines = (questionTitle.length() / 25);
  int titleHeight = 20 + (titleLines) * 16;
  descStartY = cardY + titleHeight;
  cardHeight = titleHeight + visibleLines * 12 + 50;

  tft.fillRoundRect(cardX, cardY, cardWidth, cardHeight, 8, COLOR_CARD);

  uint16_t diffColor = COLOR_EASY;
  if (difficulty == "Medium") diffColor = COLOR_MEDIUM;
  else if (difficulty == "Hard") diffColor = COLOR_HARD;
  tft.fillRect(cardX, cardY, 5, cardHeight, diffColor);

  tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_TEXT_MUTED);
  tft.setTextSize(1.7);
  tft.setCursor(cardX + 10, cardY + 10);
  tft.print("#" + questionId);

  wrapText(questionTitle, cardX + 10, cardY + 25, cardWidth - 10, 16, COLOR_TEXT_PRIMARY);

  drawDescriptionArea();
}

// ===== Description Area =====
void drawDescriptionArea() {
  int startY = descStartY + 16;
  int descHeight = visibleLines * 12 + 5;
  tft.fillRect(cardX + 5, startY, cardWidth - 10, descHeight, COLOR_BACKGROUND);

  tft.setTextColor(COLOR_TEXT_SECONDARY);
  tft.setTextSize(1);
  int y = startY;

  for (int i = scrollIndex; i < scrollIndex + visibleLines && i < wrappedLines.size(); i++) {
    tft.setCursor(cardX + 10, y);
    tft.print(wrappedLines[i]);
    y += 12;
  }
}

// ===== Text Wrapper =====
void wrapText(String text, int x, int y, int width, int lineHeight, uint16_t color) {
  tft.setTextColor(color);
  tft.setTextSize(1);
  int currentX = x;
  int currentY = y;
  String currentWord = "";

  for (int i = 0; i < text.length(); i++) {
    currentWord += text[i];
    if (text[i] == ' ' || i == text.length() - 1) {
      int wordWidth = currentWord.length() * 6;
      if (currentX + wordWidth > x + width) {
        currentX = x;
        currentY += lineHeight;
      }
      tft.setCursor(currentX, currentY);
      tft.print(currentWord);
      currentX += wordWidth + 2;
      currentWord = "";
    }
  }
}

// ===== Pause Icon =====
void drawPauseIcon() {
  int iconSize = 12;
  int barWidth = 3;
  int spacing = 4;
  int x = SCREEN_WIDTH - iconSize - 5;
  int y = 5;

  if (!autoScrollEnabled) {
    tft.fillRect(x, y, barWidth, iconSize, COLOR_TEXT_SECONDARY);
    tft.fillRect(x + barWidth + spacing, y, barWidth, iconSize, COLOR_TEXT_SECONDARY);
  } else {
    tft.fillRect(x, y, barWidth * 2 + spacing, iconSize, COLOR_CARD);
  }
}
