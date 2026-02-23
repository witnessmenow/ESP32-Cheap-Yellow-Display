/*******************************************************************
    A touch screen test for capacitive version of the ESP32 Cheap
    Yellow Display.
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

// Pins to use for touch on ESP32 LCD CYD with capacitive GT911 touch controller
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <bb_captouch.h>
// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "bb_captouch")

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

// ----------------------------

BBCapTouch bbct;

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape

  // Set up the touch screen library
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

  int x = 320 / 2; // center of display
  int y = 100;
  int fontSize = 2;
  tft.drawCentreString("Touch Screen to Start", x, y, fontSize);
}

void printTouchToSerial(TOUCHINFO ti) {
  Serial.print("x = ");
  Serial.print(ti.x[0]);
  Serial.print(", y = ");
  Serial.print(ti.y[0]);
  Serial.println();
}

void printTouchToDisplay(TOUCHINFO ti) {

  // Clear screen first
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int x = 320 / 2; // center of display
  int y = 100;
  int fontSize = 2;

  String temp = "X = " + String(ti.x[0]);
  tft.drawCentreString(temp, x, y, fontSize);

  y += 16;
  temp = "y = " + String(ti.y[0]);
  tft.drawCentreString(temp, x, y, fontSize);
}

void loop() {

  TOUCHINFO ti;

  if (bbct.getSamples(&ti)) {
    printTouchToSerial(ti);
    printTouchToDisplay(ti);
  }

  delay(100);
}
