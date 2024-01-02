#define PROGNAME  "AutoBrightness"
#ifndef VERSION     // VERSION from platformio.ini
    #define PROGVERSION "V0.1"
#else
    #define PROGVERSION VERSION 
#endif       
#ifndef BUILD_DATE     // Build date from platformio.ini
    #define PROGDATE "2023-12-16"
#else
    #define PROGDATE BUILD_DATE 
#endif   
#define PROGTIME BUILD_TIME 
  
// define that CYD is present
#ifndef CYD
  #define CYD
#endif

 
/**************************************************!
include files	
***************************************************/
#include "Free_Fonts.h"     // Free fonts
#include <Arduino.h>		// arduino standard library
// Include files for CYD
#ifdef CYD
  #include <SPI.h>                  // standard SPI lib
  #include <XPT2046_Touchscreen.h>  // touch screen lib
  #include <TFT_eSPI.h>             // LCD display lib

  // Touch Screen pins: The CYD touch uses some non default SPI pins
  #define XPT2046_IRQ 36
  #define XPT2046_MOSI 32
  #define XPT2046_MISO 39
  #define XPT2046_CLK 25
  #define XPT2046_CS 33

  // CYD Backlight stuff
  #define LCD_BACK_LIGHT_PIN 21     // PIN for backlight control
  #define LEDC_CHANNEL_0     0      // use first channel of 16 channels (started from zero)
  #define LEDC_TIMER_12_BIT  12     // use 12 bit precission for LEDC timer
  #define LEDC_BASE_FREQ     5000   // use 5000 Hz as a LEDC base frequency

  // CYD LDR PIN (Light sensitive resistor)
  #define LDR_PIN 34

  // CYD global vars
  SPIClass mySpi = SPIClass(VSPI);
  XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
  TFT_eSPI tft = TFT_eSPI();

  // Screen size
  #define SCREEN_XSIZE 320
  #define SCREEN_YSIZE 240
  
  /* Definitions for cpolors, brightnes etc.*/
  // colors in 565 rgb format. Color picker: https://chrishewett.com/blog/true-rgb565-colour-picker/
  // another color calculator: http://www.rinkydinkelectronics.com/calc_rgb565.php 

  #define NOTE_DH2 661                          //sound frequency

  #define TFT_GREY 0x5AEB                       // New colour
  #define TFT_DARKBLUE 0x439                    /* Upper dark blue area */
  #define TFT_LIGHTBLUE 0x1E9F                  /* Main light blue area */
  #define TFT_LOWBLUE  0x439                    /* Lower dark blue area */

  #define TFT_BLACK       0x0000      /*   0,   0,   0 */
  #define TFT_NAVY        0x000F      /*   0,   0, 128 */
  #define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
  #define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
  #define TFT_MAROON      0x7800      /* 128,   0,   0 */
  #define TFT_PURPLE      0x780F      /* 128,   0, 128 */
  #define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
  //#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
  #define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
  #define TFT_COALGREY    0x3186      /*50, 50, 50 */   // lighter: 0x630c /*96, 96, 96 */
  #define TFT_BLUE        0x001F      /*   0,   0, 255 */
  #define TFT_GREEN       0x07E0      /*   0, 255,   0 */
  #define TFT_CYAN        0x07FF      /*   0, 255, 255 */
  #define TFT_RED         0xF800      /* 255,   0,   0 */
  #define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
  #define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
  #define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
  #define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
  #define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
  // #define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
  #define TFT_BROWN       0x9A60      /* 150,  75,   0 */
  #define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
  #define TFT_SILVER      0xC618      /* 192, 192, 192 */
  #define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
  #define TFT_VIOLET      0x915C      /* 180,  46, 226 */

// global variables
  int LCD_BRIGHTNESS = 100;   // LCD brightnness value 0..255
  int LCD_TEXTSIZE = 2;
  int LCD_BACKLIGHT_MODE = 2; // 0=manual, 1= linear, 2 = logarithmic

  uint16_t bgndColor = TFT_DARKGREEN;
  uint16_t fgndColor = TFT_WHITE;

  // coordinates for blinking circle in permille
  int blinkMarkerX = 293; 
  int blinkMarkerY = 35;
  int blinkMarkerR = 10;
