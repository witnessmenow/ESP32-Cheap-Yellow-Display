// Copy this to secrets.h and fill the values.
// secrets.h is gitignored.
#pragma once

#define WIFI_SSID  "YOUR_WIFI_SSID"
#define WIFI_PASS  "YOUR_WIFI_PASSWORD"

#define API_HOST   "YOUR_API_SERVER_IP"
#define API_PORT   8088
// Must match the API_TOKEN set in the backend .env
#define API_TOKEN  "YOUR_API_TOKEN"

// Opcional: si se define, ArduinoOTA exige password para flashear por WiFi.
// Comentar / dejar sin definir para OTA abierto en la LAN.
// #define OTA_PASSWORD "tu-password"
