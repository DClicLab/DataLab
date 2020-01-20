#include "DemoProject.h"
#include <NtpClientLib.h>
#include <Ticker.h>
#include <TimeLib.h>
#include <Wire.h>
#include "BMPSensor.cpp"
#include "DHT11Sensor.cpp"
#include "FreeMemSensor.cpp"
#include "TestSensor.cpp"
#include "storage.h"

using namespace std;

Ticker ticks[5];
stack<int> queue;  // Sensors needed to be pulled

Storage storage;

const size_t CAPACITY = JSON_OBJECT_SIZE(40);
StaticJsonDocument<CAPACITY> doc;
JsonObject sensorsJValues = doc.to<JsonObject>();

const char* driverList[] = {"random", "dht11temp"};

CSensor* DemoProject::getSensor(JsonObject& sensorConf) {
  if (strcmp(sensorConf["driver"], "random") == 0) {
    return new TestSensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "bmp") == 0) {
    return new BMPSensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "dht11temp") == 0) {
    return new DHT11Sensor(sensorConf);
  }

  return new TestSensor(sensorConf);
  // Add here your custom sensor
}

JsonObject getSensorJValues() {
  return sensorsJValues;
}

DemoProject::DemoProject(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    AdminSettingsService(server, fs, securityManager, DEMO_SETTINGS_PATH, DEMO_SETTINGS_FILE) {
  Serial.println("Starting Demo");
  memset(sensorList, 0, sizeof(sensorList));
  // set http value responder
  server->on("/val", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncResponseStream* response = request->beginResponseStream("text/json");
    serializeJson(doc, *response);
    doc.clear();
    sensorsJValues = doc.to<JsonObject>();
    request->send(response);
  });

  server->on("/list", HTTP_GET, [](AsyncWebServerRequest* request) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    char ret[2048];
    ret[0]='\0';
    while (file) {
      strcat(ret, file.name());
      Serial.println(file.name());
      file = root.openNextFile();
    }
    request->send(200, "text/plain", ret);
  });

  server->on("/getjson", HTTP_GET, [](AsyncWebServerRequest* request) {
    
    int id = 0;
    time_t ts = storage.currentTS;
    if (request->hasParam("id"))
      id = atoi(request->getParam("id")->value().c_str());
    if (request->hasParam("ts"))
      ts = atol(request->getParam("ts")->value().c_str());
      
    AsyncWebServerResponse* response =
        request->beginChunkedResponse("text/plain", [&id,&ts](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
          // Write up to "maxLen" bytes into "buffer" and return the amount written.
          // index equals the amount of bytes that have been already sent
          // You will be asked for more data until 0 is returned
          // Keep in mind that you can not delay or yield waiting for more data!

          return storage.readAsJsonStream(id, ts, buffer, maxLen, index);
        });

    request->send(response);
  });
}

void DemoProject::start() {
  memset(ticks, 0, sizeof(ticks));
  Serial.println("starting conf");
  storage.begin();
  int i = 0;
  Serial.println("DEBUG - Preparing sensors timers");
  for (CSensor* sensor : sensorList) {
    if (sensor == NULL) {
      continue;
    }

    if (sensor->enabled) {
      Serial.println("Sensor is Enabled");
      sensor->begin();

      Serial.printf("Attaching Sensor %s, interval: %d\n", sensor->name, sensor->interval);
      ticks[i].attach<int>(sensor->interval, getValueForSensor, i);
      Serial.println("Done attaching sensor");
    }
    i++;
  }
  Serial.println("Done Adding sensors");
  // do whatever is required to react to the new settings
}
void DemoProject::onConfigUpdated() {
  reconfigureTheService();
}

void DemoProject::reconfigureTheService() {
  Serial.println("reconfig");
  start();
  // do whatever is required to react to the new settings
}

