#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>  
#include "middleFont.h"
#include "hugeFatFont.h"
#include "fatFont.h"
#include "smallFont.h"

#define LCD_BACK_LIGHT_PIN 21
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
//Adjust to your touch screen values
const uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3800, touchScreenMinimumY = 250,touchScreenMaximumY = 3850;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite s1 = TFT_eSprite(&tft);
TFT_eSprite s2 = TFT_eSprite(&tft);
TFT_eSprite s3 = TFT_eSprite(&tft);
TFT_eSprite s4 = TFT_eSprite(&tft);
TFT_eSprite spr = TFT_eSprite(&tft);

int screen_width = 240; // 240
int screen_height = 320; //536

int deb=0;
int chosen=0;  // chosen shape , background, text, circle
unsigned short grays[24];
unsigned short onOffColo[2]={TFT_RED,TFT_GREEN};
unsigned short tmpColors[3];
String lbl[3]={"B","T","S"};

int slider[3][3];// each slider value 0-400 becouse touch region is 400px
int value[3][3]={{20,20,20},
                 {200,230,200},
                 {50,60,150}};  // variable taht stores value for each slider 0-255
int brightness=120;

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup()
{
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(0);
    // Start the tft display and set it to black
  tft.init();
  // Setting up the LEDC and configuring the Back light pin
  // NOTE: this needs to be done after tft.init()
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);

  // tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
  // tft.writedata(2);
  // delay(120);
  // tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
  // tft.writedata(1);

  //tft.invertDisplay(1); //If you have a CYD2USB - https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/cyd.md#my-cyd-has-two-usb-ports
  tft.setRotation(0); //This is the display in landscape
  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

    s1.createSprite(60,screen_height-116);
    s2.createSprite(60,screen_height-116);
    s3.createSprite(60,screen_height-116);
    s4.createSprite(60,screen_height-116);
    spr.createSprite(232,110);

     //define level of grays or greys
     int co=240;
     for(int i=0;i<24;i++)
     {grays[i]=tft.color565(co, co, co);
     co=co-10;}

     for(int i=0;i<3;i++)
       {tmpColors[i]=tft.color565(value[i][0], value[i][1], value[i][2]);
       for(int j=0;j<3;j++)
       slider[i][j]=map(value[i][j],0,255,0,screen_height-152);
       }

    draw();
     draw1();
      draw2();
       draw3();
        draw4();

}

void draw()
{
  spr.fillSprite(TFT_BLACK);
  //spr.fillSprite(tft.color565(value[0][0], value[0][1], value[0][2]));
  spr.loadFont(fatFont);
  spr.setTextColor(grays[10],TFT_BLACK);
  spr.drawString("R: ",0,0);
  spr.drawString("G: ",0,30);
  spr.drawString("B: ",0,60);

  spr.setTextColor(grays[5],TFT_BLACK);
  spr.drawString(String(value[chosen][0]),26,0);
  spr.drawString(String(value[chosen][1]),26,30);
  spr.drawString(String(value[chosen][2]),26,60);
  
  spr.fillRect(112,80,120,30,0xCE01);
  spr.setTextColor(TFT_BLACK,0xCE01);
  tmpColors[chosen]=tft.color565(value[chosen][0], value[chosen][1], value[chosen][2]);
 
  spr.drawString("0x"+String(tmpColors[chosen],HEX),130,83);
  spr.unloadFont();
  spr.loadFont(smallFont);
  spr.setTextColor(0xBD41,TFT_BLACK);
  spr.drawString("BCKLIGHT: "+String(brightness),0,92);
  spr.unloadFont();

  spr.fillSmoothRoundRect(100, 0, 132, 74, 4, tmpColors[0],TFT_BLACK);
  spr.fillRoundRect(74, 30, 32, 44, 4, tmpColors[0]);
  spr.loadFont(fatFont);
  spr.setTextColor(tmpColors[1],tmpColors[0]);
  spr.drawString("Hello!",116,4);
  spr.unloadFont();
  spr.loadFont(smallFont);
  spr.drawString("This is Text :)",83,42);
  spr.setTextColor(grays[6],grays[15]);
  spr.fillSmoothRoundRect(74, 0, 22, 26, 2, grays[22],TFT_BLACK);
  spr.drawString(lbl[chosen],80,4);
  spr.unloadFont();
  spr.fillRect(190,4,30,26,tmpColors[2]);
  spr.fillRect(200,36,18,24,tmpColors[2]);
  tft.startWrite();
  tft.setAddrWindow(4, 4, 232, 110);
  tft.pushColors((uint16_t *)spr.getPointer(), 232*110, false);
  tft.endWrite();
}

void draw1()
{
  s1.fillSprite(TFT_BLACK);
  s1.fillSmoothRoundRect(2,2,56,screen_height-130,3,0x18E3,TFT_BLACK);
  drawGraRect(36, screen_height-136,slider[chosen][0], 0xF800, 0xD186,s1);
  tft.startWrite();
  tft.setAddrWindow(0, 116, 60, screen_height-116);
  tft.pushColors((uint16_t *)s1.getPointer(), 60 * (screen_height-116), false);
  tft.endWrite();
}

