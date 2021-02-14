#ifndef DataLab_h
#define DataLab_h

// #include <AdminSettingsService.h>
// #include "sensor/CSensor.h"
#include "SensorSettingsService.h"
// #include "CloudService.h"

// #include "MQTTService.h"
// #include "HTTPService.h"


#define BLINK_LED 2
#define MAX_DELAY 1000

#define DATA_SETTINGS_FILE "/config/dataSettings.json"
#define DATA_SETTINGS_PATH "/rest/dataSettings"


//enum class CloudService { http, mqtt, adafruit, IFTTT, thingsboard};

// For each service we need to define:
//  - url or endpoint
//  - Login and password (optional)
//  - timer
//  - linked sensor values (array) +++


class DummySettings {
 
};

class DataLab  {
 public:
    // DataLab(AsyncWebServer* server, FS* fs, SensorSettingsService sensorSettings);
    DataLab(AsyncWebServer* server, SensorSettingsService* sensorSettings);
    ~DataLab();
    void reconfigureTheService();
    void onConfigUpdated();
    void loop();
    void start();
    void applyDefaultConfig();
    void addDriversToJsonObject(JsonObject& root);
    void saveConf(JsonObject& root);
    void readConf(JsonObject& root);

    void processPV(const char* keyname, time_t now,float val);
    CSensor* getSensor(JsonObject& config);
    
  private:
    void static getValueForSensor(int i);
    time_t getNow();
    
  protected:
    CSensor* sensorList[5];
    SensorSettingsService* sensorsSS;
    // CloudService* cloudService;
    
  
};
#endif
