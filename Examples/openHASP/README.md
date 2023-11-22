# openHASP
[openHASP](https://www.openhasp.com) allows you to control your Home Assistant installation via customisable touchscreen UI.

## Current status
Support for the CYD is in beta, as such you'll need to use the 'nightly' builds.  Currently only the display and touchscreen are supported out of the box, adding support for the RGB LED is mostly trivial.  There is no support for the speaker and the LDR output does not appear to work

## OpenHASP vs ESPHome
ESPHome is much more configurable, supports a load more peripherals and is fairly easy to install and get working.  Generating pages for the CYD is done exclusively through YAML and is a bit of a drag.  Any buttons/widgets you create are generally automagically added to Home Assistant.

OpenHASP is much easier to work with, but is slightly harder to install.  Generating and testing pages is a lot easier, but peripheral support is basically non-existant in comparison.  Also any widgets you create have to be manually added via YAML to Home Assistant.


I went with OpenHASP as making pages was a lot easier and I didn't plan to use any other peripherals other than the touchscreen and display.

## Installing onto the CYD
Note you'll need to get the full firmware 'Sunton 2432S028R' build from the nightly page:
https://nightly.openhasp.com/

You should be able to web install from that page by pressing the 'INSTALL' button.  Personally I used the Espressif 'Flash Download Tools' to upload the binary to the CYD.
Further instructions can be found [here](https://openhasp.haswitchplate.com/0.7.0/firmware/esp32/)https://openhasp.haswitchplate.com/0.7.0/firmware/esp32/

Once installed onto the CYD you configure it by connecting to the wifi network it creates (it displays a QR code to make this easy).  Once connected to the network, navigate to http://192.168.4.1 to finish setting up the device.

## Setting the Home Assistant openHASP integration
NGL, this isn't a whole lot of fun and isn't a one click process.  It's described here:
https://openhasp.haswitchplate.com/0.7.0/integrations/home-assistant/howto/

## Creating and modifying pages
If you got this far, relax!  Installing and setting up the HA integration really is the hard part.  All that's left is to generate some pages for the CYD to display and then tell Home Assistant what you want to do when a button is pressed.

There are examples on how to do this in the Home Assistant howtos.

## Adding the RGB LED and physical button
Navigate to the device using a webbrowser
Goto 'Configuration'->'GPIO settings'->Add New Pin Outputs'
Add the following pin inputs, make sure they are set to 'inverted'
Pin	Type	Group	Default	Action
4	Mood Red	0	Inverted	
16	Mood Green	0	Inverted	
17	Mood Blue	0	Inverted	

To add the physical button, navigate to 'Configuration'->'GPIO settings'->Add New Pin Inputs'
Pin	Type	Group	Default	Action
0	Push Button	0	Normal

Unlike the touchscreen buttons, these will get automagically added to Home Assistant.
