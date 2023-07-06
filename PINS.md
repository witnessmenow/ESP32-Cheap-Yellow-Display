# Pins

This page talks about the pins on the CYD.

## Broken Out Pins

There are 3no 4P 1.25mm JST connectors on the board,here are the broken out pins.

### P3
|Pin|Use|Note|
|---|---|----|
|GND|||
|IO35||Input only pin, does not have internal pullups available|
|IO22||Also on the **CN1** connector|
|IO21||Is the TFT Backlight, so not really usuable|

### CN1
This is a great canditate for I2C devices

|Pin|Use|Note|
|---|---|----|
|GND|||
|IO22||Also on **P3** connector|
|IO27|||
|3.3V|||

### P1
|Pin|Use|Note|
|---|---|----|
|VIN|||
|IO1(?)|TX|Maybe possible to use as a GPIO?|
|IO3(?)|RX|Maybe possible to use as a GPIO?|
|GND|||


## Buttons

The CYD has two buttons, reset and boot.

|Pin|Use|Note|
|---|---|----|
|IO0|BOOT|Can be used as an input in sketches|

## Speaker

The speaker connector is a 2p 1.25mm JST connector that is connected through the amp, so not usuable as GPIO at the speaker connector

|Pin|Use|Note|
|---|---|----|
|IO26|Conneted to amp|`i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);`|

## RGB LED

If your project requires addtional pins to what is available elsewhere, this might be a good canditate to sacrafice.

Note: LEDs are "active low", meaning HIGH == off, LOW == on

|Pin|Use|Note|
|---|---|----|
|IO4|Red LED||
|IO16|Green LED||
|IO17|Blue LED||

## SD Card

|Pin|Use|Note|
|---|---|----|
|IO5|SD_CS||
|IO18|SD_SCK||
|IO19|SD_MISO||
|IO23|SD_MOSI

## Touch Screen

|Pin|Use|Note|
|---|---|----|
|IO25|XPT2046_CLK||
|IO32|XPT2046_MOSI||
|IO33|XPT2046_CS||
|IO36|XPT2046_IRQ||
|IO39|XPT2046_MISO||

## LDR (Light Sensor)

|Pin|Use|Note|
|---|---|----|
|IO34|||

## Display

|Pin|Use|Note|
|---|---|----|
|IO2|TFT_RS|AKA: TFT_DC|
|IO12|TFT_SDO|AKA: TFT_MISO|
|IO13|TFT_SDI|AKA: TFT_MOSI|
|IO14|TFT_SCK||
|IO15|TFT_CS||
|IO21|TFT_BL|Also on P3 connector, for some reason|
