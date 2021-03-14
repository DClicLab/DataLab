#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "CSensor.h"
/***************************************************
  This is an example for the BMP085 Barometric Pressure & Temp Sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> https://www.adafruit.com/products/391

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// On ESP32 ;
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here

class BMP180Sensor : public CSensor {
 private:
  Adafruit_BMP085 bmp;
  ushort _sda = -1;
  ushort _scl = -1;
  bool running;

 public:
  static constexpr const char* description = "{\"name\":\"BMP180\",\"config\":{\"sdaPin\":-1,\"sclPin\":-1}}\"";

  BMP180Sensor(){};

  BMP180Sensor(JsonObject& sensorConf) : CSensor(sensorConf) {
    // no need for extra config
    _sda = sensorConf["driver"]["config"]["sdaPin"].as<int>();
    _scl = sensorConf["driver"]["config"]["sclPin"].as<int>();
  };

  void begin() {
    Serial.println("[BMPSensor] in begin()");
    if (_sda != -1 && _scl != -1) {
      Wire.begin(_sda, _scl);
    }
    running = bmp.begin();
    if (!running) {
      Serial.printf("On sda:%d sdc:%d : ", _sda, _scl);
      Serial.println("Could not find a valid BMP085/180 sensor, check wiring!");
    }
  }
  // void getConfig(JsonObject& sensorConf){
  //     Serial.println("In getconfig for test.");
  //     CSensor::getConfig(sensorConf);//get base stuff,
  //     JsonObject config= sensorConf.createNestedObject("config");// add specific config
  //     config["sdaPin"] = _sda;
  //     config["sclPin"] = _scl;
  //     sensorConf["default"] = defaultConfig;
  // }

  float getValue() {
    if (!running || (running = bmp.begin())) {
      Serial.println("Could not find a valid BMP085/180 sensor, check wiring!");
      return 0;
    }

    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");

    // you can get a more precise measurement of altitude
    // if you know the current sea level pressure which will
    // vary with weather and such. If it is 1015 millibars
    // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");

    Serial.println();
    return bmp.readTemperature();
  }

  int getValuesAsJson(char* dest) {
    if (!running || (running = bmp.begin())) {
      Serial.println("Could not find a valid BMP085 sensor, check wiring!");
      return 0;
    }
    return sprintf(dest,
                   "{\"pressure\":%d,\"temperature\":%f,\"altitude\":%f}",
                   bmp.readPressure(),
                   bmp.readTemperature(),
                   bmp.readAltitude());
  }
};
