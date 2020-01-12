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
        minVal = sensorConf["min"].as<int>();
        maxVal = sensorConf["max"].as<int>();
    };

    void begin()
    {
        Serial.printf("we are starting the random sensor with min: %d and max:%d\n",minVal,maxVal);
    };

    void getConfig(JsonObject& sensorConf){
        sensorConf["min"] = minVal;
        sensorConf["max"] = maxVal;
    }

    //This function is called to return the sensor value at every interval
    float getValue()
    {
        return random(minVal, maxVal);
    };

};

#endif // TestSensor