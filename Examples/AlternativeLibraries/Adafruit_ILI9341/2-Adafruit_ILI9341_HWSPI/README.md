# 2-Adafruit_ILI9341_HWSPI

This is adapted from the speed test sketch provided as an example in the TFT_eSPI library, which was itself adapted from the Adafruit speed test sketch.

The original Adafruit test code uses software SPI and that is very slow. This code shows how to use hardware SPI instead. Using hardware SPI the Arduino library is about 50% slower than TFT_eSPI, although for some reason drawing lines that are not horizontal or vertical is about a factor of 4 slower.