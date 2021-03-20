#if !defined(Vlx53l0x_c)
#define Vlx53l0x_c
#include "CSensor.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"

class Vlx53l0x : public CSensor {
 private:
  Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  int scl;
  int sda;
  TwoWire I2CVL = TwoWire(0);

 public:
  Vlx53l0x(){};
  Vlx53l0x(JsonObject& sensorConf) : CSensor(sensorConf) {
    sda = sensorConf["driver"]["config"]["sda"].as<int>();
    scl = sensorConf["driver"]["config"]["scl"].as<int>();
  };

  static constexpr const char* description = "{\"name\":\"ToF\",\"config\":{\"sda\":32,\"scl\":33}}\"";

  void begin() {
    I2CVL.begin(sda, scl);
    lox.begin(VL53L0X_I2C_ADDR, true, &I2CVL);
  };

  // This function is called to return the sensor value at every interval
  float getValue() {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, true);
    return measure.RangeMilliMeter;
  };
};

#endif  // Vlx53l0x