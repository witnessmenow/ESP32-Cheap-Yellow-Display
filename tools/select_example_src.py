"""
PlatformIO pre-build script: stages the current environment's example .ino/.pde file
into the shared src/ directory.

PlatformIO's Arduino framework support only auto-converts *.ino/*.pde files that sit
directly inside the configured src_dir (see ConvertInoToCpp/FindInoNodes upstream) - it
does not consult build_src_filter for them, and src_dir itself can't be overridden per
environment. Since this repo has one src_dir shared by every example environment, this
script copies the right example's sketch file into src/ (and clears out whatever the
previous environment in this `pio run` left behind) before each environment builds.

Every other source file an example needs (extra .cpp/.h/.c files alongside its .ino) is
still picked up directly from its Examples/... folder via each environment's own
build_src_filter - only the .ino/.pde entry file needs to physically live in src/.

Keep EXAMPLE_DIRS in sync with the `+<../Examples/...>` / `+<../Variants/...>` path used
in each environment's build_src_filter in platformio.ini.
"""

import glob
import os
import shutil

Import("env")  # noqa: F821  (SCons injects this)

EXAMPLE_DIRS = {
    "adafruit-ili9341": "Examples/AlternativeLibraries/Adafruit_ILI9341/1-Adafruit_ILI9341_Test",
    "basics-1-helloworld": "Examples/Basics/1-HelloWorld",
    "basics-2-touchtest": "Examples/Basics/2-TouchTest",
    "basics-3-sdcardtest": "Examples/Basics/3-SDCardTest",
    "basics-4-backlightcontroltest": "Examples/Basics/4-BacklightControlTest",
    "basics-5-ldrtest": "Examples/Basics/5-LDRTest",
    "basics-6-ledtest": "Examples/Basics/6-LEDTest",
    "basics-7-helloradio": "Examples/Basics/7-HelloRadio",
    "basics-8-buttons": "Examples/Basics/8-Buttons",
    "bb-spi-lcd": "Examples/AlternativeLibraries/bb_spi_lcd/1-bb_spi_lcd_Test",
    "external-button-test": "Examples/InputTests/ExternalButtonTest",
    "leetcode-daily-question": "Examples/Projects/LeetCodeDailyQuestion",
    "lvgl8": "Examples/LVGL8/LVGL_Arduino",
    "lvgl9": "Examples/LVGL9/LVGL_Arduino",
    "lvgl9-plus-led": "Examples/LVGL9/LVGL_plus_LED",
    "nfc-via-sd-sniffer": "Examples/InputTests/NFCviaSDSniffer/SimpleNFCTest",
    "nunchuck-test": "Examples/InputTests/NunchuckTest",
    "rollingclock": "Examples/Projects/RollingClock",
    "rollingclock-wifimanager": "Examples/Projects/RollingClockWithWifiManager/RollingClockWithWifiManager",
    "slideshow": "Examples/Projects/Slideshow",
    "squareline": "Examples/Projects/Squareline",
    "tetris-nunchuck": "Examples/Projects/TetrisWithNunchuck",
    "volos-sliders-rgb": "Examples/Projects/VolosSlidersRGB/slidersRGB",
}


def _example_base_name(pioenv):
    for suffix in ("-cyd2usb", "-cyd"):
        if pioenv.endswith(suffix):
            return pioenv[: -len(suffix)]
    return pioenv


pioenv = env["PIOENV"]
base_name = _example_base_name(pioenv)
example_dir = EXAMPLE_DIRS.get(base_name)
if not example_dir:
    raise SystemExit(
        "select_example_src.py: no EXAMPLE_DIRS entry for environment '%s' "
        "(base name '%s')" % (pioenv, base_name)
    )

project_dir = env.subst("$PROJECT_DIR")
src_dir = env.subst("$PROJECT_SRC_DIR")
abs_example_dir = os.path.join(project_dir, example_dir)

os.makedirs(src_dir, exist_ok=True)

# Clear out whatever the previously built environment staged here.
for stale in glob.glob(os.path.join(src_dir, "*.ino")) + glob.glob(
    os.path.join(src_dir, "*.pde")
) + glob.glob(os.path.join(src_dir, "*.ino.cpp")):
    os.remove(stale)

sketch_files = glob.glob(os.path.join(abs_example_dir, "*.ino")) + glob.glob(
    os.path.join(abs_example_dir, "*.pde")
)
if not sketch_files:
    raise SystemExit(
        "select_example_src.py: no .ino/.pde file found in %s for environment '%s'"
        % (abs_example_dir, pioenv)
    )

for sketch_file in sketch_files:
    shutil.copy2(sketch_file, src_dir)
