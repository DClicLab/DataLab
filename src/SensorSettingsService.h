#ifndef SensorService_h
#define SensorService_h

// #include <LightMqttSettingsService.h>
#include <LITTLEFS.h>

#include <HttpEndpoint.h>
#include <MqttPubSub.h>
#include <WebSocketTxRx.h>
#include <FSPersistence.h>
#include <sensor/CSensor.h>
#include "sensor/BMP280.cpp"
#include "sensor/BMPsensor.cpp"
#include "sensor/DHT11Sensor.cpp"
#include "sensor/FreeMemSensor.cpp"
#include "sensor/AnalogInSensor.cpp"
#include "sensor/TestSensor.h"
#include "sensor/vl53l0x.h"
#define SENSOR_SETTINGS_FILE "/config/sensorSettings.json"
#define SENSOR_SETTINGS_ENDPOINT_PATH "/rest/sensorsState"
#define SENSOR_SETTINGS_SOCKET_PATH "/ws/sensorsState"

extern bool SEMbusy;

class SensorConfig {
 public:

  static CSensor* sensorList[5];
  // static StaticJsonDocument<4096> jsonState;
  static char jsonstring[4096];

  // Add all sensors here
  static constexpr  const char*  driverList[] = {
      Vlx53l0x::description,
      BMP180Sensor::description,
      DHT11Sensor::description,
      TestSensor::description,
      BMP280Sensor::description,
      FreeMemSensor::description,
      AnalogInSensor::description,
  };


  static CSensor* getSensor(JsonObject& sensorConf) {
    Serial.println("Adding sensor with conf");
  	serializeJsonPretty(sensorConf,Serial);

    // Add here your custom sensors
    if (strcmp(sensorConf["driver"]["name"], "ToF") == 0) {
      return new Vlx53l0x(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "Random") == 0) {
      return new TestSensor(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "BMP") == 0) {
      return new BMP180Sensor(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "BMP280") == 0) {
      return new BMP280Sensor(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "DHT11") == 0) {
      return new DHT11Sensor(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "FreeMem") == 0) {
      return new FreeMemSensor(sensorConf);
    }
    if (strcmp(sensorConf["driver"]["name"], "AnalogIn") == 0) {
      return new AnalogInSensor(sensorConf);
    }

    return new TestSensor(sensorConf);
  }

  // Received updated settings from file/UI and update JsonObject
  static StateUpdateResult update(JsonObject& root, SensorConfig& settings) ;

  static void read(SensorConfig& settings, JsonObject& root);

};

class SensorSettingsService : public StatefulService<SensorConfig> {
 public:
  SensorSettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);
  void begin();
  SensorConfig config;

 private:
  HttpEndpoint<SensorConfig> _httpEndpoint;
  WebSocketTxRx<SensorConfig> _webSocket;
  FSPersistence<SensorConfig> _fsPersistence;

  void registerConfig();
  void onConfigUpdated();
};

#endif