#endif // CYD 
 
/**************************************************!
  @brief    Arduino like analogWrite for backlight brightness LED control
  @details  
  @param  channel: previously defined
  @param  value: brightness value, in relation to valueMax. 0..255 if valueMax = 255 = default. has to be between 0 and valueMax
  @param  valueMax: max. brightness vale
  @param  
  @return   void
***************************************************/

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}
 

   /**************************************************!
  @brief    setup function
  @details  
  @param  	none
  @return   void
  ***************************************************/
void setup() 
{
  // initialize CYD functions
  #ifdef CYD
    // Start the SPI for the touch screen and init the TS library
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);
    ts.setRotation(1);

    // Start the tft display and set it to black
    tft.init();
    tft.setRotation(1); //This is the display in landscape

    // Setting up the LEDC and configuring the Back light pin
    // NOTE: this needs to be done after tft.init()
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);

    // set the brightness on LEDC channel 0
    ledcAnalogWrite(LEDC_CHANNEL_0, LCD_BRIGHTNESS);

    // setup LDR (light sensitive resistor)  
    analogSetAttenuation(ADC_0db); // Increase read sensitivity
    pinMode(LDR_PIN, INPUT);

    // startup message
    tft.fillScreen(TFT_BLUE);
    int x = SCREEN_XSIZE / 2; // center of display
    int y = 100;
    int fontSize = 4;
    bgndColor = TFT_BLUE;
    tft.setTextColor(fgndColor, bgndColor);
    tft.drawCentreString("LCD Brightness Test", x, y, fontSize);
  #endif // CYD

  // Init USB serial port
  Serial.begin(115200);
  delay(100);

  // delay(2000);
} // setup
 
 #ifdef CYD
 
/**************************************************!
  @brief    check for touch panel interactions and handle them
  @details  .
  @param    none
  @return   void
  ***************************************************/
