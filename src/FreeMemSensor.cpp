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
    static constexpr const char*  description = "{\"name\":\"FreeMem\",\"conf\":{}}\"";

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