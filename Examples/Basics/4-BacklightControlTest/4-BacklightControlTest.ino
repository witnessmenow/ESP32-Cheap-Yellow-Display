/*******************************************************************

    An Example showing how the backlight of the CYD display can be controlled

    This is adapted from the ESP32 example "LEDC Software Fade"
    
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

#define LCD_BACK_LIGHT_PIN 21

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup() {

  Serial.begin(115200);

  // Start the tft display and set it to black
  tft.init();

  // Setting up the LEDC and configuring the Back light pin
  // NOTE: this needs to be done after tft.init()
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
  
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


  // Simple on off control
  ledcAnalogWrite(LEDC_CHANNEL_0, 255); // On full brightness
  delay(1000);
  ledcAnalogWrite(LEDC_CHANNEL_0, 0); // Off
  delay(1000);

  // --------------- Digital Write ----------------
  // NOTE: you could use digitalWrite if you only want to
  // turn the display on or off
  // You would use this instead of any "ledc" code

  //pinMode(LCD_BACK_LIGHT_PIN, OUTPUT); // set pin as output

  // This will turn back light off
  //digitalWrite(LCD_BACK_LIGHT_PIN, HIGH);

  //delay(1000);

  // This will turn the back light on (full brightness)
  //digitalWrite(LCD_BACK_LIGHT_PIN, LOW);
  //-------------------------------------------------
}

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Fade the display between bright and dark
void loop() {
  // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}
