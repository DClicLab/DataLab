#ifndef DataLab_h
#define DataLab_h

#include <AdminSettingsService.h>
#include "CSensor.h"
#include "CloudService.h"

#include "MQTTService.h"
#include "HTTPService.h"


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

class DataLab : public AdminSettingsService {
 public:
    DataLab(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);
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
    //CSensor* getSensor(const char* driverName,CSensorParams params);
    CSensor* getSensor(JsonObject& config);
    
  private:
    void static getValueForSensor(int i);
    time_t getNow();
    unsigned long _lastBlink = 0;
    uint8_t _blinkSpeed = 255;
    
  protected:
    CSensor* sensorList[5];
    //CSensorParams* sensorParamsList[5];
    CloudService* cloudService;
    
    void readFromJsonObject(JsonObject& root);
    void writeToJsonObject(JsonObject& root);
  
};
#endif
