/*******************************************************************
    Testing Connecting buttons to the broken out pins of the CYD

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    JST Pins:
    The Two JST connectors on the side have the following pins broken out

    P3:
      - GND
      - IO35
      - IO22
      - IO21

    CN1:
      - GND
      - IO22
      - IO27
      - 3.3V

    Pin Info:
      Usable Pins:
        - IO27 and IO22 - should be fully usable GPIO pins
        - IO35 - Input only! Also does not support INTERNAL_PULLUP/PULLDOWN, so needs to be externally pulled up/down

      Not Really Usable:
        - IO21 - Connected to the backlight so probably unlikely that you'll be able to use it for something useful

    

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

int buttonArray[] = {35,22,27};
int arraySize = 3;

void setup() {

  Serial.begin(115200);

  // NOTE: 35 needs to be externally pulled up to work!
  for(int i =0; i < arraySize; i++){
    int pinNum = buttonArray[i];
    pinMode(pinNum, INPUT_PULLUP);
  }
  
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
  x = 320 /2;
  y += 16;
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawCentreString("World", x, y, fontSize);

}

void loop() {
  for(int i =0; i < arraySize; i++){
    int pinNum = buttonArray[i];
    if(!digitalRead(pinNum)){
      Serial.print(pinNum);
      Serial.println(" is pressed");
    }
  }
  delay(50);

}
