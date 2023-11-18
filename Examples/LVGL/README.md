# LVGL examples

[LVGL](https://lvgl.io/) is a popular library for creating user interfaces on resource constrained devices like the ESP32.

You can install LVGL through the Arduino Library manager, but it will not compile 'out of the box'. Similarly to TFT_eSPI it needs manual editing of the configuration file.

Also, LVGL on Arduino tends to assume that you are using the touchscreen component of the TFT_eSPI display library, but the basic CYD cannot use this as the touchscreen controller is connected to a different SPI bus from the display.

The LVGL_Arduino example in here is a copy of the default Arduino example edited slightly to make the examples easier to use and include support for the XPT2046 driver used elsewhere in this repository.

## Installation

To be able to use LVGL examples and demos in the Arduino IDE, take the following steps.

- Install TFT_eSPI from the Arduino Library Manager and [make sure your display works with the examples](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md), as described elsewhere in this repo.
- Install XPT2046_Touchscreen from the Arduino Library Manager and [make sure your touchscreen works](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/tree/main/Examples/Basics/2-TouchTest), as described elsewhere in this repo. 
- Install LVGL from the Arduino Library Manager, search for 'lvgl'.
- **Do not** install 'lv_examples' from the Arduino Library Manager, it is for older versions than than the one that is currently installed. Also, you do not need to install 'lv_arduino' from the Arduino Library Manager, it is a different port of lvgl.
- Copy lv_conf.h from this repository to the Arduino 'libraries' folder. Not the LVGL library location, the folder 'above' that. See the [Arduino documentation](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) for help in locating the 'libraries' folder as it can vary between systems.
- Within the LVGL library directory move the 'examples' directory into the 'src' directory. This is due to a limitation of the Arduino IDE build system compared to other build systems used by LVGL. You will need to do repeat this if you upgrade LVGL.
- Within the LVGL library directory move the 'demos' directory into the 'src' directory, as above.

## Touchscreen calibration

Lines 78-79 map the readings from the resistive touchscreen to screen locations. These values were chosen based off an example CYD, you may need ot change them slightly.

The first number on line 78 is the touchscreen value at the left edge, the second the value at the right edge.

The first number on line 79 is the touchscreen value at the top edge, the second the value at the bottom edge.

```c++
    touchX = map(p.x,200,3700,1,screenWidth); /* Touchscreen X calibration */
    touchY = map(p.y,240,3800,1,screenHeight); /* Touchscreen Y calibration */
```

You can use the '[TouchTest.ino](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/tree/main/Examples/Basics/2-TouchTest)' example to get the values for your touchscreen.

## Choosing an example

The LVGL examples are not separate sketches, they are function calls you uncomment inside the single example sketch. Uncomment one line at a time and compile and upload the sketch again to see each example or demo. The build process for LVGL is quite long as it's a large library.

- Lines 145-203 are examples of a particular style of widget. Some are animated or mildly interactive.
- Lines 206-208 are more complete demos with multiple widgets of various types. The benchmark demo does a benchmark.

The sketch comes with the 'lv_demo_widgets' demo ready to go.

Not all of the examples/demos mentioned in the LVGL docs work, but the ones that do are included in the sketch for you to uncomment and try.

## Disabling example/demo compilation

Once you are done with the examples and demos you should probably remove support for them as it will reduce compile time.

To do this, edit 'lv_conf.h' in the Arduino 'libraries' folder and change '#define LV_BUILD_EXAMPLES 1' to '#define LV_BUILD_EXAMPLES 0' and '#define LV_USE_DEMO_WIDGETS 1' to '#define LV_USE_DEMO_WIDGETS 0'.
