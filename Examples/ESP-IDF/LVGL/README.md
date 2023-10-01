# LVGL project for ESP32

This is an ESP32 demo project showcasing LVGL v7 with support for several display controllers and touch controllers.

![Example GUI_DEMO](images/new_photo.jpg)

Monochrome support:

![Example_mono](images/new_mono.jpg)

Supported display controllers:

## TFT

- ILI9341
- ILI9488
- ILI9486
- HX8357B/HX8357D
- ST7789
- ST7735S

## Monochrome

- SH1107
- SSD1306

## e-Paper

- IL3820

Supported touchscreen controllers:

- XPT2046
- FT3236
- other FT6X36 or the FT6206 controllers should work as well (not tested)
- STMPE610

If your display controller is not supported consider contributing to this repo by
adding support to it! [Contribute controller support](CONTRIBUTE_CONTROLLER_SUPPORT.md)

## Get started
### Install the ESP32 SDK
http://esp-idf.readthedocs.io/en/latest/

Note:

This project tries to be compatible with both the ESP-IDF v3.x and v4.0, but using the v4.0 is recommended.
Instructions here are given for the v4.x toolchain using `idf.py`, but it is easy to translate to make.
For example instead of running `idf.py menuconfig`, just run `make menuconfig`.

When using the ESP-IDF v3.x framework you must use `make` to build the project!.

### Build this repository standalone and run the demo.

Try this first to make sure your hardware is supported, wired and configured properly.

1. Get this project: `git clone --recurse-submodules
https://github.com/lvgl/lv_port_esp32.git`

2. From its root run `idf.py menuconfig`

