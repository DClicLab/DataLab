#if !defined(HM3301Sensor_c)
#define HM3301Sensor_c
// For HM3301 Laser PM2.5 sensor
// https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/

//Include the sensor class
#include "CSensor.h"

//Then all other one you would need
#include <Arduino.h>
#include <Tomoto_HM330X.h>

// AirQualityIndex class from Tomoto Shimizu https://github.com/tomoto/Arduino_Tomoto_HM330X
#include "AirQualityIndex.h"

//Your sensor class your be based on CSensor
class HM3301Sensor : public CSensor
{
private:
    /* data */
    Tomoto_HM330X sensor;

public:
    HM3301Sensor(){};
    HM3301Sensor( JsonObject& sensorConf): CSensor(sensorConf){
        //Extract configuration from the JsonObject
        //pin = sensorConf["config"]["pin"].as<int>();
        
    };

    //Make sure to give your sensor class a unique name and a default full configuration matching the creator
    static constexpr const char*  description = "{\"name\":\"HM3301\",\"conf\":{\"pin\":36}}";

    //This function is call after constructor
    void begin()
    {
        Wire.begin(32,33);/// just for dev
        sensor.begin();
    };

    void loop(){
        //do we need mesure? do we need to turn on the fan?

    }

    int getValuesAsJson(char* buffer) 
    {
        if(!sensor.readSensor()){
            Serial.println(F("Failed to read from HM3301 sensor!"));
            sensor.begin();
            return 0;
           
        };
        const int pm1 = sensor.atm.getPM1();
        const int pm2_5 = sensor.atm.getPM2_5();
        const int pm10 = sensor.atm.getPM10();
        const int afpm = AirQualityIndex::AtmofromPM(pm2_5,pm10);
        Serial.printf("HM3301 returned pm1= %d pm2_5=%d pm10=%d afpm=%d",pm1,pm2_5,pm10,afpm);

        sprintf(buffer,"{\"pm1\":%d,\"pm2_5\":%d,\"pm10\":%d,\"afpm\":%d}",pm1,pm2_5,pm10,afpm);
        return 2;
    }

};

#endif // HM3301Sensor