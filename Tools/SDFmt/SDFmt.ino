/*******************************************************************
    SD card formatting tool for the CYD.

    This tool will format the inserted SD card using the SD card
    library provided by the ESP32 Arduino framework.

    Based on the Examples/3-SDCardTest and Examples/6-LEDTest
    projects in this repository.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    Written by Ian Dinwoodie
    GitHub: https://www.github.com/iandinwoodie
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#include "FS.h"
#include "SD.h"
#include "SPI.h"

// ----------------------------
// SD Reader pins (default VSPI pins)
// ----------------------------
//#define SD_SCK 18
//#define SD_MISO 19
//#define SD_MOSI 23
//#define SD_CS 5

// ----------------------------
// LED Pins
// ----------------------------
#define CYD_LED_BLUE 17
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16

// ----------------------------

enum Status {
  READY,
  RUNNING,
  ERROR,
  SUCCESS
};

SPIClass* spi = nullptr;

Status formatSD() {
  Serial.println("Formatting SD card ...");

  bool format_if_empty = true; // This is how we format the SD card.
  if (!SD.begin(SS, *spi, 80000000, "/sd", 5U, format_if_empty)) {
    Serial.println("Error: Failed to format card.");
    return ERROR;
  } else {
    Serial.println("Success: Card formatted.");
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("Error: No SD card attached.");
    return ERROR;
  }

  Serial.print("SD card type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD card size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  return SUCCESS;
}

void updateLED(Status status) {
  switch (status) {
    case RUNNING: // Set LED to blue.
      digitalWrite(CYD_LED_RED, HIGH);
      digitalWrite(CYD_LED_GREEN, HIGH);
      digitalWrite(CYD_LED_BLUE, LOW);
      break;
    case ERROR: // Set LED to red.
      digitalWrite(CYD_LED_RED, LOW);
      digitalWrite(CYD_LED_GREEN, HIGH);
      digitalWrite(CYD_LED_BLUE, HIGH);
      break;
    case SUCCESS: // Set LED to green.
      digitalWrite(CYD_LED_RED, LOW);
      digitalWrite(CYD_LED_GREEN, LOW);
      digitalWrite(CYD_LED_BLUE, HIGH);
      break;
    default: // Set LED off.
      digitalWrite(CYD_LED_RED, HIGH);
      digitalWrite(CYD_LED_GREEN, HIGH);
      digitalWrite(CYD_LED_BLUE, HIGH);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  spi = new SPIClass(VSPI);

  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);

  updateLED(READY);
}

void loop() {
  static bool complete = false;
  if (!complete) {
    updateLED(RUNNING);
    Status status = formatSD();
    updateLED(status);
    complete = true;
  }
}

