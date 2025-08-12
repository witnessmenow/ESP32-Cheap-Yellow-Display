# 📘 LeetCode Daily Question Viewer (ESP32 + TFT Display)

A minimal ESP32 project that fetches and displays the **LeetCode Daily Challenge** on a 240x320 "cheap yellow" TFT display with touch input.

---

## 🔧 Features

- 🧠 Shows LeetCode daily question title, ID, and description
- 🎨 Difficulty color strip: Green (Easy), Orange (Medium), Red (Hard)
- 📜 Word-wrapped description with clean layout
- 🔁 Auto-scroll for long descriptions
- 👆 Tap screen to toggle auto-scroll on/off
- 🔄 Fetches a new question every hour (no auth needed)

---

## 🧰 Hardware

- ESP32 Dev Board  
- 2.4" TFT Display (ILI9341 controller, 240x320, aka “cheap yellow”)
- XPT2046 Touch Controller (SPI)
- Wi-Fi

---

## 📦 Dependencies

Install these libraries via Arduino Library Manager:

- `TFT_eSPI`
- `XPT2046_Touchscreen`
- `ArduinoJson`
- `HTTPClient`

---

## ⚙️ Pin Configuration

```text
TFT:
  - CS   = default (set via TFT_eSPI config)
  - DC   = "
  - RST  = "
  - MOSI = 23
  - MISO = 19
  - SCLK = 18

Touch (XPT2046):
  - CS   = GPIO 33
  - IRQ  = GPIO 36
  - MOSI = GPIO 32
  - MISO = GPIO 39
  - SCLK = GPIO 25
```

---

## 🚀 Getting Started

1. Clone this repo and open in Arduino IDE or PlatformIO.
2. Update the `WiFiCredentials` array with your network SSIDs and passwords.
3. Flash the code to your ESP32.
4. Watch the daily question render on screen.
5. Tap to toggle scroll!
