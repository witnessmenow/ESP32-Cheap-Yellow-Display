/*-------- CYD (Cheap Yellow Display) ----------*/

// TFT_eSPI tft = TFT_eSPI();              // Invoke custom library
//  Will be defined outside of here
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite class

int clockFont = 1;
int clockSize = 6;
int clockDatum = TL_DATUM;
uint16_t clockBackgroundColor = TFT_BLACK;
uint16_t clockFontColor = TFT_YELLOW;
int prevDay = 0;

bool SHOW_24HOUR = false;
bool SHOW_AMPM = true;

bool NOT_US_DATE = true;

void SetupCYD()
{
    Serial.println("SetupCYD");
    // tft.init();
    tft.fillScreen(clockBackgroundColor);
    tft.setTextColor(clockFontColor, clockBackgroundColor);

    tft.setRotation(1);
    tft.setTextFont(clockFont);
    tft.setTextSize(clockSize);
    tft.setTextDatum(clockDatum);

    sprite.createSprite(tft.textWidth("8"), tft.fontHeight());
    sprite.setTextColor(clockFontColor, clockBackgroundColor);
    sprite.setRotation(1);
    sprite.setTextFont(clockFont);
    sprite.setTextSize(clockSize);
    sprite.setTextDatum(clockDatum);
}

/*-------- Digits ----------*/
#include "Digit.h"
Digit *digs[6];
int colons[2];
int timeY = 50;
int ampm[2]; // X, Y of the AM or PM indicator
bool ispm;

void CalculateDigitOffsets()
{
    int y = timeY;
    int width = tft.width();
    int DigitWidth = tft.textWidth("8");
    int colonWidth = tft.textWidth(":");
    int left = SHOW_AMPM ? 10 : (width - DigitWidth * 6 - colonWidth * 2) / 2;
    digs[0]->SetXY(left, y);                      // HH
    digs[1]->SetXY(digs[0]->X() + DigitWidth, y); // HH

    colons[0] = digs[1]->X() + DigitWidth; // :

    digs[2]->SetXY(colons[0] + colonWidth, y); // MM
    digs[3]->SetXY(digs[2]->X() + DigitWidth, y);

    colons[1] = digs[3]->X() + DigitWidth; // :

    digs[4]->SetXY(colons[1] + colonWidth, y); // SS
    digs[5]->SetXY(digs[4]->X() + DigitWidth, y);

    ampm[0] = digs[5]->X() + DigitWidth + 4;
    ampm[1] = y - 2;
}

void SetupDigits()
{
    tft.fillScreen(clockBackgroundColor);
    tft.setTextFont(clockFont);
    tft.setTextSize(clockSize);
    tft.setTextDatum(clockDatum);

    for (size_t i = 0; i < 6; i++)
    {
        digs[i] = new Digit(0);
        digs[i]->Height(tft.fontHeight());
    }

    //-- Measure font widths --
    // Debug("1", tft.textWidth("1"));
    // Debug(":", tft.textWidth(":"));
    // Debug("8", tft.textWidth("8"));

    CalculateDigitOffsets();
}

/*-------- DRAWING ----------*/
void DrawColons()
{
    tft.setTextFont(clockFont);
    tft.setTextSize(clockSize);
    tft.setTextDatum(clockDatum);
    tft.drawChar(':', colons[0], timeY);
    tft.drawChar(':', colons[1], timeY);
}

void DrawAmPm()
{
    if (SHOW_AMPM)
    {
        tft.setTextSize(3);
        tft.drawChar(ispm ? 'P' : 'A', ampm[0], ampm[1]);
        tft.drawChar('M', ampm[0], ampm[1] + tft.fontHeight());
    }
}

void DrawADigit(Digit *digg); // Without this line, compiler says: error: variable or field 'DrawADigit' declared void.

void DrawADigit(Digit *digg)
{
    if (digg->Value() == digg->NewValue())
    {
        sprite.drawNumber(digg->Value(), 0, 0);
        sprite.pushSprite(digg->X(), digg->Y());
    }
    else
    {
        for (size_t f = 0; f <= digg->Height(); f++)
        {
            digg->Frame(f);
            sprite.drawNumber(digg->Value(), 0, -digg->Frame());
            sprite.drawNumber(digg->NewValue(), 0, digg->Height() - digg->Frame());
            sprite.pushSprite(digg->X(), digg->Y());
            delay(5);
        }
        digg->Value(digg->NewValue());
    }
}

