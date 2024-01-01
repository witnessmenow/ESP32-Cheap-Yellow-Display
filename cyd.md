## What is a Cheap Yellow Display (CYD)?

A CYD is a ESP32-2432S028, an ESP32 development board with a 2.8" display with a resistive touch screen,

There are other boards with different sizes displays that look similar but **are not** a CYD. This isn't to try exclude anyone, but there so many different displays and types that it would be incredibly difficult and very confusing to support all of them.

You can verify you have the correct board by checking the number on the back of the display.

![image](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/assets/1562562/d23bf84f-f34b-4814-b609-87c359d6334e)

## My CYD has two USB ports

The original CYD only has a micro USB port, but there is a device that is also labelled a *ESP32-2432S028* that has two USB ports, one micro USB and one USB-C.

If an extra USB port was the only difference, there would be no problems, but unfortunately that is not the case.

The display is also different on these boards, so it requires a different config file

Ideally we would just not call it a CYD, but it seems to be a very popular board, so I think it would be too confusing to not handle it.
