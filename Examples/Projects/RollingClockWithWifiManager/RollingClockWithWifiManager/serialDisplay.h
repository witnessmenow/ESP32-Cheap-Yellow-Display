#include "projectDisplay.h"

class SerialDisplay : public ProjectDisplay
{
public:
  void displaySetup()
  {
    Serial.println("Serial Display Setup");
  }

  void drawWifiManagerMessage(WiFiManager *myWiFiManager)
  {
    Serial.println("Entered Conf Mode");
  }
};