// helper function: touch parameters to serial and to display
void printTouchToSerial(TS_Point p, int button) 
{
  Serial.print("Pressure = ");
  Serial.print(p.z);
  Serial.print(", x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.print(", btn = ");
  Serial.print(button);
  Serial.println();
}

void printTouchToDisplay(TS_Point p, int button) 
{
  // Clear screen first
  //tft.fillScreen(TFT_BLACK);
  tft.setTextColor( TFT_BLACK, TFT_WHITE);
  tft.setFreeFont(FSS9);

  int x = 60, y = 200;
  int fontSize = 2;

  String temp = "Pressure = " + String(p.z)+"   ";
  tft.drawCentreString(temp, x, y, fontSize);

  x = 140;
  temp = " X = " + String(p.x)+"  ";
  tft.drawCentreString(temp, x, y, fontSize);

  x = 210;
  temp = " Y = " + String(p.y)+ "   ";
  tft.drawCentreString(temp, x, y, fontSize);

    x = 280;
  temp = " Btn = " + String(button)+ "  ";
  tft.drawCentreString(temp, x, y, fontSize);
}

 boolean inTol(int x, int y, int tol)
 {
    if(abs(x-y) < tol)
      return (true);
    else 
      return(false);  
}

void handleTouch()
{
  static long lastScreenSwitchTime=0, actTimeMS;
  static int lastcolor;
  
  if (ts.tirqTouched() && ts.touched()) 
  {
    // identify button hat has been pressed
    int tolerance = 500, button = 0;
    // area delimiter (+/- tolerance) for buttons 0..8
    int limits[9][2]={{600,600}, {2050,600}, {3450,600},
                      {600,1950},{2050,1950},{3450,1950},
                      {600,3350},{2050,3350},{3450,3350}};

    TS_Point p = ts.getPoint(); // get touch info

    // the actual button identification
    button = -1;
    for (int i = 0; i < 9;i++)
    {
      if (inTol(p.x, limits[i][0], tolerance) && inTol(p.y, limits[i][1], tolerance) )
        button = i;

      // debug: rings to show active areas per virtual button
      //tft.fillCircle(limits[i][0]*320/4096,limits[i][1]*240/4096,tolerance*320/4096,TFT_LIGHTGREY);  
    }

    // output touch Info
    printTouchToDisplay(p, button);
    // activity marker circle 
      // draw circle below program version as activity marker
    if (lastcolor ==1){
      lastcolor = 0;
      tft.fillCircle(blinkMarkerX,blinkMarkerY,blinkMarkerR,TFT_YELLOW);
    }
    else {
      lastcolor = 1;
      tft.fillCircle(blinkMarkerX,blinkMarkerY,blinkMarkerR,TFT_RED);
    }
    delay(30);

    actTimeMS = millis();

    // determine actions for "buttons" = sensitive screen areas
    switch(button){
      case 0: // top left
        if (actTimeMS > lastScreenSwitchTime+1000){
          LCD_BACKLIGHT_MODE +=1;
          if(LCD_BACKLIGHT_MODE > 2)
            LCD_BACKLIGHT_MODE = 0;
          lastScreenSwitchTime = actTimeMS;  
        }
      break;
      case 1: // top middle
      break;
      case 2: // top right
        if (actTimeMS > lastScreenSwitchTime+1000){
          if(LCD_BACKLIGHT_MODE == 0)
          {
            LCD_BRIGHTNESS += 50;
            if (LCD_BRIGHTNESS > 255)    // Maximum brightness reached ?
              LCD_BRIGHTNESS = 5;        // If yes, go to lowest value
            
            // set the brightness on LEDC channel 0
            ledcAnalogWrite(LEDC_CHANNEL_0, LCD_BRIGHTNESS);  
          }  
          lastScreenSwitchTime = actTimeMS;
        }  
      break;
      case 3: // middle left
        // prevent multiple clicks
        if (actTimeMS > lastScreenSwitchTime+1000){
         
        }  
      break;
      case 4: // middle middle
        // prevent multiple clicks
        if (actTimeMS > lastScreenSwitchTime+1000){
         
        }  
      break;
      case 5: // middle right
        // prevent multiple clicks
        if (actTimeMS > lastScreenSwitchTime+1000){
          
        }        
      break;
      case 6: // lower left
      break;
      case 7: // lower middle
      break;
      case 8: // lower right
        if (actTimeMS > lastScreenSwitchTime+1000){
          if(LCD_BACKLIGHT_MODE == 0){
            LCD_BRIGHTNESS -= 50;
            if (LCD_BRIGHTNESS < 5)        // Minimum brightness reached ?
              LCD_BRIGHTNESS = 255;        // If yes, back to 255!
            
            ledcAnalogWrite(LEDC_CHANNEL_0, LCD_BRIGHTNESS);     
          }     
          lastScreenSwitchTime = actTimeMS;
        }     
      break;
    }
  }
}


 /**************************************************!
  @brief    check for light level and set lcd backlight accordingly
  @details  .
  @param    int mode: 0: linear dependency between LDR  output and backlight level
  @param              1: logarithmic dependency between LDR  output and backlight level
  @param    int minBrightness: (1) min brightness level for LCD backlight to be set
  @param    int maxBrightness: (255) max brightness level for LCD backlight to be set
  @param    int averageCnt: (50) number of measurements over which to average the measurement value. higher = slower response
  @return   int backlightLevel  Backlight level 0..255
  ***************************************************/
  
// helper function: output brightness control data to screen  
void doBrightnessDebugOutput(int mode, int sensorValue, int backlightValue)
{   
  char printstring[100], modestring[25];

  tft.fillRect(10,0,SCREEN_XSIZE-50,3*18,bgndColor);
  tft.setTextColor(fgndColor, bgndColor);
  tft.setTextSize(1);
  tft.setFreeFont(FSS9); // Sans Serif 9 pt
  switch(mode)
  {
    case 0: strcpy(modestring, " manual       ");break;
    case 1: strcpy(modestring, " linear       ");break;
    case 2: strcpy(modestring, " logarithmic  ");break;
  }

  sprintf(printstring, "Mode:      %d: %s  ", mode, modestring);
  tft.setCursor(10, 16 ); 
  tft.print(printstring); 
  sprintf(printstring, "LDR:        %d  ", sensorValue);
  tft.setCursor(10, 32 ); 
  tft.print(printstring); 
  sprintf(printstring, "Backlight: %d  ", backlightValue);
  tft.setCursor(10, 48 ); 
  tft.print(printstring); 
  sprintf(printstring, "Press here to change mode");
  tft.setCursor(10, 66 ); 
  tft.print(printstring); 

}

 byte handleBrightness(int mode, int minBrightness, int maxBrightness, int averageCnt, bool debugOut)
 {
  int sensorValue;
  static int lastSensorValue = 0, lastBacklightValue = 0, lastMode = 0;
  static int lastSensorRead=0;
  static float backlightValue = 250.0;
  float backlightTarget = 0, backlightDelta = 0;
  float brightnessRange, backlightRange;
  int maxSensorValue = 4095;

  if(millis() > lastSensorRead + 300)
  {
    sensorValue = analogRead(LDR_PIN);    // present value of light sensitive resistor
    lastSensorRead = millis();

    brightnessRange = maxBrightness - minBrightness;
    switch (mode)
    {
      case 0: // manual, do nothing here
        backlightTarget = LCD_BRIGHTNESS;
        backlightValue = LCD_BRIGHTNESS;
        break;
      case 1: // linear
        // target value based on present lightness value, linear 
        backlightTarget = maxBrightness - (float)sensorValue/(float)maxSensorValue * brightnessRange;
        break;
      case 2: // logarithmic
        // target value based on present lightness value, logarithmic 
        float logSensorRange = log(maxSensorValue+1) - log(1); // y range, offset 1 since log not defined at 0   
        backlightTarget = maxBrightness - log(sensorValue+1)/logSensorRange * brightnessRange;
        break;
    } 
    // calc. difference to actual backlight value
    backlightDelta = backlightValue - backlightTarget;
    // increment backlight value by delta/averageCnt. ensures smooth, asymptotic approach to target
    backlightValue -= backlightDelta / averageCnt;
    // ensure that target range from paramters is met
    if (backlightValue < minBrightness) 
      backlightValue= minBrightness;
    if (backlightValue > maxBrightness) 
      backlightValue= maxBrightness;

    LCD_BRIGHTNESS = (int)backlightValue % 256;       // set the global variable for LCD brightness
    
    ledcAnalogWrite(LEDC_CHANNEL_0, LCD_BRIGHTNESS);  // and do actually set the LCD brightness on LEDC channel 0

    // debug output only, to be disabled in actual use
    if(debugOut &&(sensorValue != lastSensorValue || LCD_BRIGHTNESS != lastBacklightValue || mode != lastMode))
    {
      doBrightnessDebugOutput(mode, sensorValue, LCD_BRIGHTNESS);
      // housekeeping for debug output  
      lastSensorValue = sensorValue;
      lastBacklightValue = LCD_BRIGHTNESS;
      lastMode = mode;
    }  
  }
  return(LCD_BRIGHTNESS);
 }

// main handler for backlight control
void doBacklightWork()
{
  // test feature: display value of light sensitive resistor below version no.
  // mode 0=manual, 1= lin, 2= log, min / max brightness, averagecnt
  handleBrightness(LCD_BACKLIGHT_MODE, 1, 255, 10, true); // fast, logarithmic mode
}
#endif // CYD

 /**************************************************!
  @brief    loop: main program loop
  @details  calls worker routines
  @param    none
  @return   void
***************************************************/
void loop()
{
  int lastMillis = 0;
  
  #ifdef CYD
    if(millis()>lastMillis + 1000){
      doBacklightWork();
      lastMillis = millis();
    }  
    handleTouch();            // handle touch interactions
  #endif // CYD  
  delay(50);
} // loop