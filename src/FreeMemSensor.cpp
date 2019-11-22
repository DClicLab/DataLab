#if !defined(FreeMemSensor_c)
#define FreeMemSensor_c
#include "CSensor.h"
#include <Arduino.h>

class FreeMemSensor : public CSensor
{
private:
    /* data */

public:
    FreeMemSensor(int minVal, int maxVal, bool enabled, int interval, const char *name, const char *unit, const char *driver) : CSensor(minVal, maxVal, enabled, interval, name, unit, driver){};
    void begin()
    {
        Serial.println("we are starting the sensor");
    };
    float getValue()
    {
        return random(minVal, maxVal);
    };
};

#endif // FreeMemSensor
