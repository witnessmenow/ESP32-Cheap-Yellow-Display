# Micropython on the Cheap Yellow Display

## Installing Micropython

The [standard release of Micropython for ESP32](https://micropython.org/download/ESP32_GENERIC/) works fine with this module. 

## Additional libraries

Drivers for the display and touchscreen can be found in the [`micropython-ili9341`](https://github.com/rdagger/micropython-ili9341) project.

## Sample code

Check `demo.py` for code that shows how to use the:

* ✅ display and backlight
* ✅ RGB LED
* ✅ SD card
* ✅ light sensor
* ✅ touchscreen

Not yet tested:

* ❓ I2S audio output (should be [natively supported in Micropython](https://docs.micropython.org/en/latest/library/machine.I2S.html))

## Addidtional Micropython Examples for LVGL8
Aditional examples demonstrating the use of LVGL8 on the CYD can be found [here](https://github.com/de-dh/CYD2-MPY-LVGL).
The repositry also includes a link to precompiled firmware images of Micropython + LVGL for CYD.
Differences in the display driver's configuration for the one-usb-port version and the two-port-version are explained.
