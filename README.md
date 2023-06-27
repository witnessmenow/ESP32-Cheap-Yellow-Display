# ESP32-Cheap-Yellow-Display

There is an ESP32 with a built in 320 x 240 LCD display with a touch screen called the "ESP32-2432S028R", since this doesn't roll of the tounge, I propose it should be renamed the "Cheap Yellow Display" or CYD for short. This display is only about $15 delievered so I think it's really good value.

![image](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/assets/1562562/76c3d481-2523-4b6f-881c-2e29f9368cd0)


## Features

The CYD has the following features:

- ESP32 (With Wifi and Bluetooth)
- 320 x 240 LCD Display
- Touch Screen (Resistive)
- USB for powering and programming
- SD Card Slot, LED and some addtional pins broken out

## Who is it good for?

I think it's useful for the following types of people:

- **People just getting started with working hardware** - as everything is already connected, there is no soldering or additional components required
- **People who are familar with working with hardware, but are lazy** - (like me) Sometimes you just want to build a project without having to assemble any hardware
- **People who aren't really looking to learn anything, but just want to build some cool things** - More about this later.

## What is the purpose of this page?

So this is pretty nice hardware and a cheap price, but the software instructions/support around it is pretty poor. Just a single link to a zip file on a random website.

A couple of years ago I released the [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity), which is an open source ESP32 board for controlling Matrix panels. I think the main benifit people get out of the work I did on the Trinty is not the hardware, but the documentation, example code and ready to go projects. 

I'm no longer creating hardware products, but I think it would be interesting if we could create the same kind of community around this display, where people can share examples and projects made for this display. 

## Where to buy?

Buy from wherever works out cheapest for you:
- [Aliexpress*](https://s.click.aliexpress.com/e/_DkSpIjB)
- [Aliexpress*](https://s.click.aliexpress.com/e/_DkcmuCh)
- [Aliexpress](https://www.aliexpress.com/item/1005004502250619.html)
- [Makerfabs](https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html) - Seems to come with a 16GB SD card. Makerfabs also stock my [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity)
    
 \* = Affilate Link
 
 ## Getting Started With Your CYD

For details on how to get started with your CYD, please check out the [Setup and Configuration](/SETUP.md) page

## Examples

### The Basics
A collection of examples demonstrating how to use the different features of the CYD, this is a good place to get started. [Check them out here.](/Examples/Basics)

### Alternative Display Libraries
The basics examples are based on the TFT_eSPI display library, but the CYD also works with other display libraries too. Here is some example code if you prefer to use an alternative Arduino library. [Check them out here.](/Examples/AlternativeLibrary)

### ESPHome
Some examples for using the CYD in ESPHome. [Check them out here.](/Examples/ESPHome)

## Help Support what I do!

[If you enjoy my work, please consider becoming a Github sponsor!](https://github.com/sponsors/witnessmenow/)
