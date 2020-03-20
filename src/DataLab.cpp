#include "DataLab.h"
#include <NtpClientLib.h>
#include <Ticker.h>
#include <TimeLib.h>
#include <Wire.h>
#include "BMP280.cpp"
#include "BMPSensor.cpp"
#include "DHT11Sensor.cpp"

#include "FreeMemSensor.cpp"
#include "AnalogInSensor.cpp"
#include "TestSensor.cpp"
#include "storage.h"

// using namespace std;

Ticker ticks[5];
vector<int> queue;  // Sensors needed to be pulled

Storage storage;

const size_t CAPACITY = 2 * JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 8 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) +
                        3 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 1000;
StaticJsonDocument<CAPACITY> doc;
JsonObject sensorsJValues = doc.to<JsonObject>();

bool busy;

//Add all sensors here
const char* driverList[] = {TestSensor::description,
                            DHT11Sensor::description,
                            BMP180Sensor::description,
                            BMP280Sensor::description,
                            FreeMemSensor::description,
                            AnalogInSensor::description,
                            
                            };

char errorBuff[400];

CSensor* DataLab::getSensor(JsonObject& sensorConf) {
  if (strcmp(sensorConf["driver"], "Random") == 0) {
    return new TestSensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "BMP") == 0) {
    return new BMP180Sensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "BMP280") == 0) {
    return new BMP280Sensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "DHT11") == 0) {
    return new DHT11Sensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "FreeMem") == 0) {
    return new FreeMemSensor(sensorConf);
  }
  if (strcmp(sensorConf["driver"], "AnalogIn") == 0) {
    return new AnalogInSensor(sensorConf);
  }



  return new TestSensor(sensorConf);
  // Add here your custom sensor
}

JsonObject getSensorJValues() {
  return sensorsJValues;
}

DataLab::DataLab(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    AdminSettingsService(server, fs, securityManager, DATA_SETTINGS_PATH, DATA_SETTINGS_FILE) {
  Serial.println("Starting Data");
  memset(sensorList, 0, sizeof(sensorList));
  // set http value responder
  server->on("/val", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncResponseStream* response = request->beginResponseStream("text/json");
    // serializeJson(doc, *response);
    // doc.clear();
    serializeJson(sensorsJValues, *response);
    //doc.clear();
    //sensorsJValues = doc.to<JsonObject>();
    request->send(response);
  });

  server->on("/list", HTTP_GET, [](AsyncWebServerRequest* request) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    char ret[2048];
    ret[0] = '\0';
    while (file) {
      strcat(ret, file.name());
      Serial.println(file.name());
      file = root.openNextFile();
    }
    request->send(200, "text/plain", ret);
  });

  server->on("/getjson", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("This can take some time...");
    busy = true;
    int id = 0;
    time_t ts = storage.currentTS;
    if (request->hasParam("id"))
      id = request->getParam("id")->value().toInt();
    if (request->hasParam("ts"))
      ts = request->getParam("ts")->value().toDouble();

    AsyncWebServerResponse* response =
        request->beginChunkedResponse("text/plain", [&id, &ts](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
          // Write up to "maxLen" bytes into "buffer" and return the amount written.
          // index equals the amount of bytes that have been already sent
          // You will be asked for more data until 0 is returned
          // Keep in mind that you can not delay or yield waiting for more data!

          return storage.readAsJsonStream(id, ts, buffer, maxLen, index);
        });
    request->onDisconnect([] {
      // free resources
      Serial.println("Request completed, back to sensor work");
      busy = false;
    });
    request->send(response);
  });

  server->on("/getErrors", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (strlen(errorBuff))
      request->send(200, "text/plain", errorBuff);
    else
      request->send(200, "text/plain", "");
  });
}

