#if !defined(DHTSensor_c)
#define DHTSensor_c

#include "CSensor.h"
#include <Arduino.h>

#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 33
#define DHTTYPE DHT11

//DHT dhtSensor(DHTPIN, DHTTYPE);

// Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11

class DHT11Sensor : public CSensor
{
private:
    /* data */
    DHT dhtSensor = DHT(3,DHT11); //Pin is not use here, pin is set in constructor.
    uint32_t delayMS;
    int pin;

public:
    
    DHT11Sensor( JsonObject& sensorConf): CSensor(sensorConf){
        pin = sensorConf["pin"].as<int>();
        dhtSensor = DHT(pin, DHT11,6);
    };

    void begin()
    {
        pinMode(pin, OUTPUT);
        Serial.println("we are starting the DHT11 sensor");
        dhtSensor.begin();
    };


    int getValuesAsJson(char* buffer) 
    {
        float h = dhtSensor.readHumidity();
        float t = dhtSensor.readTemperature();
        if (isnan(h) || isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
            return 0;
        }
        sprintf(buffer,"{\"humidity\":%f,\"temperature\":%f}",h,t);
        return 2;
    }
    //This function is called to return the sensor value at every interval
    float getValue()
    {
        // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float h = dhtSensor.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dhtSensor.readTemperature();

        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
            return -1;
        }
        Serial.print(F("Humidity: "));
        Serial.print(h);
        Serial.print(F("%  Temperature: "));
        Serial.print(t);
        Serial.print(F("°C "));
        return t;
    };
};

#endif // DHT11Sensor