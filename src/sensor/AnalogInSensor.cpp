#if !defined(AnalogIn)
#define AnalogIn

// Include the sensor class
#include "CSensor.h"

// Then all other one you would need
#include <Arduino.h>

// Your sensor class your be based on CSensor
class AnalogInSensor : public CSensor {
 private:
  /* data */
  int pin;

 public:
  AnalogInSensor(){};
  AnalogInSensor(JsonObject& sensorConf) : CSensor(sensorConf) {
    // Extract configuration from the JsonObject
    pin = sensorConf["driver"]["config"]["pin"].as<int>();
  };

  // Make sure to give your sensor class a unique name and a default full configuration matching the creator
  static constexpr const char* description = "{\"name\":\"AnalogIn\",\"config\":{\"pin\":0}}\"";

  // This function is call after constructor
  void begin() {
    pinMode(pin, INPUT);
  };

  void loop() {
    // do we need mesure? do we need to turn on the fan?
  }

  // This function is called to return the sensor value at every interval
  float getValue() {
    return analogRead(pin);
  };
};

#endif  // AnalogInSensor