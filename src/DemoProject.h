#ifndef DemoProject_h
#define DemoProject_h

#include <AdminSettingsService.h>
#include "CSensor.h"
#include "CloudService.h"
#include "MQTTService.h"

#define BLINK_LED 2
#define MAX_DELAY 1000

#define DEMO_SETTINGS_FILE "/config/demoSettings.json"
#define DEMO_SETTINGS_PATH "/rest/demoSettings"


//enum class CloudService { http, mqtt, adafruit, IFTTT, thingsboard};

// For each service we need to define:
//  - url or endpoint
//  - Login and password (optional)
//  - timer
//  - linked sensor values (array) +++

class DemoProject : public AdminSettingsService {
 public:
    DemoProject(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);
    ~DemoProject();
    void reconfigureTheService();
    void onConfigUpdated();
    void loop();
    void start();
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
    MQTTService* cloudService;
    
    void readFromJsonObject(JsonObject& root);
    void writeToJsonObject(JsonObject& root);
  
};
#endif
