# openHASP
[openHASP](https://www.openhasp.com) allows you to control your Home Assistant installation via a customisable touchscreen UI.

## Current status
Support for the CYD is in beta, as such you'll need to use the 'nightly' builds.  Currently only the display and touchscreen are supported out of the box but adding support for the RGB LED is mostly trivial.  There is no support for the speaker and the LDR output does not appear to work

## OpenHASP vs ESPHome
ESPHome is much more configurable, supports more peripherals and is fairly easy to install the integration and get working.  Generating pages for the CYD is done exclusively through YAML and is a bit fiddly.  Any buttons/widgets you create are generally automagically added to Home Assistant.

OpenHASP is much easier to work with, but is slightly harder to install.  Generating and testing pages is a lot easier, but peripheral support is basically non-existant in comparison.  Also any widgets you create have to be manually added via YAML to Home Assistant.


I went with OpenHASP as making pages was a lot easier and I didn't plan to use any other peripherals other than the touchscreen and display.

## Installing onto the CYD
Note you'll need to get the full firmware `Sunton 2432S028R` build from the [nightly page](https://nightly.openhasp.com/):


You should be able to web install from that page by selecting the correct build and pressing the `INSTALL` button.  Alternatively you can use the Espressif 'Flash Download Tools' to upload the firmware, instructions can be found [here](https://openhasp.haswitchplate.com/0.7.0/firmware/esp32/)

Once installed onto the CYD you configure it by connecting to the wifi network it creates (it displays a QR code to make this easy).  Once connected to the network, navigate to http://192.168.4.1 to finish [setting up the device.](https://www.openhasp.com/0.7.0/firmware/wifi-setup/)

## Setting up the Home Assistant openHASP integration
This isn't a whole lot of fun and isn't a one click process.  It's described [here](https://openhasp.haswitchplate.com/0.7.0/integrations/home-assistant/howto/)


## Creating and modifying pages
If you got this far, relax!  Installing and setting up the HA integration is the hard part.  All that's left is to generate some pages for the CYD to display and then tell Home Assistant what you want to do when a button is pressed.

There are examples on how to do this on the [openHASP website](https://www.openhasp.com/0.7.0/integrations/home-assistant/sampl_conf/).  Make sure you refer to the [0.7.0 documentation](https://www.openhasp.com/0.7.0/) as that is what the nightly builds use

## Loading pages from Home Assistant
It's possible to setup openHASP so instead of the pages being held in flash memory, they are loaded over the network from your Home Assistant installation.  You may find this useful if you have multiple devices displaying the same content.

Upload your pages.jsonl to somewhere on your Home Assistant server, I put mine in `/homeassistant/openhasp/pages.jsonl`

Make sure your configuration.yaml has something like the following:
```
homeassistant:
  allowlist_external_dirs:
    - "/config/openhasp"
```

Restart Home Assistant to update the configuration and navigate to `Home Assistant -> Settings -> Devices and services.`

Find the openHASP button under 'Configured' and click on it.  On the right hand side you should have a heading 'Integration Entities' with a list of your plates.  Click on configure.

An options box will pop up, under 'Full path to the JSONL file' put the path of the page you uploaded earlier, in my case it's `/config/openhasp/pages.jsonl`

## Adding the RGB LED and physical button
Navigate to the device using a webbrowser and goto `Configuration'->'GPIO settings'->Add New Pin Outputs`


Add the following pin inputs, make sure they are set to 'inverted'
```
Pin	Type	  Group	Default	Action

4	Mood Red	0	Inverted	

16	Mood Green	0	Inverted	

17	Mood Blue	0	Inverted	
```

To add the physical button, navigate to `Configuration'->'GPIO settings'->Add New Pin Inputs`
```
Pin	Type	  Group	Default	Action

0	Push Button	0	Normal
```
Unlike the touchscreen buttons, these will get automagically added to Home Assistant.


# Things to watch out for

## Large fonts
Be aware that [using fonts](https://www.openhasp.com/0.7.0/design/fonts/) with a pixel size greater than 32px causes openHASP to use a TTF font which requires a *lot* more memory.  On devices without PSRAM like the CYD this can cause random reboots and issues when loading certain pages in the WebUI

## MQTT Authentication problems
It should be as simple as going to `Home Assistant -> Settings -> People`, adding an `openhasp` user and setting a password.  Bear in mind there may be a problem when using especially long MQTT passwords as openHASP appears to have a hardcoded limit of [64 characters](https://github.com/HASwitchPlate/openHASP/blob/6cf4262f2356eaa686abe045ab01a5467a6963b1/include/hasp_conf.h#L16)

## Troubleshooting
When things start being weird, it's a good idea to connect to the serial port and look at the logs. Connect the plate to your PC via USB and download [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html).  Select the connection type as 'Serial' and the speed to '115200'.  You'll have to use device manager to see which COM port to put into the 'Serial line' box (mine comes up as COM7).
```
ELF file SHA256: 0000000000000000

Rebooting...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:184
load:0x40078000,len:12732
ho 0 tail 12 room 4
load:0x40080400,len:2908
entry 0x400805c4



        open____ _____ _____ _____
          |  |  |  _  |   __|  _  |
          |     |     |__   |   __|
          |__|__|__|__|_____|__|
        Home Automation Switch Plate
        Open Hardware edition v0.7.0-rc6

[06:21:16.693][110580/160512 31][    0/    0  0] DBUG: Started @ 115200 bps
#[06:21:16.702][110580/160512 31][    0/    0  0] DBUG: Environment: esp32-2432s028r_4MB
#[06:21:16.712][110580/160512 31][    0/    0  0] UART: Started
#[06:21:16.720][110580/160512 31][    0/    0  0] UART: Client login from serial
#[06:21:16.729][110580/160512 31][    0/    0  0] CONF: SPI flash FS mounted
Prompt > [06:21:16.767][110580/160512 31][    0/    0  0] CONF: Loading /config.json
Prompt > [06:21:16.778][110580/159712 30][    0/    0  0] CONF: Loaded /config.json
Prompt > [06:21:16.788][110580/160576 31][    0/    0  0] DBUG: Loading debug settings
```