void DataLab::start() {
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

void DataLab::onConfigUpdated() {
  reconfigureTheService();
}

void DataLab::reconfigureTheService() {
  Serial.println("reconfig");
  start();
  // do whatever is required to react to the new settings
}

void DataLab::getValueForSensor(int i) {
  // adding sensor id to queue as we should not block a timer call
  Serial.printf("Timer rings for sensor %d\n", i);
  if (!queue.empty()) {
    if (std::find(queue.begin(), queue.end(), i) != queue.end())  // if i already in queue
      return;
  }
  queue.push_back(i);
}

DataLab::~DataLab() {
}

time_t DataLab::getNow() {
  if (year(now()) > 2035) {
    Serial.println("NTP Error - not returning date >2035. Returning 0 aka 1970.");
    return 0;
  }
  return now();
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.print(now());
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  time_t tnow;
  time(&tnow);
  setTime(tnow);
  Serial.println(now());
}
void DataLab::processPV(const char* keyname, time_t now, float val) {
  if (cloudService != NULL && cloudService->_enabled) {
    char str[strlen(keyname) + 50];
    // if (now>1000)//if we have correct time.
    //   snprintf(str, sizeof(str), "{\"ts\":%lu,\"values\":{\"%s\":%f}}", now*1000, keyname, val );
    // else
    snprintf(str, sizeof(str), "{\"%s\":%f}", keyname, val);
    cloudService->publishValue(str);
  }
  Serial.printf("Calling store on %s, %lu, %f\n", keyname, now, val);
  storage.store(keyname, now, val);
}

void DataLab::loop() {
  while ((queue.size() > 0) && (!busy)) {
    printLocalTime();

    int sensorID = queue.back();
    queue.pop_back();
    // queue.pop();
    CSensor* currentSensor = sensorList[sensorID];
    if (currentSensor->enabled) {
      Serial.printf("Getting value for %s\n", currentSensor->name);
      char buffer[256];
      int size = currentSensor->getValuesAsJson(buffer);
      if (size > 0) {
        StaticJsonDocument<200> sensorDoc;
        deserializeJson(sensorDoc, buffer);
        JsonObject sensorObj = sensorDoc.as<JsonObject>();

        serializeJsonPretty(sensorObj, Serial);
        
        for (JsonPair kvp : sensorObj) {
          int size = sizeof(kvp.key().c_str()) + sizeof(currentSensor->name) + 1;
          char keyname[size];
          sprintf(keyname, "%s-%s", currentSensor->name, kvp.key().c_str());
          Serial.printf("Got JSON val: %s:%f\n", keyname, kvp.value().as<float>());
          sensorsJValues[keyname] = kvp.value().as<float>();
          processPV(keyname, getNow(), kvp.value().as<float>());
        }
      } else {
        Serial.print("single Value:");

        float val = currentSensor->getValue();
        Serial.println(val);
        sensorsJValues[currentSensor->name] = val;
        processPV(currentSensor->name, getNow(), val);
      }
    } else {
      Serial.println("Disabled.");
      sensorsJValues.remove(sensorList[sensorID]->name);
    }

    String ret;
    serializeJson(sensorsJValues, ret);
    Serial.printf("JSON loop: %s\n", ret.c_str());
  }
  if (cloudService != NULL && cloudService->_enabled) {
    cloudService->loop();
  }
}

void DataLab::readFromJsonObject(JsonObject& root)  // Unserialise json to conf
{
  Serial.println("in read from json:");

  serializeJsonPretty(root, Serial);
  addDriversToJsonObject(root);

  saveConf(root);

  JsonObject jcloud = root.getMember("cloudService");
  if (!jcloud.isNull()) {
    Serial.println("We have a cloud service ");
    if (cloudService != NULL) {
      Serial.print("delete cloud service...");
      // free(cloudService); //? needed?
      Serial.println("done.");
    }

    // Serial.printf("jcloud[\"driver\"]: %s\n", jcloud["driver"].as<const char*>());
    // Serial.printf("jcloud[\"host\"]: %s\n", jcloud["host"].as<const char*>());
    // Serial.printf("jcloud[\"credentials\"]: %s\n", jcloud["credentials"].as<const char*>());
    // Serial.printf("jcloud[\"format\"]: %s\n", jcloud["format"].as<const char*>());
    // Serial.printf("jcloud[\"target\"]: %s\n", jcloud["target"].as<const char*>());

    if (strcmp(jcloud["driver"], "MQTT") == 0) {
      Serial.println("MQTT service");
      cloudService = new MQTTService(jcloud["host"].as<const char*>(),
                                     jcloud["credentials"].as<const char*>(),
                                     jcloud["format"].as<const char*>(),
                                     jcloud["target"].as<const char*>());
    } else if (strcmp(jcloud["driver"], "HTTP") == 0) {
      Serial.println("HTTP service");
      cloudService = new HTTPService(jcloud["host"].as<const char*>(),
                                     jcloud["credentials"].as<const char*>(),
                                     jcloud["format"].as<const char*>(),
                                     jcloud["target"].as<const char*>());
    }
  }

  for (auto &&tick : ticks)
  {    
      tick.detach(); 
  }
  
  for (size_t i = 0; i < (sizeof(sensorList) / sizeof(CSensor*)); i++) {
    if (sensorList[i] != NULL) {
      Serial.println("Deleting sensor");
      delete (sensorList[i]);
      sensorList[i] = NULL;
    }
  }

  int i = 0;
  Serial.println("sensors");
  JsonArray jsensors = root.getMember("sensors");
  if (jsensors.size() > 0) {
    for (JsonObject jsensor : jsensors) {
      sensorList[i] = getSensor(jsensor);
      i++;
    }
  }
}

void DataLab::addDriversToJsonObject(JsonObject& root) {
  if (root.containsKey("drivers"))  // driver list is already here
    return;
  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char* driver : driverList) {
    if (driver == NULL)
      continue;
    StaticJsonDocument<200> doc;
    deserializeJson(doc, driver);
    driverJList.add(doc);
  }
}

void DataLab::saveConf(JsonObject& root) {
  // serialize it to filesystem
  File configFile = _fs->open(_filePath, "w");

  // failed to open file, return false
  if (!configFile) {
    return;
  }

  serializeJson(root, configFile);
  configFile.close();
}
void DataLab::readConf(JsonObject& root) {
  File configFile = _fs->open(_filePath, "r");

  // use defaults if no config found
  if (configFile) {
    // Protect against bad data uploaded to file system
    // We never expect the config file to get very large, so cap it.
    size_t size = configFile.size();

    if (size <= MAX_SETTINGS_SIZE) {
      DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_SETTINGS_SIZE);
      DeserializationError error = deserializeJson(jsonDocument, configFile);
      if (error == DeserializationError::Ok && jsonDocument.is<JsonObject>()) {
        root.set(jsonDocument.as<JsonObject>());
        // root = jsonDocument.as<JsonObject>();
      }
    }
    configFile.close();
  }
}

void DataLab::writeToJsonObject(JsonObject& root) {  // Serialize conf to JSON

  readConf(root);
  addDriversToJsonObject(root);
  Serial.println("Done serializing conf:");
  serializeJsonPretty(root, Serial);
  return;

}
void DataLab::applyDefaultConfig() {  // should load default file.
  DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_SETTINGS_SIZE);
  JsonObject root = jsonDocument.to<JsonObject>();
  readFromJsonObject(root);
}
