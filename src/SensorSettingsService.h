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

#define SENSOR_SETTINGS_FILE "/config/sensorSettings.json"
#define SENSOR_SETTINGS_ENDPOINT_PATH "/rest/sensorsState"
#define SENSOR_SETTINGS_SOCKET_PATH "/ws/sensorsState"

class SensorConfig {
 public:

  CSensor* sensorList[5];

  // Add all sensors here
  static constexpr  const char*  driverList[] = {
      BMP180Sensor::description,
      DHT11Sensor::description,
      TestSensor::description,
      BMP280Sensor::description,
      FreeMemSensor::description,
      AnalogInSensor::description
  };

  static void read(SensorConfig& settings, JsonObject& root);

  static CSensor* getSensor(JsonObject& sensorConf) {
    Serial.println("Adding sensor with conf");
  	serializeJsonPretty(sensorConf,Serial);

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
    // Add here your custom sensor
  }

  // Received updated settings from file/UI and update JsonObject
  static StateUpdateResult update(JsonObject& root, SensorConfig& settings) {
    for (size_t i = 0; i < (sizeof(sensorList) / sizeof(CSensor*)); i++) {
      if (settings.sensorList[i] != NULL) {
        settings.sensorList[i]->end();
        Serial.println("Deleting sensor");
        delete (settings.sensorList[i]);
        settings.sensorList[i] = NULL;
      }
    }
    int i = 0;
    Serial.println("Adding sensors from json conf:");
    JsonArray jsensors = root.getMember("sensors");
    serializeJsonPretty(jsensors,Serial);

    if (jsensors.size() > 0) {
      for (JsonObject jsensor : jsensors) {
        settings.sensorList[i] = getSensor(jsensor);
        i++;
      }
    }
    
    return StateUpdateResult::CHANGED;

  }
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
