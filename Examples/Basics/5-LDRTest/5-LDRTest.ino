/*******************************************************************
    Testing the LDR (light dependant resistor) of the CYD.
    This can in theory be used to test how bright the room is.

    Sensor reading values:

    0 is full brightness, higher values == darker

    NOTE: I find this does not work that reliably and will require
    some addtional work in software to be used to adjust the screen
    brightness.

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

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

#define LDR_PIN 34

unsigned long ldrCheckDue = 0;
int ldrDelay = 1000; // How often to check the LDR reading in miliseconds (1000 = once a second)

TFT_eSPI tft = TFT_eSPI();

void setup() {

  Serial.begin(115200);

  // Increase read sensitivity
  analogSetAttenuation(ADC_0db);

  pinMode(LDR_PIN, INPUT);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape

  drawValueToScreen(0); // Just start with a 0 value on screen

}

void drawValueToScreen(int sensorValue) {
  // Clear screen first
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int x = 320 / 2; // center of display
  int y = 100;
  int fontSize = 4;

  tft.drawCentreString(String(sensorValue), x, y, fontSize);

}

void loop() {
  unsigned long now = millis(); // millis() returns how many milliseconds since the program started running
  if (now > ldrCheckDue) {
    ldrCheckDue = now + ldrDelay;
    int sensorValue = analogRead(LDR_PIN);
    Serial.print("Analog Read: ");
    Serial.println(sensorValue);
    drawValueToScreen(sensorValue);
  }
}