void DemoProject::getValueForSensor(int i) {
  // adding sensor to queue as we should not block a timer call
  Serial.printf("Timer rings for sensor %d\n", i);
  queue.push(i);
}

DemoProject::~DemoProject() {
}

time_t DemoProject::getNow() {
  if (year(now()) > 2035) {
    Serial.println("NTP Error - not returning date >2035. Returning 0 aka 1970.");
    return 0;
  }
  return now();
}

void DemoProject::loop() {

  while (queue.size() > 0) {
    CSensor* currentSensor = sensorList[queue.top()];
    if (currentSensor->enabled) {
      Serial.printf("Getting value for %s\n", currentSensor->name);
      char buffer[256];
      int size = currentSensor->getValuesAsJson(buffer);
      if (size > 0) {
        StaticJsonDocument<200> sensorDoc;
        deserializeJson(sensorDoc, buffer);
        JsonObject sensorObj = sensorDoc.as<JsonObject>();
        for (JsonPair kvp : sensorObj) {
          int size = sizeof(kvp.key().c_str()) + sizeof(currentSensor->name) + 1;
          char keyname[size];
          sprintf(keyname, "%s-%s", currentSensor->name, kvp.key().c_str());
          Serial.printf("Got JSON val: %s:%f\n", keyname, kvp.value().as<float>());
          sensorsJValues[keyname] = kvp.value().as<float>();
          storage.store(keyname, getNow(), kvp.value().as<float>());
        }
      } else {
        Serial.print("single Value:");
        float val = currentSensor->getValue();
        Serial.println(val);
        sensorsJValues[currentSensor->name] = val;
        storage.store(currentSensor->name, getNow(), val);
      }
    } else {
      Serial.println("Disabled.");
      sensorsJValues.remove(sensorList[queue.top()]->name);
    }

    String ret;
    serializeJson(doc, ret);
    Serial.printf("JSON loop: %s\n", ret.c_str());

    if (cloudService != NULL && cloudService->enabled) {
      cloudService->publishValue(ret.c_str());
    }

    queue.pop();
  }

  cloudService->loop();
}

void DemoProject::readFromJsonObject(JsonObject& root)  // create the local conf from JSON POST
{
  Serial.println("in read from json");

  JsonObject jcloud = root.getMember("cloudService");
  if (!jcloud.isNull()) {
    Serial.println("We have a cloud service ");
    if (cloudService!=NULL){
      delete(cloudService);
    }

    cloudService = new MQTTService(jcloud["host"].as<const char*>(),
                                   jcloud["credentials"].as<const char*>(),
                                   jcloud["format"].as<const char*>(),
                                   jcloud["target"].as<const char*>());
  }


  //memset(sensorList, 0, sizeof(sensorList));
  memset(ticks, 0, sizeof(ticks));
  int i = 0;

  JsonArray jsensors = root.getMember("sensors");
  if (jsensors.size() > 0) {
    for (JsonObject jsensor : jsensors) {
      if (sensorList[i]!=NULL){
        delete(sensorList[i]);
      }
      sensorList[i] = getSensor(jsensor);
      i++;
    }
  }
}

void DemoProject::writeToJsonObject(JsonObject& root) {
  Serial.println("in write to json");
  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char* driver : driverList) {
    if (driver == NULL)
      continue;
    driverJList.add(driver);
  }

  JsonObject jcloud = root.createNestedObject("cloudService");
  jcloud["driver"] = "MQTT";
  jcloud["host"] = cloudService->_host.c_str();
  jcloud["credentials"] = cloudService->_credentials.c_str();
  jcloud["format"] = cloudService->_format.c_str();
  jcloud["target"] = cloudService->_target.c_str();

  JsonArray sensorJList = root.createNestedArray("sensors");
  for (CSensor* sensor : sensorList) {
    if (sensor != NULL) {
      JsonObject jsensor = sensorJList.createNestedObject();
      sensor->getConfig(jsensor);
    }
  }
}
