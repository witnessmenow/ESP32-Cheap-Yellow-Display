# SDFmt

## Overview

The SDFmt tool is a utility for formatting SD cards using the SD card library
provided by the ESP32 Arduino framework.

If you have an SD card that you cannot get the CYD to utilize, this tool
may be able to help.

> ***Caveat:*** If you are using this tool because you cannot get the CYD to
> utilize the card when formatted by other platforms, then you may be presented
> with the inverse scenario when the formatting is complete (i.e., the platform
> you were originally attempting to format the card with may no longer be able
> to utilize the CYD formatted card). This is a useful solution if you goal is
> to use the card only with the CYD. You can always restore the card to its
> original (CYD incompatible) state by reformatting it with the original
> platform.

## Setup

Upload the SDFmt project to your CYD board.

## Usage

> ***NOTICE:*** Technically the SD card must be empty before formatting because
> this tool uses the `format_if_empty` flag of the `SDFS::begin` method.
> However, in practice, if your card fails to mount then it will be formatted
> regardless of its contents.

1. Insert the SD card you want to format into the CYD.
2. Press the reset button on the CYD to format the card.
    * Progress is indicated by the board LED:
        * Blue: In progress.
        * Green: Success.
        * Red: Error.
    * Progress messages are also written to the serial console.
3. Repeat steps 1 and 2 for each card you want to format.
