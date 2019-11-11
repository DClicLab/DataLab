#if !defined(TestSensor_c)
#define TestSensor_c
#include "CSensor.h"
#include <Arduino.h>

class TestSensor : public CSensor
{
private:
    /* data */

public:
    TestSensor(int minVal, int maxVal, bool enabled,int interval,const char* name, const char* unit) : CSensor(minVal, maxVal, enabled, interval, name, unit ){};
    void begin()
    {
        Serial.println("we are starting the sensor");
    };
    float getValue()
    {
        return random(minVal, maxVal);
    };
};

#endif // TestSensor
