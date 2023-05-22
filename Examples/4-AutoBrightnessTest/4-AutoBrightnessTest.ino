/*******************************************************************
    WIP of brightness control for the CYD

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();

#define LDR_PIN 34
#define LCD_BACK_LIGHT_PIN 21
unsigned long ldrCheckDue = 0;
int ldrDelay = 100;

// This is the range of values we might expect from the LDR (can be adjusted)
// A low value means the sensor is getting a lot of light
#define MIN_INPUT 0
#define MAX_INPUT 350 // This was the highest number I saw when fully covering the sensor

// This is the minimum and maximum values we will auto set the display to
// if the sensor is bright (low reading), we want the display to be bright (low value)
// (This can be adjusted)
#define MIN_DISPLAY_BRIGHTNESS 0
#define MAX_DISPLAY_BRIGHTNESS 200

// setting PWM properties, can leave as is
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void setup() {

  Serial.begin(115200);

  //Need to manually control the back light
  pinMode(LCD_BACK_LIGHT_PIN, OUTPUT);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int x = 5;
  int y = 10;
  int fontSize = 2;
  tft.drawString("Hello", x, y, fontSize); // Left Aligned
  x = 320 / 2;
  y += 16;
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawCentreString("World", x, y, fontSize);

  delay(1000);

  // This will turn back light off
  digitalWrite(LCD_BACK_LIGHT_PIN, HIGH);

  delay(1000);

  // This will turn the back light on (full brightness)
  digitalWrite(LCD_BACK_LIGHT_PIN, LOW);
}
//
//void loop() {
//  unsigned long now = millis();
//  if (now > ldrCheckDue) {
//    ldrCheckDue = now + ldrDelay;
//    int sensorValue = analogRead(LDR_PIN);
//    Serial.print("Analog Read: ");
//    Serial.println(sensorValue);
//    int brightnessValue = map(sensorValue, MIN_INPUT, MAX_INPUT, MIN_DISPLAY_BRIGHTNESS, MAX_DISPLAY_BRIGHTNESS);
//    Serial.print("brightnessValue: ");
//    Serial.println(brightnessValue);
//    Serial.print("----------------- ");
//    ledcWrite(ledChannel, brightnessValue);
//
//  }
//
//}

void loop() {
}
