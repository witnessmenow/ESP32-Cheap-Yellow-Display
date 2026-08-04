/*******************************************************************
    A simple radio player for the ESP32 Cheap Yellow Display.

    Note: This sketch can play most radio streams, but it might be hard to 
    find the exact url to use. 
    You can some url's on the following sites:
    
    - https://streamurl.link/ 
    - http://listenlive.nl/
    - https://en.wikipedia.org/wiki/List_of_Internet_radio_stations/
    - https://github.com/mikepierce/internet-radio-streams/

    Also note that many streams are geographically restricted as well.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    If you find what Brian Lough do useful and would like to support him,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Jonny Bergdahl
    YouTube: https://www.youtube.com/jonnybergdahl
    Twitter: https://twitter.com/jonnybergdahl
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

// ----------------------------
// Standard Libraries
// ----------------------------

#include <Arduino.h>

// This example only works with ESP32 Board package in version 2.0.17 (not 3.x.x!)
#if ESP_IDF_VERSION_MAJOR == 5
  #warning This example only works with ESP32 Board package in version 2.0.17 (not 3.x.x!)
  void setup(){}
  void loop(){}
#else


#include <WiFi.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

#include "Audio.h"
// A library for playing music on the DAC of the ESP32
//
// Not available in library manager, so needs to be manually downloaded
// https://github.com/schreibfaul1/ESP32-audioI2S

// ----------------------------

// Change ssid and password for your local WiFi, or flash this example with the web
// wizard and enter your WiFi details there - it will patch these buffers for you.
const char WIFI_SSID[100] = "|*SSID*|";
const char WIFI_PASS[100] = "|*PASS*|";
const char RADIO_URL[100] = "https://media-ssl.musicradio.com/Heart90sMP3";

// Change this to set a fixed display rotation (0-3), or flash this example with the web
// wizard and pick a rotation there - it will patch this buffer for you.
const char DISPLAY_ROTATION[16] = "|*ROTATION*|";

TFT_eSPI tft = TFT_eSPI();
Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);

void setup() 
{
  bool succeeded;
  Serial.begin(115200);
  
  // Start the TFT display and set it to black
  tft.init();
  tft.setRotation(atoi(DISPLAY_ROTATION)); //This is the display in landscape
  tft.setTextWrap(true, true);

  // Clear the screen before writing to it and set default text colors
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.drawString("Title", 0, 10, 1);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup audio and 
  audio.forceMono(true);
  audio.setVolume(10);
  do
  {
    // Connect to 'FM - Disco Ball 70's-80's Los Angeles'
    succeeded = audio.connecttohost(RADIO_URL);
    delay(500);
    Serial.println("Retrying");
  } while (!succeeded);

}

void loop()
{
  // Prcoess audio
  audio.loop();
}

void printTitle(const char* info)
{
  tft.fillRect(0, 20, 320, 200, TFT_BLACK);
  tft.setCursor(0, 20, 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.println(info);
}

void printInfo(const char* info)
{
  tft.fillRect(0, 230, 320, 10, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(info, 0, 230, 1);
}

void audio_info(const char *info)
{
  Serial.print("info        "); 
  Serial.println(info);
  printInfo(info);
}

void audio_id3data(const char *info)
{  //id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info)
{  //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}

void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}

void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
    printTitle(info);
}

void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}

void audio_commercial(const char *info)
{  //duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}

void audio_icyurl(const char *info)
{  //homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}

void audio_lasthost(const char *info)
{  //stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}

void audio_eof_speech(const char *info)
{
    Serial.print("eof_speech  ");
    Serial.println(info);
}

#endif // ESP_IDF_VERSION_MAJOR
