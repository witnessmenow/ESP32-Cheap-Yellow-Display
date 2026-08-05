#define PROJECT_CONFIG_JSON "/project_config.json"

#define PROJECT_TIME_TWENTY_FOUR_HOUR "twentyFourHour"
#define PROJECT_TIME_US_DATE "usDate"

class ProjectConfig
{
public:
  bool twentyFourHour = false;

  bool usDateFormat = false;

  bool fetchConfigFile()
  {
    if (SPIFFS.exists(PROJECT_CONFIG_JSON))
    {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(PROJECT_CONFIG_JSON, "r");
      if (configFile)
      {
        Serial.println("opened config file");
        StaticJsonDocument<1024> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("\nparsed json");

          if (json.containsKey(PROJECT_TIME_TWENTY_FOUR_HOUR))
          {
            twentyFourHour = json[PROJECT_TIME_TWENTY_FOUR_HOUR].as<bool>();
          }

          if (json.containsKey(PROJECT_TIME_US_DATE))
          {
            usDateFormat = json[PROJECT_TIME_US_DATE].as<bool>();
          }

          return true;
        }
        else
        {
          Serial.println("failed to load json config");
          return false;
        }
      }
    }

    Serial.println("Config file does not exist");
    return false;
  }

  bool saveConfigFile()
  {
    Serial.println(F("Saving config"));
    StaticJsonDocument<1024> json;
    json[PROJECT_TIME_TWENTY_FOUR_HOUR] = twentyFourHour;
    json[PROJECT_TIME_US_DATE] = usDateFormat;

    File configFile = SPIFFS.open(PROJECT_CONFIG_JSON, "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
      return false;
    }

    serializeJsonPretty(json, Serial);
    if (serializeJson(json, configFile) == 0)
    {
      Serial.println(F("Failed to write to file"));
      return false;
    }
    configFile.close();
    return true;
  }
};
