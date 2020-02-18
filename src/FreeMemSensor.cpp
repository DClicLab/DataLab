#if !defined(FreeMemSensor_c)
#define FreeMemSensor_c
#include "CSensor.h"
#include <Arduino.h>

class FreeMemSensor : public CSensor
{
private:
    /* data */

public:
    FreeMemSensor(JsonObject& sensorConf): CSensor(sensorConf){
        //no need for extra config
    };

    void begin()
    {
        Serial.println("we are starting the sensor");
    };
    float getValue()
    {
        return ESP.getFreeHeap();
    };
};
#endif // FreeMemSensor