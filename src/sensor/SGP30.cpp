#include <Wire.h>
#include "Adafruit_SGP30.h"

#include "CSensor.h"
class SGP30Sensor : public CSensor {
 private:
  /* data */
  Adafruit_SGP30 sgp;
  short _sda = -1;
  short _scl = -1;

 public:
  SGP30Sensor(){};
  SGP30Sensor(JsonObject& sensorConf) : CSensor(sensorConf) {
    // Extract configuration from the JsonObject
    // pin = sensorConf["config"]["pin"].as<int>();
    _sda = sensorConf["driver"]["config"]["sdaPin"].as<int>();
    _scl = sensorConf["driver"]["config"]["sclPin"].as<int>();
  };

  // Make sure to give your sensor class a unique name and a default full configuration matching the creator
  static constexpr const char* description = "{\"name\":\"SGP30\",\"config\":{\"sdaPin\":-1,\"sclPin\":-1}}";

  // This function is call after constructor
  void begin() {
    Wire.begin(_sda, _scl);
    sgp.begin();
  };

  void loop() {
    // do we need mesure? do we need to turn on the fan?
  }

  int getValuesAsJson(char* buffer) {
    if (!sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return -1;
    }

    if (! sgp.IAQmeasureRaw()) {
        Serial.println("Raw Measurement failed");
        return -1;
    }

    return sprintf(buffer, "{\"TVOC\":%d,\"eCO2\":%d,\"H2\":%d,\"Ethanol\":%d}", sgp.TVOC, sgp.eCO2, sgp.rawH2, sgp.rawEthanol);
  }
};