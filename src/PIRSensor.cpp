#if !defined(PIRSensor_c)
#define PIRSensor_c
// For M5SticC PIR HAT AS312
//Include the sensor class
#include "CSensor.h"

//Then all other one you would need
#include <Arduino.h>


//Your sensor class your be based on CSensor
class PIRSensor : public CSensor
{
private:
    /* data */
    int pin;

public:
    PIRSensor(){};
    PIRSensor( JsonObject& sensorConf): CSensor(sensorConf){
        //Extract configuration from the JsonObject
        pin = sensorConf["config"]["pin"].as<int>();
    };

    //Make sure to give your sensor class a unique name and a default full configuration matching the creator
    static constexpr const char*  description = "{\"name\":\"PIR\",\"conf\":{\"pin\":36}}";

    //This function is call after constructor
    void begin()
    {
        pinMode(pin,INPUT_PULLUP);
    };

    void loop(){
        //do we need mesure? do we need to turn on the fan?

    }

    //This function is called to return the sensor value at every interval
    float getValue()
    {
        return digitalRead(pin);
    };

};

#endif // PIRSensor