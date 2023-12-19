
#ifndef PROJECTDISPLAY_H
#define PROJECTDISPLAY_H

class ProjectDisplay
{
public:
  virtual void displaySetup() = 0;

  virtual void drawWifiManagerMessage(WiFiManager *myWiFiManager) = 0;

  void setWidth(int w)
  {
    screenWidth = w;
    screenCenterX = screenWidth / 2;
  }

  void setHeight(int h)
  {
    screenHeight = h;
  }

protected:
  int screenWidth;
  int screenHeight;
  int screenCenterX;
};
#endif
