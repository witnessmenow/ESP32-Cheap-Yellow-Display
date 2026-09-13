// TFT_eSPI configuration for LCDWIKI E32R40T board
// Source: https://www.lcdwiki.com/4.0inch_ESP32-32E_Display
//
// Display: 4.0" 320x480 ST7796S, SPI mode
// Touch:   XPT2046 resistive, sharing SPI bus with display
// MCU:     ESP32-WROOM-32E, no PSRAM, CH340C USB-serial
#pragma once

#define ST7796_DRIVER

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// Display SPI pins (shared with touch)
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1     // tied to ESP32 EN line
#define TFT_BL   27
#define TFT_BACKLIGHT_ON HIGH

// Touch controller chip-select (XPT2046)
#define TOUCH_CS 33

// Fonts to embed
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI clocks
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
