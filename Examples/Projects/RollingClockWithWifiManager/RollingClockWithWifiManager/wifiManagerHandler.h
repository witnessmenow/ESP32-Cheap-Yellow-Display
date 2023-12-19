
DoubleResetDetector *drd;
ProjectDisplay *wm_Display;

// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  wm_Display->drawWifiManagerMessage(myWiFiManager);
}

void setupWiFiManager(bool forceConfig, ProjectConfig config, ProjectDisplay *theDisplay)
{
  wm_Display = theDisplay;
  WiFiManager wm;
  // set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  // set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  WiFiManagerParameter timeZoneParam(PROJECT_TIME_ZONE_LABEL, "Time Zone", config.timeZone.c_str(), 60);

  char checkBox[] = "type=\"checkbox\"";
  char checkBoxChecked[] = "type=\"checkbox\" checked";
  char *customHtml;

  if (config.twentyFourHour)
  {
    customHtml = checkBoxChecked;
  }
  else
  {
    customHtml = checkBox;
  }
  WiFiManagerParameter isTwentyFourHour(PROJECT_TIME_TWENTY_FOUR_HOUR, "24H Clock", "T", 2, customHtml);

  char *customHtmlTwo;
  if (config.usDateFormat)
  {
    customHtmlTwo = checkBoxChecked;
  }
  else
  {
    customHtmlTwo = checkBox;
  }
  WiFiManagerParameter isUsDateFormat(PROJECT_TIME_US_DATE, "US Date Format", "T", 2, customHtmlTwo);

  wm.addParameter(&timeZoneParam);
  wm.addParameter(&isTwentyFourHour);
  wm.addParameter(&isUsDateFormat);

  if (forceConfig)
  {
    // IF we forced config this time, lets stop the double reset so it doesn't get stuck in a loop
    drd->stop();
    if (!wm.startConfigPortal("esp32Project", "brianlough"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("esp32Project", "brianlough"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  // save the custom parameters to FS
  if (shouldSaveConfig)
  {

    config.timeZone = String(timeZoneParam.getValue());
    config.twentyFourHour = (strncmp(isTwentyFourHour.getValue(), "T", 1) == 0);
    config.usDateFormat = (strncmp(isUsDateFormat.getValue(), "T", 1) == 0);

    config.saveConfigFile();
    drd->stop();
    ESP.restart();
    delay(5000);
  }
}
