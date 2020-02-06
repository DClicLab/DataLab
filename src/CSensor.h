#if !defined(CSensor_h)
#define CSensor_h
#include <string.h>
#include <Arduino.h>
#include <AsyncJson.h>
#include <map>


// class CSensorParams
// {
//     public:
//     int minVal;
//     int maxVal;
//     bool enabled;
//     int interval;
//     char name[32];
//     char unit[5];
//     char driver[16];

//     CSensorParams(int pmin, int pmax,bool penabled, int pinterval, const char* pname, const char* punit = "", const char* pdriver = "random")
//         : minVal(pmin), maxVal(pmax), enabled(penabled),interval(pinterval){
//         Serial.println("Creating sensorparams: ");
//             strcpy(driver,pdriver);
//             strcpy(name,pname);
//             strcpy(unit,punit);
//         }
    
// };

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
        strcpy(driver,sensorConf["driver"].as<char*>());
        strcpy(name,sensorConf["name"].as<char*>());        
    };
    //virtual void begin();
    bool enabled;
    int interval;
    char name[32];
    char driver[16];
    char config[512];

    const char* defaultConfig = "No default config";

    // virtual void getConfig(JsonObject& sensorConf){
    //     sensorConf["driver"]= driver;
    //     sensorConf["name"]= name;
    //     sensorConf["interval"]= interval;
    //     sensorConf["enabled"]= enabled;
    //     //sensorConf["config"]= config;
    // }

    virtual float getValue() {return 0;};
    virtual int getValuesAsJson(char* buffer) 
        {
            Serial.println("no json");
            return 0;
        }
    virtual void begin() {};
    virtual ~CSensor(){};
};

#endif // Sensor_h