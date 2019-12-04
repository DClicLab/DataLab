#include "DemoProject.h"
#include "TestSensor.cpp"
#include "FreeMemSensor.cpp"
#include <Ticker.h>
#include <Wire.h>
#include "DHT11Sensor.cpp"
#include "BMPSensor.cpp"
using namespace std;

//Ticker* ticks[5];
//CSensor* sensorList[5];
Ticker ticks[5];
stack<int> queue; //Sensors needed to be pulled

const size_t CAPACITY = JSON_OBJECT_SIZE(40);
StaticJsonDocument<CAPACITY> doc;
JsonObject sensorsJValues = doc.to<JsonObject>();

// template<typename T> CSensor * createInstance(CSensorParams params) { return new T(params); }
// typedef CSensor* (*mapT)();

// typedef std::map<std::string,mapT> mapSensor;
// mapSensor test;

const char *driverList[] = {"random", "dht11temp"};

CSensor *DemoProject::getSensor(const char *driverName, CSensorParams params)
{

  Serial.println("    In getSensor with params:");
  Serial.print("    sensorParams->minVal: ");
  Serial.println(params.minVal);
  Serial.print("    params.maxVal: ");
  Serial.println(params.maxVal);
  Serial.print("    params.enabled: ");
  Serial.println(params.enabled);
  Serial.print("    params.interval: ");
  Serial.println(params.interval);
  Serial.print("    params.unit: ");
  Serial.println(params.unit);
  Serial.print("    params.unit: ");

  if (strcmp(driverName, "random") == 0)
  {
    return new TestSensor(params);
  }
  if (strcmp(driverName, "bmp") == 0)
  {
    return new BMPSensor(params);
  }
  if (strcmp(driverName, "dht11temp") == 0)
  {
    return new DHT11Sensor(params);
  }

  return new TestSensor(params);
  //Add here your custom sensor
}

JsonObject getSensorJValues()
{
  return sensorsJValues;
}

DemoProject::DemoProject(AsyncWebServer *server, FS *fs, SecurityManager *securityManager) : AdminSettingsService(server, fs, securityManager, DEMO_SETTINGS_PATH, DEMO_SETTINGS_FILE)
{
  Serial.println("Starting Demo");
  memset(sensorList, 0, sizeof(sensorList));
  //set http
  server->on("/val", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = new AsyncJsonResponse(MAX_SETTINGS_SIZE);
    JsonObject jsonObject = response->getRoot();
    jsonObject.set(getSensorJValues());
    response->setLength();
    request->send(response);
  });
}

