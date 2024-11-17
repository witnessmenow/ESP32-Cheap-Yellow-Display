# CYD Mods

Here are some hardware mods that can be performed on the CYD to improve or change some of its functionality

## Improve LDR Range and performance

The LDR on the CYD has a pretty limited range from fully dark to fully bright. This mod increases that range and also smooths the output from the LDR out.

It involves soldering hand soldering on top of 0603 components, so probably not for beginner solderers!

[Link](https://github.com/hexeguitar/ESP32_TFT_PIO#1-ldr)

## Audio amp gain mod

The configuration of the Audio amp on the CYD is not good, resulting in poor quality audio, this mod reduces the gain which leads to better sounding audio.

As decribed, it involves soldering another resistor on top of a 0603 component, but you could also add a 10k resistor across the bottom two IC legs for the same mod. Do which ever seems easier to you!

![image](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/assets/1562562/04b98352-ca41-4bcc-bf77-380db4cce1da)

[Link](https://github.com/hexeguitar/ESP32_TFT_PIO#2-audio-amp-gain-mod)

## Adding PSRAM

Its possible with some board modification, to use use PSRAM with the CYD

This mod involves removing the RGB LED, cutting some traces and installing the PSRAM IC

[Link](https://github.com/hexeguitar/ESP32_TFT_PIO#adding-psram)
