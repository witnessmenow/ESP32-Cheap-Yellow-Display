/*******************************************************************
    Testing using a Nintendo Wii Nunchuck with the CYD

    More info on Nunchucks with Arduino: https://youtu.be/Cl9f1DUbMnc

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    Addtional Hardware Required:
      - Nunchuck adapter
        - My open source one from Oshpark: https://oshpark.com/shared_projects/RcIxSx2D
        - From Aliexpress*: https://s.click.aliexpress.com/e/_AEEtc3 
      - A nintendo Nunchuck
        - Amazon.co.uk Search*: https://amzn.to/3nQrXcE
        - Amazon.com Search*: https://amzn.to/3nRJTUd
        - Aliexpress (Third Party)*: https://s.click.aliexpress.com/e/_AaQbXh

    *= Affiliate Links

    Wiring:
      - Plug wire that came with CYD into the JST connector closest to the Micro SD card slot
      - Connect the wire to the adapter as follows:
          CYD  -> Adapter
          ------------------------------------------
          GND  -> - (AKA GND) - Black wire for me
          3.3V -> + (AKA 3V)  - Red wire for me
          IO22 -> d (AKA SDA) - Blue wire for me
          IO27 -> c (AKA SCL) - Yellow wire for me
          
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
// Standard Libraries
// ----------------------------

#include <Wire.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <NintendoExtensionCtrl.h>
// This library is for interfacing with the Nunchuck

// Can be installed from the library manager
// https://github.com/dmadison/NintendoExtensionCtrl

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();

// Controller connected to pins broken out on JTAG close to RGB LED
Nunchuk nchuk;

#define NUNCHUCK_SDA 22
#define NUNCHUCK_SCL 27

#define PRINT_DELAY 1000


void setup() {

  Serial.begin(115200);

  Wire.begin(NUNCHUCK_SDA, NUNCHUCK_SCL);
  while (!nchuk.connect()) {
    Serial.println("Nunchuk on bus #1 not detected!");
    delay(1000);
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
  x = 320 / 2;
  y += 16;
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawCentreString("World", x, y, fontSize);

}

void loop() {
  Serial.println("----- Nunchuk Test -----"); // Making things easier to read

  boolean success = nchuk.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Controller disconnected!");
    delay(1000);
  }
  else {

    // -------------------
    // Buttons
    // -------------------

    boolean zButton = nchuk.buttonZ();
    boolean cButton = nchuk.buttonC();

    Serial.print("Buttons pressed: ");
    if (zButton) {
      Serial.print("z");
    }

    if (cButton) {
      Serial.print("c");
    }
    Serial.println("");

    // -------------------
    // Joystick
    // -------------------

    // Read the joystick axis (0-255)
    // Note: I havent seen it go near 0 or 255
    // I've seen ranges closer to 30-210
    int joyY = nchuk.joyY();
    int joyX = nchuk.joyX();

    Serial.print("Joystick Value (x,y): ");
    Serial.print(joyX);
    Serial.print(",");
    Serial.println(joyY);

    // -------------------
    // Joystick
    // -------------------


    // Read the accelerometer (0-1023)
    int accelX = nchuk.accelX();
    int accelY = nchuk.accelY();
    int accelZ = nchuk.accelZ();

    Serial.print("Accelerometer Value (x,y,z): ");
    Serial.print(accelX);
    Serial.print(",");
    Serial.print(accelY);
    Serial.print(",");
    Serial.println(accelZ);
  }

  delay(PRINT_DELAY);

}
