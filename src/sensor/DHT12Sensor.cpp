#if !defined(DHT12Sensor_c)
#define DHT12Sensor_c

#include "CSensor.h"
#include <Arduino.h>

#include <DHT12.h>


class DHT12Sensor : public CSensor {
 private:
  /* data */
  short _sda = -1;
  short _scl = -1;
  DHT12 dht12;
  uint _addr;
 public:
 static constexpr const char* description = 
      "{\"name\":\"DHT12\",\"config\":{\"sdaPin\":-1,\"sclPin\":-1,\"address\":\"0x5C\"}}";

  DHT12Sensor(JsonObject& sensorConf) : CSensor(sensorConf) {
    // _sda = sensorConf["driver"]["config"]["sdaPin"].as<int>();
    // _scl = sensorConf["driver"]["config"]["sclPin"].as<int>();
    _addr = (int)strtol(sensorConf["driver"]["config"]["address"].as<char*>(), NULL, 0);
    
  };

  void begin() {
    // Wire.begin(_sda, _scl);

    // dht12.begin();
    // pinMode(pin, OUTPUT);
    
  };

  int getValuesAsJson(char* buffer) {
    float h = dht12.readHumidity();
    float t = dht12.readTemperature();
    Serial.printf("DHT returned %f %f", h, t);

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      dht12.begin();
      return 0;
    }
    sprintf(buffer, "{\"humidity\":%f,\"temperature\":%f}", h, t);
    return 2;
  }
  // This function is called to return the sensor value at every interval
  float getValue() {
    
    // Read temperature as Celsius (the default)
    float t = dht12.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return -1;
    }
    return t;
  };
};

#endif  // DHT12Sensor