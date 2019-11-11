#if !defined(CSensor_h)
#define CSensor_h
#include <string.h>
#include <Arduino.h>
class CSensor
{
private:
    /* data */
public:
    //CSensor();
    CSensor(int pmin, int pmax,bool penabled, int pinterval, const char* pname, const char* punit = "")
        : minVal(pmin), maxVal(pmax), enabled(penabled),interval(pinterval)
    {
        Serial.println("Creating sensor with values: ");
        minVal = pmin;
        Serial.println(minVal);
        maxVal = pmax;
        Serial.println(maxVal);
        Serial.println(enabled);
        Serial.println(interval);
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
    char status[64];
    virtual float getValue() {return 0;};
    ~CSensor(){};
};

#endif // Sensor_h