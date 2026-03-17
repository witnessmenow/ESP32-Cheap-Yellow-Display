#include <Arduino.h>
#include <WiFi.h>          // WiFi support for ESP32
#include <WiFiUdp.h>       // UDP used for NTP time requests
#include <TimeLib.h>       // Time keeping library
#include <Timezone.h>      // Timezone conversion support
#include <TFT_eSPI.h>      // Display driver library
#include <SPI.h>
#include <User_Setup.h>    // TFT configuration (pins, driver, etc)

/* ------------------------------------------------------------------
   WIFI CREDENTIALS
   List of SSID / password pairs the device will try to connect to
------------------------------------------------------------------ */

String credentials[][2] = {
  {"rue","NSA4ever"}
};

/* NTP server used to get accurate time */
const char ntpServerName[] = "pool.ntp.org";

/* ------------------------------------------------------------------
   TIMEZONE CONFIGURATION
   Defines daylight savings and standard time rules
------------------------------------------------------------------ */

TimeChangeRule myDST = {"PDT", Second, Sun, Mar, 2, -420}; // Pacific Daylight Time
TimeChangeRule mySTD = {"PST", First, Sun, Nov, 2, -480};  // Pacific Standard Time
Timezone myTZ(myDST,mySTD);

TimeChangeRule *tcr;   // pointer used when converting UTC → local time

/* ------------------------------------------------------------------
   DISPLAY OBJECT
------------------------------------------------------------------ */

TFT_eSPI tft = TFT_eSPI();

/* ------------------------------------------------------------------
   CLOCK MODE
------------------------------------------------------------------ */

bool use24Hour = true;     // true = 24 hour clock, false = 12 hour

/* Cache of previously displayed bits
   Used to prevent unnecessary redraws and eliminate screen flicker

   [row][bit]
   row 0 = hours
   row 1 = minutes
   row 2 = seconds
*/
bool lastBits[3][6];

/* ------------------------------------------------------------------
   DISPLAY LAYOUT SETTINGS
------------------------------------------------------------------ */

int dotSize = 11;    // radius of binary dots

// starting position for first dot
int startX = 60;
int startY = 80;

// spacing between dots
int colSpacing = 32;
int rowSpacing = 70;

/* ------------------------------------------------------------------
   NTP NETWORK SETTINGS
------------------------------------------------------------------ */

WiFiUDP Udp;                // UDP object used for NTP packets
unsigned int localPort = 8888;

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

/* ------------------------------------------------------------------
   WIFI CONNECTION FUNCTION
   Attempts to connect using each credential pair
------------------------------------------------------------------ */

void connectWiFi()
{
  int networks = sizeof(credentials)/sizeof(credentials[0]);

  while(WiFi.status()!=WL_CONNECTED)
  {
    for(int i=0;i<networks;i++)
    {
      WiFi.begin(credentials[i][0].c_str(),
                 credentials[i][1].c_str());

      int tries=20;

      // wait up to ~10 seconds
      while(WiFi.status()!=WL_CONNECTED && tries--)
        delay(500);

      if(WiFi.status()==WL_CONNECTED)
        return;
    }
  }
}

/* ------------------------------------------------------------------
   SEND NTP REQUEST
   Sends a packet to the NTP server asking for the current time
------------------------------------------------------------------ */

void sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer,0,NTP_PACKET_SIZE);

  // NTP request header
  packetBuffer[0]=0b11100011;
  packetBuffer[1]=0;
  packetBuffer[2]=6;
  packetBuffer[3]=0xEC;

  // magic bytes used by NTP
  packetBuffer[12]=49;
  packetBuffer[13]=0x4E;
  packetBuffer[14]=49;
  packetBuffer[15]=52;

  Udp.beginPacket(address,123);
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

/* ------------------------------------------------------------------
   NTP TIME FETCH
   Requests time from the server and converts it to Unix time
------------------------------------------------------------------ */

time_t getNtpTime()
{
  IPAddress ntpServerIP;

  // clear any old packets
  while(Udp.parsePacket()>0);

  // resolve hostname
  WiFi.hostByName(ntpServerName,ntpServerIP);

  // send request
  sendNTPpacket(ntpServerIP);

  uint32_t beginWait=millis();

  // wait up to 1.5 seconds for response
  while(millis()-beginWait<1500)
  {
    int size=Udp.parsePacket();

    if(size>=NTP_PACKET_SIZE)
    {
      Udp.read(packetBuffer,NTP_PACKET_SIZE);

      unsigned long secsSince1900=
      (unsigned long)packetBuffer[40]<<24|
      (unsigned long)packetBuffer[41]<<16|
      (unsigned long)packetBuffer[42]<<8|
      (unsigned long)packetBuffer[43];

      // convert NTP epoch (1900) to Unix epoch (1970)
      return secsSince1900-2208988800UL;
    }
  }

  return 0; // failed
}

/* ------------------------------------------------------------------
   DRAW A SINGLE DOT
   bit = true  → green dot (1)
   bit = false → dark gray dot (0)
------------------------------------------------------------------ */

void drawDot(int x,int y,bool bit)
{
  uint16_t color = bit ? TFT_GREEN : TFT_DARKGREY;

  tft.fillCircle(x,y,dotSize,color);
}

/* ------------------------------------------------------------------
   UPDATE ONE ROW OF BITS
   Only redraws dots that have changed since last update
   This prevents the screen from blinking
------------------------------------------------------------------ */

void updateRow(int value,int bits,int row)
{
  for(int i=0;i<bits;i++)
  {
    bool bit = value & (1<<(bits-1-i));

    if(lastBits[row][i] != bit)
    {
      int x = startX + i*colSpacing;
      int y = startY + row*rowSpacing;

      drawDot(x,y,bit);

      lastBits[row][i] = bit;
    }
  }
}

/* ------------------------------------------------------------------
   DRAW THE CLOCK
   Converts UTC → local time and updates binary rows
------------------------------------------------------------------ */

void drawClock(time_t utc)
{
  time_t local=myTZ.toLocal(utc,&tcr);

  int h=hour(local);
  int m=minute(local);
  int s=second(local);

  if(!use24Hour)
    h=hourFormat12(local);

  /* draw labels */

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.drawString("HOURS",60,30);
  tft.drawString("MINUTES",60,100);
  tft.drawString("SECONDS",60,170);

  /* update binary rows */

  updateRow(h,5,0);
  updateRow(m,6,1);
  updateRow(s,6,2);
}

/* ------------------------------------------------------------------
   SETUP
   Runs once at boot
------------------------------------------------------------------ */

void setup()
{
  Serial.begin(115200);

  // initialize display
  tft.init();

  tft.setRotation(1);        // landscape orientation

  // some CYD displays require inversion
  tft.invertDisplay(true);

  // clear display
  tft.fillScreen(TFT_BLACK);

  // initialize bit cache so first draw updates everything
  memset(lastBits,255,sizeof(lastBits));

  // connect to WiFi
  connectWiFi();

  // start UDP service for NTP
  Udp.begin(localPort);

  // set time sync provider
  setSyncProvider(getNtpTime);

  // resync every 5 minutes
  setSyncInterval(300);
}

/* ------------------------------------------------------------------
   MAIN LOOP
   Updates clock once per second
------------------------------------------------------------------ */

void loop()
{
  // reconnect WiFi if needed
  if(WiFi.status()!=WL_CONNECTED)
    connectWiFi();

  // update display
  drawClock(now());

  delay(1000);
}