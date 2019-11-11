#include "DemoProject.h"
#include "TestSensor.cpp"
#include <Ticker.h>
#include <stack>
using namespace std;

//Ticker* ticks[5];
//CSensor* sensorList[5];
Ticker ticks[5];
stack<int> queue;//Sensors needed to be pulled
JsonObject sensorsValues;


DemoProject::DemoProject(AsyncWebServer *server, FS *fs, SecurityManager *securityManager) : AdminSettingsService(server, fs, securityManager, DEMO_SETTINGS_PATH, DEMO_SETTINGS_FILE)
{
  pinMode(BLINK_LED, OUTPUT);
  //set http 
  server->on("/val", HTTP_GET, [](AsyncWebServerRequest *request){
    String ret;
    serializeJson(sensorsValues, ret );
    request->send(200, "application/json", ret);;
  });
  memset(sensorList,0,sizeof(sensorList));
}

void DemoProject::start(){
  Serial.println("starting conf");
  // init all sensors
  int i=0;
  Serial.println("reconfig");
  Serial.println("instanciating test sensors");
  for (CSensor* sensorc: sensorList){
      Serial.println("Adding sensor...");
      if (sensorc == NULL){
        Serial.println("Skipping not set");
        continue;
      }
      Serial.println("Adding test, interval");
      Serial.println(sensorc->interval);
      Serial.println(sensorc->name);
      sensorList[i]= new TestSensor(sensorc->minVal,sensorc->maxVal,sensorc->enabled,sensorc->interval,sensorc->name,sensorc->unit);
      ticks[i].attach<int>(sensorc->interval,getValueForSensor,i);
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

void DemoProject::getValueForSensor(int i){
     //adding sensor to queue as we should not block in timer call
     queue.push(i);
}

DemoProject::~DemoProject() {
}

void DemoProject::loop()
{
  while (queue.size()>0)
  {
    Serial.println("getting val for sensor");
    Serial.println(queue.top());
    if (sensorList[queue.top()]->enabled){
      sensorsValues[sensorList[queue.top()]->name]= sensorList[queue.top()]->getValue();
    }
    queue.pop();
  }  
}

void DemoProject::readFromJsonObject(JsonObject &root)
{
  Serial.println("in read from json");
  _blinkSpeed = root["blink_speed"] | DEFAULT_BLINK_SPEED;
  int i = 0;
  JsonArray jsensors =  root.getMember("sensors");

  //Deserializing sensors
  // deserialisation needed so that JsonBuffer is kept in memory as short as possible
  for (JsonObject jsensor : jsensors)
  {
    Serial.println("adding sensor");
    Serial.println(jsensor["name"].as<const char*>());
    Serial.print("min: ");
    Serial.println(jsensor["min"].as<int>());
    Serial.print("max: ");
    Serial.println(jsensor["max"].as<int>());
    Serial.print("enabled: ");
    Serial.println(jsensor["enabled"].as<char*>());
    Serial.print("interval: ");
    Serial.println(jsensor["interval"].as<int>());
    bool enabled = (jsensor["enabled"] == String("true"));

    sensorList[i++]= new CSensor(jsensor["min"].as<int>() , jsensor["max"].as<int>() , enabled, jsensor["interval"].as<int>() | 10, jsensor["name"] | "untitled" );
  }
}

void DemoProject::writeToJsonObject(JsonObject &root)
{
  // connection settings
  //root["blink_speed"] = _blinkSpeed;
  Serial.println("in write to json");
  root["test"] = "write";
  Serial.println("in write to json");
  JsonArray sensorJList = root.createNestedArray("sensors");
  Serial.println("done nested");

  for (CSensor* sensor : sensorList){
    if (sensor==NULL)
      continue;
    JsonObject jsensor = sensorJList.createNestedObject();
    jsensor["min"] = sensor->minVal;
    Serial.print("sensor->minVal: ");
    Serial.println(sensor->minVal);
    jsensor["max"] = sensor->maxVal;
    Serial.print("sensor->maxVal: ");
    Serial.println(sensor->maxVal);
    jsensor["enabled"] = (sensor->enabled  == true?"true":"false");
    Serial.print("sensor->enabled: ");
    Serial.println(sensor->enabled);
    jsensor["interval"] = sensor->interval;
    Serial.print("sensor->interval: ");
    Serial.println(sensor->interval);
    jsensor["name"] = sensor->name ;
    Serial.print("sensor->unit: ");
    jsensor["unit"] = sensor->unit; 
  }

}
