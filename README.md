# ESP32-Cheap-Yellow-Display

There is an ESP32 with a built in 320 x 240 2.8" LCD display with a touch screen called the "ESP32-2432S028R", since this doesn't roll of the tongue, I propose it should be renamed the "Cheap Yellow Display" or CYD for short. This display is only about $15 delivered so I think it's really good value.

![image](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/assets/1562562/76c3d481-2523-4b6f-881c-2e29f9368cd0)

## Features

The CYD has the following features:

- ESP32 (With Wifi and Bluetooth)
- 320 x 240 LCD Display (2.8")
- Touch Screen (Resistive)
- USB for powering and programming
- SD Card Slot, LED and some additional pins broken out

## Who is it good for?

I think it's useful for the following types of people:

- **People just getting started with working hardware** - as everything is already connected, there is no soldering or additional components required
- **People who are familiar with working with hardware, but are lazy** - (like me) Sometimes you just want to build a project without having to assemble any hardware
- **People who aren't really looking to learn anything, but just want to build some cool things** - More about this later.

## What is the purpose of this page?

So this is pretty nice hardware and a cheap price, but the software instructions/support around it is pretty poor. Just a single link to a zip file on a random website.

A couple of years ago I released the [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity), which is an open source ESP32 board for controlling Matrix panels. I think the main benefit people get out of the work I did on the Trinty is not the hardware, but the documentation, example code and ready to go projects.

I'm no longer creating hardware products, but I think it would be interesting if we could create the same kind of community around this display, where people can share examples and projects made for this display.

## How do I know if a display is a CYD?
![CYD decision tree](http://www.plantuml.com/plantuml/png/RP0nJyCm48Nt_8gZNIb3fge3LD2b2q92235UamDRE7PaNuhyxxda7DGgJBs-zxtSE-yJO-IXSzKD6-e8UeVMLyQs1DJrdA6br4JRims-4fW9LiS4bY6JS-47qBTWC052QvEayyCAvA-wS-8vi01F7mS8SVevOxJeUK9zu55QzzP_Nw-exxPmz8tHJzRRsJq4cdo3Pu98oIQsCd4O6WDIbyXF4LN-JNMsYG7UNXyXUAUTLHDfqVeMJWClUfSPrY_OOyPtO_ivUPcfnoMV3iyXJh4cj_MGJd8lEleQkvQKi9TYUT_DvbukXnraIfTQURMT39Nu8kcrXInIwQYO-gCyNwgm6al-ZneTNIRqjLokqS2UV3jqxXS0)

## Where to buy?

Buy from wherever works out cheapest for you:

- [Aliexpress\*](https://s.click.aliexpress.com/e/_DkSpIjB)
- [Aliexpress\*](https://s.click.aliexpress.com/e/_DkcmuCh)
- [Aliexpress](https://www.aliexpress.com/item/1005004502250619.html)
- [Makerfabs](https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html) - Seems to come with a 16GB SD card. Makerfabs also stock my [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity) (NOTE there will be import due in the EU from makerfabs)

\* = Affiliate Link

## Getting Started With Your CYD

For details on how to get started with your CYD, please check out the [Setup and Configuration](/SETUP.md) page

## Code Examples

### The Basics

A collection of examples demonstrating how to use the different features of the CYD, this is a good place to get started. [Check them out here.](/Examples/Basics)

### Alternative Display Libraries

The basics examples are based on the TFT_eSPI display library, but the CYD also works with other display libraries too. Here is some example code if you prefer to use an alternative Arduino library. [Check them out here.](/Examples/AlternativeLibraries)

### ESPHome

Some examples for using the CYD in ESPHome. [Check them out here.](/Examples/ESPHome)

## Additional Info and Links

### Discord

Join the CYD discussion on [my Discord channel](https://discord.gg/nnezpvq)

### 3DPrinting

Some examples of 3D printed stands and cases. [Check them out here.](/3dModels)

### Pin Information

[This page](/PINS.md) contains information about what pins are used where, and what ones are free to use.

### Add-ons

[This page](/ADDONS.md) contains information about additional hardware add-ons that can add functionality to your CYD

### Troubleshooting

[This page](/TROUBLESHOOTING.md) contains information about how to troubleshoot your CYD device

### Hardware Mods

[This page](/Mods/README.md) contains information about some hardware mods that can be performed on the CYD to improve or change some of its functionality

### Media and Video Mentions

[This page](/MEDIA.md) lists any times the CYD project was mentioned somewhere!

## License Info

This project is licensed as MIT as per the [license file](/LICENSE)

The one exception to this is the [OriginalDocumentation](/OriginalDocumentation/) folder, that I do not have the right to license

## Other Languages

Some members of the community have ported some of this information to other languages! 

Please note: I can't gaurantee the accuracy of the translation, how up to date they are or the content on them in general.

- [French / Française](https://github.com/usini/ESP32-Cheap-Yellow-Display-Documentation-FR)
- [German / Deutsch](https://github.com/paelzer/ESP32-Cheap-Yellow-Display-Documentation-DE)

If you would like to contribure a translation, please name the repo with the language name or code in the repo name and you can link it here.

## Help Support what I do!

[If you enjoy my work, please consider becoming a Github sponsor!](https://github.com/sponsors/witnessmenow/)
