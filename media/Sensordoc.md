## How to create a Sensor

*We will take the VL53L0X ToF sensor as exemple.*
1. Make sure you include the required libraries in the `platformio.ini` file:

```ini
lib_deps = 
	Adafruit_VL53L0X
```

2. Start from a simple example file from your librairy. Here, `Adafruit_VL53L0X/examples/vl53l0x/vl53l0x.ino`

3. Copy the code from `src\sensor\TestSensor.h` to your sensor name. Here `src\sensor\vl5310x.ino`

4. Replace everywhere `TestSensor` with your sensor name. Here `Vlx53l0x`. And include the required libraries. Here: 
```cpp
//Keep them
#include "CSensor.h"
#include <Arduino.h>
//And add yours:
#include "Adafruit_VL53L0X.h"
```

5. Your example might have global variable. Place them in the class as private. Here:
```cpp
class Vlx53l0x : public CSensor
{
private:
    Adafruit_VL53L0X lox = Adafruit_VL53L0X();
```



5. Your sensor will probably need some variables to be configured by the user (eg GPIO ports numbers, min, max etc...)
> Place your requiered variables in the private section of the class of your sensor.

```cpp
class Vlx53l0x : public CSensor
{
private:
    Adafruit_VL53L0X lox = Adafruit_VL53L0X();
    int scl;
    int sda;

public:
...
```

6. Adjust the constructor of your sensor to initialize the values from the user interface/config file: 

```cpp
    Vlx53l0x( JsonObject& sensorConf): CSensor(sensorConf){
        sda = sensorConf["driver"]["config"]["sda"].as<int>();
        scl = sensorConf["driver"]["config"]["scl"].as<int>();
    };
```

7. From your example, put the `setup()` content in the `begin()` method.
```cpp
    void begin()
    {
        lox.begin();
        Serial.printf("we are starting the random sensor with min: %d and max:%d\n",minVal,maxVal);
    };
```
8. Now integrate your sensor in `src\SensorSettingsService.h`:
> 1. Include your sensor file:
```cpp
#include "sensor/TestSensor.h"
#include "sensor/vl53l0x.h"//Here
#define SENSOR_SETTINGS_FILE "/config/sensorSettings.json"

```

> 2. Include your description:
```cpp
  static constexpr  const char*  driverList[] = {
      Vlx53l0x::description, // Here
      BMP180Sensor::description,
...
```

> 3. Include the name correspondance: The name must match the one on your `description` property.
```cpp
    // Add here your custom sensors
    if (strcmp(sensorConf["driver"]["name"], "ToF") == 0) {
      return new Vlx53l0x(sensorConf);
    }

```