## What is a Cheap Yellow Display (CYD)?

A CYD is a ESP32-2432S028, an ESP32 development board with a 2.8" display with a resistive touch screen,

There are other boards with different sizes displays that look similar but **are not** a CYD. This isn't to try exclude anyone, but there so many different displays and types that it would be incredibly difficult and very confusing to support all of them.

You can verify you have the correct board by checking the number on the back of the display.

![image](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/assets/1562562/d23bf84f-f34b-4814-b609-87c359d6334e)

## My CYD has two USB ports

The original CYD only has a micro USB port, but there is a device that is also labelled a _ESP32-2432S028_ that has two USB ports, one micro USB and one USB-C.

Having an additional USB port would be a minor problem if that was the only difference, but unfortunately the display also works differently, the colours are inverted on the display.

It can be fixed in a couple of ways:

- Use platformio - The examples on the Github have all been updated so they can be used with platformio, and you can simply select CYD or CYD2USB and it will just work
- Use the CYD2USB specific User_setup.h that is on the repo, you can now use all the examples like normal
- Invert the display at the code level using the `tft.invertDisplay(1);` method

Ideally we would just not call it a CYD, but it seems to be a very popular board, so I think it would be too confusing to not handle it.