void DrawDigitsAtOnce()
{
    tft.setTextDatum(TL_DATUM);
    for (size_t f = 0; f <= digs[0]->Height(); f++) // For all animation frames...
    {
        for (size_t di = 0; di < 6; di++) // for all Digits...
        {
            Digit *dig = digs[di];
            if (dig->Value() == dig->NewValue()) // If Digit is not changing...
            {
                if (f == 0) //... and this is first frame, just draw it to screeen without animation.
                {
                    sprite.drawNumber(dig->Value(), 0, 0);
                    sprite.pushSprite(dig->X(), dig->Y());
                }
            }
            else // However, if a Digit is changing value, we need to draw animation frame "f"
            {
                dig->Frame(f);                                                       // Set the animation offset
                sprite.drawNumber(dig->Value(), 0, -dig->Frame());                   // Scroll up the current value
                sprite.drawNumber(dig->NewValue(), 0, dig->Height() - dig->Frame()); // while make new value appear from below
                sprite.pushSprite(dig->X(), dig->Y());                               // Draw the current animation frame to actual screen.
            }
        }
        delay(5);
    }

    // Once all animations are done, then we can update all Digits to current new values.
    for (size_t di = 0; di < 6; di++)
    {
        Digit *dig = digs[di];
        dig->Value(dig->NewValue());
    }
}

void DrawDigitsWithoutAnimation()
{
    for (size_t di = 0; di < 6; di++)
    {
        Digit *dig = digs[di];
        dig->Value(dig->NewValue());
        dig->Frame(0);
        sprite.drawNumber(dig->NewValue(), 0, 0);
        sprite.pushSprite(dig->X(), dig->Y());
    }
}

void DrawDigitsOneByOne()
{
    tft.setTextDatum(TL_DATUM);
    for (size_t i = 0; i < 6; i++)
    {
        DrawADigit(digs[5 - i]);
    }
}

void ParseDigits()
{
    time_t local = myTZ.now();
    digs[0]->NewValue((SHOW_24HOUR ? hour(local) : hourFormat12(local)) / 10);
    digs[1]->NewValue((SHOW_24HOUR ? hour(local) : hourFormat12(local)) % 10);
    digs[2]->NewValue(minute(local) / 10);
    digs[3]->NewValue(minute(local) % 10);
    digs[4]->NewValue(second(local) / 10);
    digs[5]->NewValue(second(local) % 10);
    ispm = isPM(local);
}

void DrawDate()
{
    // time_t local = myTZ.toLocal(utc, &tcr);
    time_t local = myTZ.now();
    int dd = day(local);
    int mth = month(local);
    int yr = year(local);

    if (dd != prevDay)
    {
        prevDay = dd;
        tft.setTextDatum(BC_DATUM);
        char buffer[50];
        if (NOT_US_DATE)
        {
            sprintf(buffer, "%02d/%02d/%d", dd, mth, yr);
        }
        else
        {
            // MURICA!!
            sprintf(buffer, "%02d/%02d/%d", mth, dd, yr);
        }

        tft.setTextSize(4);
        int h = tft.fontHeight();
        tft.fillRect(0, 210 - h, 320, h, TFT_BLACK);

        tft.drawString(buffer, 320 / 2, 210);

        int dow = weekday(local);
        String dayNames[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        tft.setTextSize(4);
        tft.fillRect(0, 170 - h, 320, h, TFT_BLACK);
        tft.drawString(dayNames[dow], 320 / 2, 170);
    }
}

void rollingClockSetup(bool is24Hour, bool notUsDate)
{
    Serial.println("rollingClockSetup");
    SHOW_24HOUR = is24Hour;
    SHOW_AMPM = !is24Hour;
    NOT_US_DATE = notUsDate;
    SetupCYD();
    SetupDigits();
}

void drawRollingClock()
{
    ParseDigits();
    DrawDigitsAtOnce(); // Choose one: DrawDigitsWithoutAnimation(), DrawDigitsAtOnce(), DrawDigitsOneByOne()
    DrawDate();         // Draw Date and day of the week.
    DrawColons();
    DrawAmPm();
}