#if !defined(TestSensor_c)
#define TestSensor_c
#include "CSensor.h"
#include <Arduino.h>

class TestSensor : public CSensor
{
private:
    /* data */
    int minVal;
    int maxVal;

public:
    TestSensor(){};
    TestSensor( JsonObject& sensorConf): CSensor(sensorConf){
        minVal = sensorConf["config"]["min"].as<int>();
        maxVal = sensorConf["config"]["max"].as<int>();
        Serial.printf("Creating random sensor with min: %d and max:%d\n",minVal,maxVal);
    };
    
    static constexpr const char*  description = "{\"name\":\"Random\",\"conf\":{\"min\":0,\"max\":95}}\"";
    
    void begin()
    {
        Serial.printf("we are starting the random sensor with min: %d and max:%d\n",minVal,maxVal);
    };

    //This function is called to return the sensor value at every interval
    float getValue()
    {
        return random(minVal, maxVal);
    };

};

#endif // TestSensor