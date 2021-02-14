#if !defined(CSensor_h)
#define CSensor_h
#include <string.h>
#include <Arduino.h>
#include <AsyncJson.h>
#include <map>


class CSensor //XXX add buffer to csensor
{
private:
    /* data */
public:
    CSensor();
    static constexpr const char* description="";
    CSensor(JsonObject& sensorConf)
        :enabled(sensorConf["enabled"].as<bool>()),interval(sensorConf["interval"].as<int>())
    {
        Serial.printf("Creating CSensor for %s\n",sensorConf["name"].as<char*>());
        strcpy(driver,sensorConf["driver"]["name"].as<char*>());
        strcpy(name,sensorConf["name"].as<char*>());        
    };
    //virtual void begin();
    bool enabled;
    int interval;
    char name[32];
    char driver[16];
    char config[512];

    const char* defaultConfig = "No default config";

    virtual float getValue() {return NAN;};
    virtual int getValuesAsJson(char* buffer) 
        {
            Serial.println("no json");
            return 0;
        }
    virtual void begin() {};
    virtual void loop() {};
    virtual void end() {};
    virtual ~CSensor(){};
};


#endif // Sensor_h