void draw2()
{
  s2.fillSprite(TFT_BLACK);
  s2.fillSmoothRoundRect(2,2,56,screen_height-130,3,0x18E3,TFT_BLACK);
  drawGraRect(36, screen_height-136,slider[chosen][1], 0x220, 0x2FC4,s2);
  tft.startWrite();
  tft.setAddrWindow(60, 116, 60, screen_height-116);
  tft.pushColors((uint16_t *)s2.getPointer(), 60 * (screen_height-116), false);
  tft.endWrite();
}

void draw3()
{
  s3.fillSprite(TFT_BLACK);
  s3.fillSmoothRoundRect(2,2,56,screen_height-130,3,0x18E3,TFT_BLACK);
  drawGraRect(36, screen_height-136,slider[chosen][2], 0xF7, 0x1D5E,s3);
  tft.startWrite();
  tft.setAddrWindow(120, 116, 60, screen_height-116);
  tft.pushColors((uint16_t *)s3.getPointer(), 60 * (screen_height-116), false);
  tft.endWrite();
}

void draw4()
{
  s4.fillSprite(TFT_BLACK);
  s4.fillSmoothRoundRect(2,2,56,screen_height-130,3,0x18E3,TFT_BLACK);
  drawGraRect(36, screen_height-136,map(brightness,0,255,0,screen_height-152), 0x7B40, 0xF70A,s4);
  tft.startWrite();
  tft.setAddrWindow(180, 116, 60, screen_height-116);
  tft.pushColors((uint16_t *)s4.getPointer(), 60 * (screen_height-116), false);
  tft.endWrite();
}

void drawGraRect(int16_t x, int16_t y, int leng, uint16_t color_start, uint16_t color_end,TFT_eSprite &sprite) {
  int total_height = screen_height - 120 - 18;
  int line_space = 7;
  int count = total_height / line_space;

  for(int i=0;i<count;i++)
  if(i%5==0)
  sprite.drawWedgeLine(4,y-(i*line_space),18,y-(i*line_space),2,2,0x8430);
  else
  sprite.drawWedgeLine(10,y-(i*line_space),14,y-(i*line_space),1,1,0x8430);

  sprite.drawWedgeLine(x+5, y, x+5, 10, 5, 5,0x39E7,TFT_BLACK);
  sprite.drawSpot(x+5, y, 5, color_start);
  for (int16_t i = 0; i <= leng; i++) {
    // Interpolacija boje između početne i završne boje
    uint8_t interpolation = map(i, 0, leng, 0, 255);
    uint8_t red = map(interpolation, 0, 255, (color_start >> 8) & 0xF8, (color_end >> 8) & 0xF8);
    uint8_t green = map(interpolation, 0, 255, (color_start >> 3) & 0xFC, (color_end >> 3) & 0xFC);
    uint8_t blue = map(interpolation, 0, 255, (color_start << 3) & 0xF8, (color_end << 3) & 0xF8);
    uint16_t color = tft.color565(red, green, blue);

  sprite.fillRect(x,y-i,10,1,color); 
  }
  sprite.fillSmoothCircle(x+5, y-leng, 14,TFT_SILVER, TFT_BLACK);
  sprite.fillSmoothCircle(x+5, y-leng, 7,color_start,TFT_SILVER);
}

void loop()
{
   static int16_t x, y;
   bool touched = ts.tirqTouched() && ts.touched();
   if(touched)
  {
    TS_Point p = ts.getPoint();
    x = map(p.x,touchScreenMinimumX,touchScreenMaximumX,0,screen_width);
    y = map(p.y,touchScreenMinimumY,touchScreenMaximumY,0,screen_height);
    if(y>110){
    int end=map(y,screen_height-20,132,0,screen_height-152);
    if(end>screen_height-152) end=screen_height-152;
    if(end<0) end=0;

    if(x>0 && x<60)
    {slider[chosen][0]=end; value[chosen][0]=map(end,0,screen_height-152,0,255); draw1(); draw();}
     if(x>60 && x<120)
    {slider[chosen][1]=end;value[chosen][1]=map(end,0,screen_height-152,0,255); draw2(); draw();}
     if(x>120 && x<180)
    {slider[chosen][2]=end;value[chosen][2]=map(end,0,screen_height-152,0,255);draw3(); draw();}
     if(x>180 && x<240)
    {brightness=map(end,0,screen_height-152,0,255);draw4(); draw(); ledcAnalogWrite(LEDC_CHANNEL_0, brightness);}
    }

    if(y<70)
    {
      if(deb==0)
      {deb=1; chosen++; if(chosen==3) chosen=0; draw(); draw1(); draw2(); draw3();}
    }
 
  }else deb=0;
}





