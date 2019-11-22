#include "DemoProject.h"
#include "TestSensor.cpp"
#include "FreeMemSensor.cpp"
#include <Ticker.h>
#include <Wire.h>
#include "DHT11Sensor.cpp"
#include <string>

using namespace std;

//Ticker* ticks[5];
//CSensor* sensorList[5];
Ticker ticks[5];
stack<int> queue; //Sensors needed to be pulled

const size_t CAPACITY = JSON_OBJECT_SIZE(10);
StaticJsonDocument<CAPACITY> doc;
JsonObject sensorsJValues =doc.to<JsonObject>();

// template<typename T> CSensor * createInstance(CSensorParams params) { return new T(params); }
// typedef CSensor* (*mapT)();

// typedef std::map<std::string,mapT> mapSensor;
// mapSensor test;

const char* driverList[]={"random","dht11temp"};

CSensor* DemoProject::getSensor(const char* driverName,CSensorParams params){
  if (strcmp(driverName, "random") == 0){
    return new TestSensor(params);
  }
  if (strcmp(driverName, "dht11temp") == 0){
    return new DHT11Sensor(params);
  }
  
  return new TestSensor(params);
  //Add here your custom sensor
}



JsonObject getSensorJValues(){  
  return sensorsJValues;
}


DemoProject::DemoProject(AsyncWebServer *server, FS *fs, SecurityManager *securityManager) : AdminSettingsService(server, fs, securityManager, DEMO_SETTINGS_PATH, DEMO_SETTINGS_FILE)
{
  Serial.println("Starting Demo");

  //test.emplace("Test sensor",&createInstance<TestSensor>);
  pinMode(BLINK_LED, OUTPUT);
  memset(sensorList, 0, sizeof(sensorList));
  //set http
  server->on("/val", HTTP_GET, [](AsyncWebServerRequest *request) {


    AsyncJsonResponse * response = new AsyncJsonResponse(MAX_SETTINGS_SIZE);
    JsonObject jsonObject = response->getRoot(); 
      
    jsonObject.set(getSensorJValues());
    response->setLength();
    request->send(response);

  });
}

void DemoProject::start()
{
  Serial.println("starting conf");
  // init all sensors
  int i = 0;
  Serial.println("instanciating test sensors");
  for (CSensor *sensorc : sensorList)
  {
    Serial.println("Adding sensor...");
    if (sensorc == NULL)
    {
      Serial.println("Skipping not set");
      continue;
    }
    Serial.println("Adding test, interval");
    Serial.println(sensorc->interval);
    Serial.println(sensorc->name);

    //Instantiate the sensor based on the type attribute
    //TODO: Implement a sensor factory with self-registering sensors.
    sensorList[i] = getSensor(sensorList[i]->driver,CSensorParams(sensorc->minVal, sensorc->maxVal, sensorc->enabled, sensorc->interval, sensorc->name, sensorc->unit, sensorc->driver));
    
    ticks[i].attach<int>(sensorc->interval, getValueForSensor, i);
    i++;
  }
  Serial.println("Done Adding sensors");
  // do whatever is required to react to the new settings
}
void DemoProject::onConfigUpdated()
{
  reconfigureTheService();
}

void DemoProject::reconfigureTheService()
{
  Serial.println("reconfig");
  // do whatever is required to react to the new settings
}

void DemoProject::getValueForSensor(int i)
{
  //adding sensor to queue as we should not block in timer call
  queue.push(i);
}

DemoProject::~DemoProject()
{
}

void DemoProject::loop()
{
  while (queue.size() > 0)
  {
    Serial.println(queue.top());
    Serial.print("Sensor is ");
    Serial.println(sensorList[queue.top()]->enabled);

    if (sensorList[queue.top()]->enabled)
    {
      Serial.println("getting val for sensor");
      Serial.println(sensorList[queue.top()]->name);
      Serial.print("Value:");
      Serial.println(sensorList[queue.top()]->getValue());
      //doesnt work...
      sensorsJValues[sensorList[queue.top()]->name] = sensorList[queue.top()]->getValue();
      sensorsJValues["last"] = sensorList[queue.top()]->getValue();
    }
    String ret;
    serializeJson(doc, ret);
    Serial.println("ret");
    Serial.println(ret);
  


    queue.pop();
  }
  
}

void DemoProject::readFromJsonObject(JsonObject &root)//create the local conf from JSON POST
{
  Serial.println("in read from json");
  int i = 0;
  JsonArray jsensors = root.getMember("sensors");
  memset(sensorList,0,sizeof(sensorList));
  //Deserializing sensors
  // deserialisation needed so that JsonBuffer is kept in memory as short as possible
  for (JsonObject jsensor : jsensors)
  {
    Serial.println("adding sensor");
    Serial.println(jsensor["name"].as<const char *>());
    Serial.print("min: ");
    Serial.println(jsensor["min"].as<int>());
    Serial.print("max: ");
    Serial.println(jsensor["max"].as<int>());
    Serial.print("enabled: ");
    Serial.println(jsensor["enabled"].as<char *>());
    Serial.print("interval: ");
    Serial.println(jsensor["interval"].as<int>());
    Serial.print("unit: ");
    Serial.println(jsensor["unit"].as<const char *>());
    bool enabled = (jsensor["enabled"] == String("true"));

    sensorList[i++] = new CSensor(jsensor["min"].as<int>(), jsensor["max"].as<int>(), enabled, jsensor["interval"].as<int>(), jsensor["name"] | "untitled", jsensor["unit"], jsensor["driver"]);
  }
}

// void DemoProject::saveValue()


void DemoProject::writeToJsonObject(JsonObject &root)
{
  Serial.println("in write to json");
  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char* driver : driverList)
  {
    if (driver == NULL)
      continue;
    driverJList.add(driver);
  }  



  JsonArray sensorJList = root.createNestedArray("sensors");

  for (CSensor *sensor : sensorList)
  {
    if (sensor == NULL)
      continue;
    JsonObject jsensor = sensorJList.createNestedObject();
    jsensor["min"] = sensor->minVal;
    Serial.print("sensor->minVal: ");
    Serial.println(sensor->minVal);
    jsensor["max"] = sensor->maxVal;
    Serial.print("sensor->maxVal: ");
    Serial.println(sensor->maxVal);
    jsensor["enabled"] = (sensor->enabled == true ? "true" : "false");
    Serial.print("sensor->enabled: ");
    Serial.println(sensor->enabled);
    jsensor["interval"] = sensor->interval;
    Serial.print("sensor->interval: ");
    Serial.println(sensor->interval);
    Serial.print("sensor->unit: ");
    Serial.println(sensor->unit);
    jsensor["name"] = sensor->name;
    Serial.print("sensor->unit: ");
    jsensor["unit"] = sensor->unit;
    jsensor["driver"] = sensor->driver;
  }
}
