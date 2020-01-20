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
    
    const char* defaultConfig = "{\"min\":0,\"max\":95}";

    void begin()
    {
        Serial.printf("we are starting the random sensor with min: %d and max:%d\n",minVal,maxVal);
    };

    void getConfig(JsonObject& sensorConf){
        Serial.println("In getconfig for test.");
        CSensor::getConfig(sensorConf);//get base stuff,
        JsonObject config= sensorConf.createNestedObject("config");// add specific config
        config["min"] = minVal;
        config["max"] = maxVal;
        sensorConf["default"] = defaultConfig;
    }

    //This function is called to return the sensor value at every interval
    float getValue()
    {
        return random(minVal, maxVal);
    };

};

#endif // TestSensor