#include "Adafruit_Sensor.h"
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
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

class BMP280Sensor : public CSensor
{
private:
    Adafruit_BMP280 bme;
    short _sda = -1;
    short _scl = -1;
    uint _addr=76;
    bool running;
    TwoWire* wire;
public:

    static constexpr const char*  description = "{\"name\":\"BMP280\",\"config\":{\"sdaPin\":-1,\"sclPin\":-1,\"address\":\"0x76\"}}\"";

    BMP280Sensor(){};
    
    BMP280Sensor( JsonObject& sensorConf): CSensor(sensorConf){
        _sda = sensorConf["driver"]["config"]["sdaPin"].as<int>();
        _scl = sensorConf["driver"]["config"]["sclPin"].as<int>();
        
        Serial.println("ok till here");
        Serial.println(sensorConf["driver"]["config"]["address"].as<String>());
        Serial.println("ok till here");
        Serial.println((int)strtol(sensorConf["driver"]["config"]["address"].as<char*>(), NULL, 16));
        Serial.println("ok till here");

        _addr = (int)strtol(sensorConf["driver"]["config"]["address"].as<char*>(), NULL, 16);
        Serial.println("ok till here");
        Wire.begin(32,33);

    };

    void begin()
    {
        Serial.println("[BMP280Sensor] in begin()");
        // if(_sda != -1 && _scl != -1){
        //     Wire1.begin(_sda,_scl);
        // }
        running = bme.begin(0x76);
        if (!running)
        {
            Serial.printf("On sda:%d sdc:%d add:%#04x",_sda,_scl,_addr);
            Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        }else
        {
            Serial.println("BMP280 found!");
            /* code */
        }
        


          /* Default settings from datasheet. */
  bme.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    }

    // void getConfig(JsonObject& sensorConf){
    //     Serial.println("In getconfig for test.");
    //     CSensor::getConfig(sensorConf);//get base stuff,
    //     JsonObject config= sensorConf.createNestedObject("config");// add specific config
    //     config["sdaPin"] = _sda;
    //     config["sclPin"] = _scl;
    //     char buf[6];
    //     sprintf(buf,"%#04x",_addr);
    //     config["address"] = buf;
    //     sensorConf["default"] = defaultConfig;
    // }

     float getValue()
    {
        if (!running || (running = bme.begin(0x76)) )
        {
            Serial.println("Could not find a valid BMP280 sensor, check wiring!");
            return 0;
        }
    
    return bme.readTemperature();
    
}

int getValuesAsJson(char* dest){
        // if (!running || (running = bme.begin(0x76)) )       
        // {
        //     Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        //     // return 0;
        // }
    return sprintf(dest, "{\"pressure\":%f,\"temperature\":%f,\"altitude\":%f}",bme.readPressure(),bme.readTemperature(),bme.readAltitude(1013.25));
}

};


