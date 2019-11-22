#if !defined(CSensor_h)
#define CSensor_h
#include <string.h>
#include <Arduino.h>
#include <map>


class CSensorParams
{
    public:
    int minVal;
    int maxVal;
    bool enabled;
    int interval;
    char name[32];
    char unit[5];
    char driver[16];

    CSensorParams(int pmin, int pmax,bool penabled, int pinterval, const char* pname, const char* punit = "", const char* pdriver = "random")
        : minVal(pmin), maxVal(pmax), enabled(penabled),interval(pinterval){
        Serial.println("Creating sensorparams: ");
            strcpy(driver,pdriver);
            strcpy(name,pname);
            strcpy(unit,punit);
        }
    
};


class CSensor
{
private:
    /* data */
public:
    CSensor();
    CSensor( const CSensorParams &s ): minVal(s.minVal),maxVal(s.maxVal),interval(s.interval)
    { 
        strcpy(driver,s.driver);
        strcpy(unit,s.unit);
        strcpy(name,s.name);
    } 

    CSensor(int pmin, int pmax,bool penabled, int pinterval, const char* pname, const char* punit = "", const char* pdriver = "random")
        : minVal(pmin), maxVal(pmax), enabled(penabled),interval(pinterval)
    {
        Serial.println("Creating sensor with values: ");
        Serial.println(minVal);
        Serial.println(maxVal);
        Serial.println(enabled);
        Serial.println(interval);
        strcpy(driver,pdriver);
        strcpy(name,pname);
        strcpy(unit,punit);
    };
    //virtual void begin();
    int minVal;
    int maxVal;
    bool enabled;
    int interval;
    char name[32];
    char unit[5];
    char driver[16];
    char status[64];
    virtual float getValue() {return 0;};
    ~CSensor(){};
};



#endif // Sensor_h