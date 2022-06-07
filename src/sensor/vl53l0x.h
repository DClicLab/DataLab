#if !defined(Vlx53l0x_c)
#define Vlx53l0x_c
#include "CSensor.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"

class Vlx53l0x : public CSensor {
 private:
  Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  int _addr;

 public:
  Vlx53l0x(JsonObject& sensorConf) : CSensor(sensorConf) {
    _addr = (int)strtol(sensorConf["driver"]["config"]["address"].as<char*>(), NULL, 0);
  };

  static constexpr const char* description = "{\"name\":\"ToF\",\"config\":{\"address\":\"0x29\"},\"i2c\":1}\"";

  void begin() {
    lox.begin(_addr);
  };

  // This function is called to return the sensor value at every interval
  float getValue() {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure);
    return measure.RangeMilliMeter;
  };
};

#endif  // Vlx53l0x