void DemoProject::start()
{
  Serial.print("MQTT Created at");
  Serial.printf("0x%" PRIXPTR "\n", (uintptr_t) cloudService);
  
  memset(sensorList, 0, sizeof(sensorList));
  memset(ticks, 0, sizeof(ticks));

  Serial.println("starting conf");
  // init all sensors
  int i = 0;
  Serial.println("instanciating test sensors");
  for (CSensorParams *sensorParams : sensorParamsList)
  {
    Serial.println("Adding sensorParams...");
    if (sensorParams == NULL)
    {
      Serial.println("Skipping not set");
      continue;
    }
    Serial.println("Adding test, interval");
    Serial.println(sensorParams->interval);
    Serial.println(sensorParams->name);
    if (sensorParams->enabled)
    {
      Serial.println("  ENABLED");
    }
    else
    {
      Serial.println("  DISABLED");
    }
    //Instantiate the sensor based on the type attribute
    //TODO: Implement a sensor factory with self-registering sensors.
    sensorList[i] = getSensor(sensorParams->driver, *sensorParams);

    Serial.println("Sensor created:");
    Serial.print("    sensorParams->minVal: ");
    Serial.println(sensorList[i]->minVal);
    Serial.print("    sensorList[i]->maxVal: ");
    Serial.println(sensorList[i]->maxVal);
    Serial.print("    sensorList[i]->enabled: ");
    Serial.println(sensorList[i]->enabled);
    Serial.print("    sensorList[i]->interval: ");
    Serial.println(sensorList[i]->interval);
    Serial.print("    sensorList[i]->unit: ");
    Serial.println(sensorList[i]->unit);

    if (sensorList[i]->enabled)
    {
      Serial.println("Sensor is Enabled");
      sensorList[i]->begin();
      ticks[i].attach<int>(sensorParams->interval, getValueForSensor, i);
    }
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
  start();
  // do whatever is required to react to the new settings
}

void DemoProject::getValueForSensor(int i)
{
  //adding sensor to queue as we should not block a timer call
  Serial.print("Timer rings for sensor ");
  Serial.println(i);
  queue.push(i);
}

DemoProject::~DemoProject()
{
}

void DemoProject::loop()
{
  while (queue.size() > 0)
  {
    CSensor *currentSensor = sensorList[queue.top()];
    Serial.print("Sensor is number ");
    Serial.print(queue.top());

    Serial.print(" name ");
    Serial.println(currentSensor->name);

    if (currentSensor->enabled)
    {
      Serial.println("Enabled");
      Serial.println("getting val for sensor");
      Serial.println(currentSensor->name);
      char buffer[256];
      int size = currentSensor->getValuesAsJson(buffer);
      if (size > 0)
      {
        StaticJsonDocument<200> sensorDoc;
        deserializeJson(sensorDoc, buffer);
        JsonObject sensorObj = sensorDoc.as<JsonObject>();
        for (JsonPair kvp : sensorObj)
        {
          int size = sizeof(kvp.key().c_str()) + sizeof(currentSensor->name) + 1;
          char keyname[size];
          strcpy(keyname, currentSensor->name);
          strcat(keyname, "-");
          strcat(keyname, kvp.key().c_str());

          Serial.println("JSON val for sensor: ");
          Serial.print(keyname);
          Serial.print("  ");
          Serial.println(kvp.value().as<float>());
          sensorsJValues[keyname] = kvp.value().as<float>();
        }
      }
      else
      {
        Serial.print("single Value:");
        float val = currentSensor->getValue();
        sensorsJValues[currentSensor->name] = val;
        Serial.println(val);
      }
    }
    else
    {
      Serial.println("Disabled.");
      sensorsJValues.remove(sensorList[queue.top()]->name);
    }

    String ret;
    serializeJson(doc, ret);
    Serial.println("ret");
    Serial.println(ret);

    if (cloudService != NULL){
      cloudService->publishValue(ret.c_str());
    }
    //    storeInSPIFFS(ret);

    queue.pop();
  }

  cloudService->loop();
}

void DemoProject::readFromJsonObject(JsonObject &root) //create the local conf from JSON POST
{
  Serial.println("in read from json");

  JsonObject jcloud = root.getMember("cloudService");
  
  Serial.print("host: ");Serial.println(jcloud["host"].as<char *>());
  Serial.print("credentials: ");Serial.println(jcloud["credentials"].as<char *>());
  Serial.print("format: ");Serial.println(jcloud["format"].as<char *>());
  Serial.print("target: ");Serial.println(jcloud["target"].as<char *>());
  Serial.print("target: ");Serial.println(jcloud["target"].as<char *>());
  
  //cloudService = new MQTTService(jcloud["host"].as<char *>(), jcloud["credentials"].as<char *>(), jcloud["format"].as<char *>(), jcloud["target"].as<char *>());
  cloudService = new MQTTService("192.168.1.46","","","/capto");
  
  Serial.print("MQTT Created at");
  Serial.printf("0x%" PRIXPTR "\n", (uintptr_t) cloudService);

  cloudService->publishValue("doed shit words");
  
  delay(3000);
  
  int i = 0;
  JsonArray jsensors = root.getMember("sensors");
  memset(sensorParamsList, 0, sizeof(sensorParamsList));
  //Deserializing sensors
  // deserialisation needed so that JsonBuffer is kept in memory as short as possible
  for (JsonObject jsensor : jsensors)
  {
    Serial.println("adding sensor");
    Serial.println(jsensor["name"].as<const char *>());
    Serial.print("  min: ");
    Serial.println(jsensor["min"].as<int>());
    Serial.print("  max: ");
    Serial.println(jsensor["max"].as<int>());
    Serial.print("  enabled: ");
    Serial.println(jsensor["enabled"].as<char *>());
    Serial.print("  interval: ");
    Serial.println(jsensor["interval"].as<int>());
    Serial.print("  unit: ");
    Serial.println(jsensor["unit"].as<const char *>());
    bool enabled = (strcmp(jsensor["enabled"], "true") == 0);
    if (enabled)
    {
      Serial.print("  ENABLED");
    }
    else
    {
      Serial.print("  DISABLED");
    }

    sensorParamsList[i++] = new CSensorParams(jsensor["min"].as<int>(), jsensor["max"].as<int>(), enabled, jsensor["interval"].as<int>(), jsensor["name"] | "untitled", jsensor["unit"], jsensor["driver"]);
  }

  Serial.print("DONE SENSOR PARAMS NOW TO THE CLOUDSERVICE");
}

// void DemoProject::saveValue()

void DemoProject::writeToJsonObject(JsonObject &root)
{
  Serial.println("in write to json");
  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char *driver : driverList)
  {
    if (driver == NULL)
      continue;
    driverJList.add(driver);
  }

  JsonObject jcloud = root.createNestedObject("cloudService");
  jcloud["driver"] = "MQTT";
  jcloud["host"] = "192.168.1.46";
  jcloud["credentials"] = "";
  jcloud["format"] = "";
  jcloud["target"] = "/capto";

  JsonArray sensorJList = root.createNestedArray("sensors");
  for (CSensorParams *sensorParams : sensorParamsList)
  {
    if (sensorParams == NULL)
      continue;
    JsonObject jsensor = sensorJList.createNestedObject();
    jsensor["min"] = sensorParams->minVal;
    Serial.print("sensorParams->minVal: ");
    Serial.println(sensorParams->minVal);
    jsensor["max"] = sensorParams->maxVal;
    Serial.print("sensorParams->maxVal: ");
    Serial.println(sensorParams->maxVal);
    jsensor["enabled"] = (sensorParams->enabled == true ? "true" : "false");
    Serial.print("sensorParams->enabled: ");
    Serial.println(sensorParams->enabled);
    jsensor["interval"] = sensorParams->interval;
    Serial.print("sensorParams->interval: ");
    Serial.println(sensorParams->interval);
    Serial.print("sensorParams->unit: ");
    Serial.println(sensorParams->unit);
    jsensor["name"] = sensorParams->name;
    Serial.print("sensorParams->unit: ");
    jsensor["unit"] = sensorParams->unit;
    jsensor["driver"] = sensorParams->driver;
  }
}
