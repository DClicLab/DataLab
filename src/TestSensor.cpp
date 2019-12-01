#if !defined(TestSensor_c)
#define TestSensor_c
#include "CSensor.h"
#include <Arduino.h>

class TestSensor : public CSensor
{
private:
    /* data */

public:
    TestSensor(){};
    TestSensor( const CSensorParams &s ): CSensor(s){Serial.println("In testsensor constructor");};

    TestSensor(int minVal, int maxVal, bool enabled,int interval,const char* name, const char* unit, const char* driver) 
        : CSensor(minVal, maxVal, enabled, interval, name, unit, driver ){
            
        };
    void begin()
    {
        Serial.println("we are starting the sensor");
    };

    //This function is called to return the sensor value at every interval
    float getValue()
    {
        return random(minVal, maxVal);
    };

};

#endif // TestSensor