#ifndef DataLab_h
#define DataLab_h

// #include <AdminSettingsService.h>
// #include "sensor/CSensor.h"
// #include "CloudService.h"
#include "SensorSettingsService.h"

// #include "MQTTService.h"
// #include "HTTPService.h"

#define DATA_SETTINGS_FILE "/config/dataSettings.json"
#define DATA_SETTINGS_PATH "/rest/dataSettings"

#ifdef ARDUINO_M5Stick_C_Plus
#include <M5StickCPlus.h>
#define HASRTC
#define ISM5
#elif ARDUINO_M5Stick_C
#define HASRTC
#define ISM5
#include <M5StickC.h>
#endif

//enum class CloudService { http, mqtt, adafruit, IFTTT, thingsboard};

// For each service we need to define:
//  - url or endpoint
//  - Login and password (optional)
//  - timer
//  - linked sensor values (array) +++


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
    void static onWSEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
    
  protected:
    CSensor* sensorList[5];
    SensorSettingsService* sensorsSS;
    AsyncWebSocket ws = AsyncWebSocket("/ws/sensorValue");

    // CloudService* cloudService;
    
  
};
#endif
