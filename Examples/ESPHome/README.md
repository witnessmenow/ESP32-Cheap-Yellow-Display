# ESPHome examples

These examples shows the yaml code to get the ESP32-2432S028R display to work in ESPHome.

In order for the yaml to work you need to add the following settings in your secrets.yaml file:
 - api_key
 - ota_password
 - wifi_ssid
 - wifi_password
 - ap_password

For some of the examples you need to copy font or image files to your ESPHome folder. Those
examples contains the instructions in the yaml file itself.

> [!NOTE]
> #### NEW HARDWARE VERSION WITH USB-C CONNECTOR:
> for this, the display component needs to be set up like the following (model ili9342, no rotation, inverted)
>```
>display:
>  - platform: ili9xxx
>    id: esp_display
>    model: ili9342
>    spi_id: tft
>    cs_pin: GPIO15
>    dc_pin: GPIO2
>    #invert_display: true (changed to below with ESPhome 2023.12.0)
>    invert_colors: true
>```

## 1-HelloWorld 

This yaml displays the text Hello World centered in the display.

## 2-ESPHomeClock

This yaml displays the ESPHome logo and the current date and time.

## 3-RGBLED

This yaml publishes the RGB LED on the back of the display to Home Assistant.

## 4-TouchDemo

This yaml displays two rectangles acting as binary touch sensors inside of Home Assistant.

## 5-TemperatureGraph

This yaml displays a graph of a Home Assistant temperature sensor.

## 6-VariousFeatures

Example for ESPHome for board with 2 USB ports using most of the features of the board (also works on single USB port boards)

## 7-ExtendedTouchDemo

Extend the ESPHome touch demo to add:
- Icons from the Material Design Icons font
- A Wifi signal icon in the top right
- Sleep mode that turns off the backlight
- The LED on the back of the board