3. Select your display kit or board and other options - see [config options](#configuration-options)

4. For monochrome displays we suggest enabling the `unscii 8` font (Component config -> LVGL configuration -> FONT USAGE) and the MONO theme (Component config -> LVGL configuration -> THEME USAGE).

5. Store your project configuration.

6. For monochrome displays edit the `lv_conf.h` file available on the `components/lvgl` directory to look like follows:

```
#define LV_THEME_DEFAULT_INIT               lv_theme_mono_init
#define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_BLACK
#define LV_THEME_DEFAULT_COLOR_SECONDARY    LV_COLOR_WHITE
#define LV_THEME_DEFAULT_FLAG               0
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_unscii_8
#define LV_THEME_DEFAULT_FONT_NORMAL        &lv_font_unscii_8
#define LV_THEME_DEFAULT_FONT_SUBTITLE      &lv_font_unscii_8
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_unscii_8
```

7. `idf.py build`

8. `idf.py -p (YOUR PORT) flash` (with make this is just `make flash` - in 3.x PORT is configured in `menuconfig`)



### Support for development kits with embedded TFT displays.

Several ESP32 evaluation kits are supported via preconfigurations.

- ESP Wrover Kit v4.1
- M5Stack
- M5Stick
- M5StickC
- Adafruit 3.5 Featherwing
- RPi MPI3501
- Wemos Lolin OLED
- ER-TFT035-6
- AIRcable ATAGv3

### Install this project as a library submodule in your own project

It is recommended to install this repo as a submodule in your IDF project's git repo. The configuration system has been designed so that you do not need to copy or edit any files in this repo. By keeping your submodule directory clean you can ensure reproducible builds and easy updates from this upstream repository.

From your project root (you can get the esp32 idf project template [here](https://github.com/espressif/esp-idf-template)):

1. `mkdir -p components`
2. `git submodule add https://github.com/lvgl/lv_port_esp32.git components/lv_port_esp32`
3. `git submodule update --init --recursive`
4. Edit your CMake or Makefile to add this repo's components folder to the IDF components path.


#### CMake

The examples below are taken from the ESP-IDF [blink](https://github.com/espressif/esp-idf/tree/master/examples/get-started/blink) example which you can copy and use as the basis for your own project.
The project root CMakeLists.txt file needs one line added, just before the project to add the extra components directory to the path like this:

```cmake
#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS components/lv_port_esp32/components/lv_examples components/lv_port_esp32/components/lvgl components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_tft components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_touch components/lv_port_esp32/components/lvgl_esp32_drivers)

project(blink)
```


### Temporal workaround

When adding this project as a component you need to update it's CMakeLists.txt file located at the root directory, like so (comment out the include line):

`components/lv_port_esp32/CMakeLists.txt`

```cmake

cmake_minimum_required(VERSION 3.5)

# include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS components/lv_port_esp32/components/lv_examples components/lv_port_esp32/components/lvgl components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_tft components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_touch components/lv_port_esp32/components/lvgl_esp32_drivers)

if (NOT DEFINED PROJECT_NAME)
	project(lvgl-demo)
endif (NOT DEFINED PROJECT_NAME)

```

In the CMakeLists.txt file for your `/main` or for the component(s) using LVGL you need to add REQUIRES directives for this project's driver and lvgl itself to the `idf_component_register` function, it should look like this:


```cmake
set (SOURCES main.c)

idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS .
    REQUIRES lvgl_esp32_drivers lvgl lv_examples lvgl_tft lvgl_touch)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
```

Please note that if your project require the use of the `nvs_flash` module \(for example required by WiFi\), it should be put in the `REQUIRES` list.

#### Makefile
If you are using make, you only need to add the EXTRA_COMPONENT_DIRS in the root Makefile of your project:
```Makefile
PROJECT_NAME := blink

EXTRA_COMPONENT_DIRS := components/lv_port_esp32/components/lv_examples \
    components/lv_port_esp32/components/lvgl \
    components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_tft \
    components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_touch \
    components/lv_port_esp32/components/lvgl_esp32_drivers \

include $(IDF_PATH)/make/project.mk
```

## Configuration options
There are a number of configuration options available, all accessed through `idf.py menuconfig` -> Components -> LittlevGL (LVGL).

![Main Menu](images/menu-main.png)
![Component Menu](images/new_lvgl_options.png)

You can configure the TFT controller and the touch controller (if your display have one)

![TFT Controller Menu](images/tft_controllers_options.png)
![Touch Controller Menu](images/touch_menu.png)

## Touch Controller options

Options include:
 * Touch controller options

![Touch Controllers](images/touch_controllers_options.png)

 * Pinout

![Touch pinout](images/touch_pinout.png)

 * SPI Bus: Choose what SPI bus is used to communicate with the touch controller.

![Touch SPI Bus](images/touch_spi_bus.png)

 * Touchpanel configuration: Maximum and minimum coordinate values, inverting coordinate values, etc.

![Touchpanel Configuration](images/touch_touch_panel_config.png)

## TFT Controller options

Options include:

 * Display controller: Support for the most common TFT display controllers

![TFT Display Controllers](images/tft_display_controller.png)

 * SPI Bus: Choose what SPI bus is used to communicate with the tft controller.

![Touch SPI Bus](images/tft_spi_bus.png)

 * Display resolution - set the height and width of the display

![TFT Resolution](images/tft_width_height.png)

 * Invert display - if text and objects are backwards, you can enable this
 * Enable backlight control via GPIO (vs hardwiring on)
 * Backlight active high or low - some displays expect a high (1) signal to enable backlight, others expect (low) (default) - if your backlight doesn't come on try switching this

![TFT Backlight Control](images/tft_backlight_control.png)

### Assign the correct pinout depending on your ESP32 dev board
There are several development boards based on the ESP32 chip, make sure you assign the correct pin numbers to the signals that interface with the TFT display board. Its recommended to use a predefined configuration below, but you can also set individual pins for both display controller and touch controller.

![Pins](images/tft_pin_assignments.png)

### Predefined Display Configurations

![Predefines](images/tft_predefined_display_config.png)

For development kits that come with a display already attached, all the correct settings are already known and can be selected in `menuconfig` from the first option "Select predefined display configuration." Once selected all the other options will be defaulted and won't appear in the menu.

