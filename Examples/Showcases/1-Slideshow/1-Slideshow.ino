/*******************************************************************
    SD card slide show for the ESP32 Cheap Yellow Display.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    Written by Claus Näveke
    Github: https://github.com/TheNitek
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
// https://github.com/Bodmer/TFT_eSPI

#include <XPT2046_Bitbang.h>
// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "XPT2046_Bitbang_Slim")
// https://github.com/TheNitek/XPT2046_Bitbang_Arduino_Library

#include <SPI.h>

#include <SdFat.h>
// A library for accessing SD cards
//
// Can be installed from the library manager (Search for "SdFat")
// https://github.com/greiman/SdFat

typedef SdBaseFile File; // Avoid compile issues
#include <JPEGDEC.h>
// A library for decoding JPGs
//
// Can be installed from the library manager (Search for "JPEGDEC")
// https://github.com/bitbank2/JPEGDEC


// ----------------------------
// Touch Screen pins
// ----------------------------

// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

XPT2046_Bitbang ts = XPT2046_Bitbang(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);

// Functions to access a file on the SD card
SPIClass sdSpi = SPIClass(VSPI);
SdSpiConfig sdSpiConfig = SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(32), &sdSpi);
SdFat sd;
SdBaseFile root;
SdBaseFile jpgFile;
int16_t currentIndex = 0;
uint16_t fileCount = 0;

uint32_t timer;
int16_t y_offset;
int16_t lastY;

volatile bool buttonPressed = false;

void IRAM_ATTR buttonInt() {
    buttonPressed = true;
}

int JPEGDraw(JPEGDRAW *pDraw) {
  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

void * myOpen(const char *filename, int32_t *size) {
  jpgFile = sd.open(filename);
  *size = jpgFile.size();
  return &jpgFile;
}
void myClose(void *handle) {
  if (jpgFile) jpgFile.close();
}
int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!jpgFile) return 0;
  return jpgFile.read(buffer, length);
}
int32_t mySeek(JPEGFILE *handle, int32_t position) {
  if (!jpgFile) return 0;
  return jpgFile.seek(position);
}

void decodeJpeg(const char *name) {
  jpeg.open(name, myOpen, myClose, myRead, mySeek, JPEGDraw);
  // If the image doesn't fill the screen: make background black (but safe the time if not needed)
  if(jpeg.getWidth() < tft.width() || jpeg.getHeight() < tft.height()) {
    tft.fillScreen(TFT_BLACK);
  }
  jpeg.decode((tft.width()-jpeg.getWidth())/2, (tft.height()-jpeg.getHeight())/2, 0);
  jpeg.close();
}

void loadImage(uint16_t targetIndex) {
  if(targetIndex >= fileCount) {
    // Starting from the beginning again
    targetIndex = 0;
  }

  Serial.print("Loading file no "); Serial.println(targetIndex);

  root.rewind();

  uint16_t index = 0;

  FsFile entry;
  char name[100];
  while (entry.openNext(&root)) {
    if (entry.isDirectory()) {
      entry.close();
      continue;
    }
    entry.getName(name, sizeof(name));
    const int len = strlen(name);
    if (len > 3 && strcasecmp(name + len - 3, "JPG") != 0) {
      entry.close();
      continue;
    }

    if(index < targetIndex) {
      index++;
      entry.close();
      continue;
    }

    Serial.print("File: "); Serial.println(name);

    decodeJpeg(name);

    entry.close();
    return;
  }

  Serial.print("Could not load file no "); Serial.println(targetIndex);
}

// Show error on the display and serial
void error(const char *msg) {
  tft.fillScreen(TFT_BLACK);
  Serial.println();
  tft.setCursor(0, 0);
  tft.println("Could not open /");
  while(true) delay(1000);
}

void setup() {
  Serial.begin(115200);

  // Button for next image
	pinMode(0, INPUT);
  // Register an interrupt for the next button
  attachInterrupt(0, buttonInt, FALLING);

  // disable RGB LED on the back
  pinMode(4, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape
  
  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setSwapBytes(true);

  ts.begin();

  // Initialize SD card
  if(!sd.begin(sdSpiConfig)) {
    // SD error, stop here
    sd.initErrorPrint(&Serial);
    tft.setCursor(0, 0);
    sd.initErrorPrint(&tft);
    while(true) delay(1000);
  }

  Serial.print("FAT type:   "); sd.printFatType(&Serial); Serial.println();

  if(!root.open("/")) {
    error("Could not open /");
  }

  // Count the number of JPGs on the card
  FsFile file;
  char name[100];
  while (file.openNext(&root))
  {
    file.getName(name, sizeof(name));
    const int len = strlen(name);
    if (len > 3 && strcasecmp(name + len - 3, "JPG") == 0) {
      fileCount++;
    }
    file.close();
  }

  if(fileCount == 0) {
    error("No .JPG files found");
  }

  Serial.print("JPGs found: "); Serial.println(fileCount);

  // Show first image
  loadImage(currentIndex);

  timer = millis();
}

void loop() {
  TouchPoint t = ts.getTouch();

  // Display next image after 10 seconds or button press
  if((millis() - timer > 10*1000) || buttonPressed || t.zRaw > 0) {
    Serial.printf("x: %d y: %d xRaw: %d yRaw: %d zRaw: %d\n", t.x, t.y, t.xRaw, t.yRaw, t.zRaw);
    currentIndex++;
    loadImage(currentIndex);
    timer = millis();
    buttonPressed = false;
  }

  if(currentIndex >= fileCount) {
    // Starting from the beginning again
    currentIndex = 0;
  }
  delay(100);
}