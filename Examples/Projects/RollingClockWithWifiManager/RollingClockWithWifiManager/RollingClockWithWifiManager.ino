/*******************************************************************
    /* === CYD ROLLING CLOCK ===
    This example shows a digital clock with a rolling effect as the digits change.
    Most of the code are borrowed from other examples. Thanks Internet!


    Brian:
    Took the rolling clock example and added the following to make it webflashable

    - Wifi manager for configuring
    - Double reset detector for entering config mode
    - Saving and loading config
    - NTP and Timezones

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// Change these to set a fixed display rotation (0-3) and/or color inversion (0 or 1 -
// CYD2USB units usually need 1, see cyd.md), or flash this example with the web wizard
// and pick them there - it will patch these buffers for you.
const char DISPLAY_ROTATION[16] = "|*ROTATION*|";
const char DISPLAY_INVERT[16] = "|*INVERT*|";

// Your timezone as a POSIX TZ string, e.g. "CST6CDT,M3.2.0,M11.1.0" for US Central, or
// "GMT0BST,M3.5.0/1,M10.5.0" for UK time - see https://github.com/nayarsystems/posix_tz_db
// for a lookup table covering every IANA timezone. Or flash this example with the web
// wizard and pick your timezone there - it auto-detects it from your browser and
// patches this buffer for you.
const char POSIX_TZ[64] = "|*TZ*|";

#include "genericBaseProject.h"
#include "RollingClockLogic.h"

void setup()
{
    Serial.begin(115200);

    baseProjectSetup();
    // You will be fully connected by the time you are here

    rollingClockSetup(projectConfig.twentyFourHour, projectConfig.usDateFormat);
}

bool first = true;
time_t prevDisplay = 0;

void loop()
{
    baseProjectLoop();

    time_t current;
    time(&current);
    if (first || current != prevDisplay)
    {
        prevDisplay = current;
        drawRollingClock();
        first = false;
    }
}