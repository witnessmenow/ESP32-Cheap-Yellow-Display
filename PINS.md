# Pins

This page talks about the pins on the CYD.

## Connector types

|Connector|Type    |Note                   |
|---      |---     |----                   |
|[**P1**](#p1)  |4P 1.25mm JST|Serial     |
|[**P3**](#p3)  |4P 1.25mm JST|GPIO       |
|[**P4**](#p4)  |2P 1.25mm JST|Speaker    |
|[**CN1**](#cn1)|4P 1.25mm JST|GPIO (I2C) |

## What pins are available on the CYD?

There are 3 easily accessible GPIO pins

|Pin|Location|Note|
|---|---|----|
|IO35|**P3** JST connector|Input only pin, no internal pull-ups available|
|IO22|**P3** and **CN1** JST connector||
|IO27|**CN1** JST connector||

If you need more than that, you need to start taking them from something else. An SD Card sniffer like mentioned in the [Add-ons](/ADDONS.md) is probably the next easiest.

After that you're probably de-soldering something!

## Broken Out Pins

There are three 4P 1.25mm JST connectors on the board

### P3
|Pin|Use|Note|
|---|---|----|
|GND|||
|IO35||Input only pin, no internal pull-ups available|
|IO22||Also on the **CN1** connector|
|IO21||Used for the TFT Backlight, so not really usable|

### CN1
This is a great candidate for I2C devices

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

The speaker connector is a 2P 1.25mm JST connector that is connected to the amplifier, so not usable as GPIO at the speaker connector

|Pin|Use|Note|
|---|---|----|
|IO26|Connected to amp|`i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);`|

## RGB LED

If your project requires additional pins to what is available elsewhere, this might be a good candidate to sacrifice.

Note: LEDs are "active low", meaning HIGH == off, LOW == on

|Pin|Use|Note|
|---|---|----|
|IO4|Red LED||
|IO16|Green LED||
|IO17|Blue LED||

## SD Card
Uses the VSPI
Pin names are predefined in SPI.h

|Pin|Use|Note|
|---|---|----|
|IO5|SS||
|IO18|SCK||
|IO19|MISO||
|IO23|MOSI||

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
Uses the HSPI

|Pin|Use|Note|
|---|---|----|
|IO2|TFT_RS|AKA: TFT_DC|
|IO12|TFT_SDO|AKA: TFT_MISO|
|IO13|TFT_SDI|AKA: TFT_MOSI|
|IO14|TFT_SCK||
|IO15|TFT_CS||
|IO21|TFT_BL|Also on P3 connector, for some reason|

## Test points
|Pad|Use|Note|
|---|---|----|
|S1|GND|near USB-SERIAL|
|S2|3.3v|for ESP32|
|S3|5v|near USB-SERIAL|
|S4|GND|for ESP32|
|S5|3.3v|for TFT|
|JP0 (pad nearest USB socket)|5v|TFT LDO|
|JP0|3.3v|TFT LDO|
|JP3 (pad nearest USB socket)|5v|ESP32 LDO|
|JP3|3.3v|ESP32 LDO|
