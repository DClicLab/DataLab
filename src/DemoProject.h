#ifndef DemoProject_h
#define DemoProject_h

#include <AdminSettingsService.h>
#include "CSensor.h"

#define BLINK_LED 2
#define MAX_DELAY 1000

#define DEFAULT_BLINK_SPEED 100
#define DEMO_SETTINGS_FILE "/config/demoSettings.json"
#define DEMO_SETTINGS_PATH "/rest/demoSettings"

// struct SensorConf
// {
//   char name[32];
//   uint8_t index;
//   char type[12];//TODO convert to enum
//   char extra[64];
//   bool enabled;
//   int min;
//   int max;
//   int interval;// in sec
// };


enum class CloudService { http, mqtt, adafruit, IFTTT, thingsboard};

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
    
  private:
    //struct SensorConf _confSensorList[5];
    CSensor* sensorList[5];
    void static getValueForSensor(int i);
    unsigned long _lastBlink = 0;
    uint8_t _blinkSpeed = 255;
    
  protected:
    
    void readFromJsonObject(JsonObject& root);
    void writeToJsonObject(JsonObject& root);
  
};




#endif
