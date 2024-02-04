This section is to provide information for the different variants of "Yellow PCB with Display" type devices that are not a CYD

Note: The information here will hopefully help you get setup with the different device, but do not expect the examples and other info contained in this repo to work without making changes.

These displays will also need a custom user_setup.h, if one doesn't exist you may need to make one with the info from the Platform.io configs

### ESP32-1732S019

[AliExpress\*](https://s.click.aliexpress.com/e/_Ddcpl1j)

#### PlatformIO Config

```
[env:cyd1732]
platform = espressif32
board = esp32-s3-devkitm-1
framework = arduino
lib_ldf_mode = deep+
monitor_filters = esp32_exception_decoder
monitor_speed = 115200
upload_speed = 460800
board_build.partitions = min_spiffs.csv
build_flags =
    -DST7789_DRIVER
    -DTFT_WIDTH=170
    -DTFT_HEIGHT=320
    -DTFT_MISO=-1
    -DTFT_MOSI=13
    -DTFT_SCLK=12
    -DTFT_CS=10
    -DTFT_DC=11
    -DTFT_RST=1
    -DTFT_BL=14
    -DTFT_BACKLIGHT_ON=HIGH
    -DTFT_BACKLIGHT_OFF=LOW
    -DTOUCH_CS=-1
    -DLOAD_GLCD
    -DSPI_FREQUENCY=65000000
    -DSPI_READ_FREQUENCY=20000000
    -DSPI_TOUCH_FREQUENCY=2500000
    -DTFT_INVERSION_ON
```

### ESP32-2432S024

[AliExpress](https://www.aliexpress.com/item/1005005865107357.htm)

#### PlatformIO Config (Partial)

```
-DILI9341_2_DRIVER
-DTFT_WIDTH=240
-DTFT_HEIGHT=320
-DTFT_MISO=12
-DTFT_MOSI=13
-DTFT_SCLK=14
-DTFT_CS=15
-DTFT_DC=2
-DTFT_RST=-1
-DTFT_BL=27
-DTFT_BACKLIGHT_ON=HIGH
-DTFT_BACKLIGHT_OFF=LOW
-DTOUCH_CS=-1
-DLOAD_GLCD
-DSPI_FREQUENCY=55000000
-DSPI_READ_FREQUENCY=20000000
-DSPI_TOUCH_FREQUENCY=2500000
-DTFT_INVERSION_OFF
```

\* = Affiliate Link
