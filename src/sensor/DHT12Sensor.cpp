#if !defined(DHT12Sensor_c)
#define DHT12Sensor_c

#include "CSensor.h"
#include <Arduino.h>
#include <Wire.h>
#include <DHT12.h>


class DHT12Sensor : public CSensor {
 private:
  /* data */
  short _sda = -1;
  short _scl = -1;
  DHT12* mydht12;
  uint8_t _addr;
 public:
 static constexpr const char* description = 
      "{\"name\":\"DHT12\",\"config\":{\"address\":\"0x5C\"},\"i2c\":1}";

  DHT12Sensor(JsonObject& sensorConf) : CSensor(sensorConf) {
    // _sda = sensorConf["driver"]["config"]["sdaPin"].as<int>();
    // _scl = sensorConf["driver"]["config"]["sclPin"].as<int>();
    _addr = (uint8_t)strtol(sensorConf["driver"]["config"]["address"].as<char*>(), NULL, 0);
    mydht12 = new DHT12(&Wire,_addr);
  };

  void begin() {
    // Wire.begin(_sda, _scl);
    Serial.printf("Starting DHT12 with Wire as %d %d and addres to ");
    mydht12->begin();
    // pinMode(pin, OUTPUT);
    
  };

  int getValuesAsJson(char* buffer) {
    float h = mydht12->readHumidity();
    float t = mydht12->readTemperature();
    Serial.printf("DHT returned %f %f", h, t);

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      mydht12->begin();
      return 0;
    }
    sprintf(buffer, "{\"humidity\":%f,\"temperature\":%f}", h, t);
    return 2;
  }
  // This function is called to return the sensor value at every interval
  float getValue() {
    
    // Read temperature as Celsius (the default)
    float t = mydht12->readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return -1;
    }
    return t;
  };
};

#endif  // DHT12Sensor