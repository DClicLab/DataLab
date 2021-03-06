#include "DataLab.h"
// #include <NtpClientLib.h>
#include <Ticker.h>
// #include <TimeLib.h>
#include <time.h>
#include <Wire.h>
// #include "BMP280.cpp"
// #include "BMPsensor.cpp"
// #include "DHT11Sensor.cpp"

// #include "FreeMemSensor.cpp"
// #include "AnalogInSensor.cpp"
// #include "TestSensor.cpp"
#include "storage.h"

// using namespace std;

Ticker ticks[5];
vector<int> queue;  // Sensors needed to be pulled

Storage storage;

// const size_t CAPACITY = 2 * JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 8 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3)
// +
//                         3 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 1000;
// StaticJsonDocument<CAPACITY> doc;
// JsonObject sensorsJValues = doc.to<JsonObject>();

char errorBuff[400];
extern bool SEMbusy;

// JsonObject getSensorJValues() {
// return sensorsJValues;
// }

void DataLab::onWSEvent(AsyncWebSocket* server,
                        AsyncWebSocketClient* client,
                        AwsEventType type,
                        void* arg,
                        uint8_t* data,
                        size_t len) {
  if (type == WS_EVT_CONNECT) {
    client->printf("{\"type\":\"id\",\"id\":\"datalab\"}");
  }
}

DataLab::DataLab(AsyncWebServer* server, SensorSettingsService* sensorSettings) {
  Serial.println("Starting Data");
  sensorsSS = sensorSettings;
  memset(sensorList, 0, sizeof(sensorList));
  // set http value responder
  // server->on("/val", HTTP_GET, [](AsyncWebServerRequest* request) {
  //   AsyncResponseStream* response = request->beginResponseStream("text/json");
  //   // serializeJson(doc, *response);
  //   // doc.clear();
  //   serializeJson(sensorsJValues, *response);
  //   // doc.clear();
  //   // sensorsJValues = doc.to<JsonObject>();
  //   request->send(response);
  // });

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

  AsyncCallbackJsonWebHandler* handler =
      new AsyncCallbackJsonWebHandler("/settime", [](AsyncWebServerRequest* request, JsonVariant& json) {
        StaticJsonDocument<200> data;
        data = json.as<JsonObject>();
        int day, month, year, h, m, s;
        Serial.printf("Got: %s\n", data["time"].as<const char*>());
        serializeJsonPretty(data, Serial);
        if (6 == sscanf(data["time"].as<const char*>(), "%d-%d-%d %d:%d:%d", &day, &month, &year, &h, &m, &s)) {
          struct tm tm;
          tm.tm_year = year - 1900;
          tm.tm_mon = month;
          tm.tm_mday = day;
          tm.tm_hour = h;
          tm.tm_min = m;
          tm.tm_sec = s;
          time_t t = mktime(&tm);
          printf("Setting time: %s", asctime(&tm));
          struct timeval now = {.tv_sec = t};
          settimeofday(&now, NULL);
          Serial.printf("Got:%d-%d-%d %d:%d:%d", day, month, year, h, m, s);
          request->send(200, "text/plain", "time set");
        } else
          request->send(500, "application/json", "error in timeset");
      });
  server->addHandler(handler);

  server->on("/rest/getjson", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("This can take some time...");

    SEMbusy = true;
    int id = -1;
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
      SEMbusy = false;
    });
    request->send(response);
  });

  server->on("/getErrors", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (strlen(errorBuff))
      request->send(200, "text/plain", errorBuff);
    else
      request->send(200, "text/plain", "");
  });

  server->on("/rest/files", HTTP_GET, [](AsyncWebServerRequest* request) {
    char buffer[4096];
    storage.getFileList(buffer);
    request->send(200, "application/json", buffer);
  });

  server->on("/rest/indexes", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/data/index", "application/json");
    request->send(response);
  });


  server->on("/rest/files/delete", HTTP_POST, [](AsyncWebServerRequest* request) {
    Serial.printf("Got delete request for %s\n",request->getParam("ts", true)->value().c_str());
    int ts = request->getParam("ts", true)->value().toInt();
    if (ts!=NAN) {
      storage.deleteTS(ts);
      request->send(200, "", "DELETED: " + request->getParam("ts", true)->value());
    } else {
      request->send(404);
    }
  });

  sensorSettings->addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);

  server->addHandler(&ws);
  ws.onEvent(onWSEvent);
}

void DataLab::start() {
  memset(ticks, 0, sizeof(ticks));
  Serial.println("starting conf");
  storage.begin();
  int i = 0;

  Serial.println("DEBUG - Preparing sensors timers");

  for (CSensor* sensor : sensorsSS->config.sensorList) {
    if (sensor == NULL) {
      Serial.println("Sensor is null.");
      sensorList[i] = NULL;
      continue;
    }

    if (sensor->enabled) {
      sensorList[i] = sensor;
      Serial.printf("Sensor %s is Enabled\n", sensor->name);
      sensor->begin();
      Serial.printf("Attaching Sensor %s, interval: %d\n", sensor->name, sensor->interval);
      Serial.printf(" senor at %p \n", sensor);
      ticks[i].attach<int>(sensor->interval, getValueForSensor, i);

      Serial.println("Done attaching sensor");
    }
    i++;
  }
  Serial.println("Done Adding sensors");
  SEMbusy = false;

  // do whatever is required to react to the new settings
}

void DataLab::onConfigUpdated() {
  // cleanup
  Serial.println("reconfig");
  for (auto&& tick : ticks) {
    tick.detach();
  }
  queue.clear();

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
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year < (2016 - 1900)) {
    Serial.println("NTP Error - not returning date >2035. Returning 0 aka 1970.");
    return 0;
  }
  return now;
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void DataLab::processPV(const char* keyname, time_t now, float val) {
  // if (cloudService != NULL && cloudService->_enabled) {
  //   char str[strlen(keyname) + 50];
  //   // if (now>1000)//if we have correct time.
  //   //   snprintf(str, sizeof(str), "{\"ts\":%lu,\"values\":{\"%s\":%g}}", now*1000, keyname, val );
  //   // else
  //   snprintf(str, sizeof(str), "{\"%s\":%g}", keyname, val);
  //   cloudService->publishValue(str);
  // }
  Serial.printf("Calling store on %s, %lu, %g\n", keyname, now, val);
  if (ws.getClients().length()) {
    Serial.printf("We have %d clients\n", ws.getClients().length());

    ws.printfAll(
        "{\"type\":\"payload\",\"payload\":{\"name\":\"%s\",\"val\":\"%g\",\"ts\":\"%lu\"}}", keyname, val, now);
  }
  storage.store(keyname, now, val);
}

int lcount = 0;
void DataLab::loop() {
  if (SEMbusy) {
    // if (lcount % 100 == 0) {
    //   Serial.println("Skipp loop");
    // }
    return;
  }
  for (CSensor* sensor : sensorList) {
    if (sensor != NULL && sensor->enabled) {
      sensor->loop();
    }
  }

  while ((queue.size() > 0) && (!SEMbusy)) {
    int sensorID = queue.back();
    Serial.print("Getting value");
    Serial.printf(" for sensor %d\n", sensorID);
    queue.pop_back();
    // queue.pop();
    CSensor* currentSensor = sensorList[sensorID];
    Serial.printf("Getting value for senor at %p", currentSensor);
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
          Serial.printf("Got JSON val: %s:%g\n", keyname, kvp.value().as<float>());
          // sensorsJValues[keyname] = kvp.value().as<float>();
          processPV(keyname, getNow(), kvp.value().as<float>());
        }
      } else {
        Serial.print("single Value:");
        float val = currentSensor->getValue();
        Serial.println(val);
        if (val != NAN) {
          // sensorsJValues[currentSensor->name] = val;
          processPV(currentSensor->name, getNow(), val);
        } else {
          Serial.println("return value is NAN, discard.");
        }
      }
    } else {
      Serial.println("Disabled.");
      // sensorsJValues.remove(sensorList[sensorID]->name);
    }

    // String ret;
    // serializeJson(sensorsJValues, ret);
    // Serial.printf("JSON loop: %s\n", ret.c_str());
  }
  // if (cloudService != NULL && cloudService->_enabled) {
  //   cloudService->loop();
  // }
}

// void DataLab::readFromJsonObject(JsonObject& root)  // Unserialise json to conf
// {
//   Serial.println("in read from json:");

//   serializeJsonPretty(root, Serial);
//   addDriversToJsonObject(root);

//   saveConf(root);

//   JsonObject jcloud = root.getMember("cloudService");
//   if (!jcloud.isNull()) {
//     Serial.println("We have a cloud service ");
//     if (cloudService != NULL) {
//       Serial.print("delete cloud service...");
//       // free(cloudService); //? needed?
//       Serial.println("done.");
//     }

//     // Serial.printf("jcloud[\"driver\"]: %s\n", jcloud["driver"].as<const char*>());
//     // Serial.printf("jcloud[\"host\"]: %s\n", jcloud["host"].as<const char*>());
//     // Serial.printf("jcloud[\"credentials\"]: %s\n", jcloud["credentials"].as<const char*>());
//     // Serial.printf("jcloud[\"format\"]: %s\n", jcloud["format"].as<const char*>());
//     // Serial.printf("jcloud[\"target\"]: %s\n", jcloud["target"].as<const char*>());

//     if (strcmp(jcloud["driver"], "MQTT") == 0) {
//       Serial.println("MQTT service");
//       cloudService = new MQTTService(jcloud["host"].as<const char*>(),
//                                      jcloud["credentials"].as<const char*>(),
//                                      jcloud["format"].as<const char*>(),
//                                      jcloud["target"].as<const char*>());
//     } else if (strcmp(jcloud["driver"], "HTTP") == 0) {
//       Serial.println("HTTP service");
//       cloudService = new HTTPService(jcloud["host"].as<const char*>(),
//                                      jcloud["credentials"].as<const char*>(),
//                                      jcloud["format"].as<const char*>(),
//                                      jcloud["target"].as<const char*>());
//     }
//   }

//   for (auto&& tick : ticks) {
//     tick.detach();
//   }

//   for (size_t i = 0; i < (sizeof(sensorList) / sizeof(CSensor*)); i++) {
//     if (sensorList[i] != NULL) {
//       Serial.println("Deleting sensor");
//       delete (sensorList[i]);
//       sensorList[i] = NULL;
//     }
//   }

//   int i = 0;
//   Serial.println("sensors");
//   JsonArray jsensors = root.getMember("sensors");
//   if (jsensors.size() > 0) {
//     for (JsonObject jsensor : jsensors) {
//       sensorList[i] = getSensor(jsensor);
//       i++;
//     }
//   }
// }

// void DataLab::addDriversToJsonObject(JsonObject& root) {
//   if (root.containsKey("drivers"))  // driver list is already here
//     return;
//   JsonArray driverJList = root.createNestedArray("drivers");
//   for (const char* driver : driverList) {
//     if (driver == NULL)
//       continue;
//     StaticJsonDocument<200> doc;
//     deserializeJson(doc, driver);
//     driverJList.add(doc);
//   }
// }

// void DataLab::saveConf(JsonObject& root) {
//   // serialize it to filesystem
//   File configFile = _fs->open(_filePath,"w");

//   // failed to open file, return false
//   if (!configFile) {
//     return;
//   }

//   serializeJson(root, configFile);
//   configFile.close();
// }
// void DataLab::readConf(JsonObject& root) {
//   File configFile = _fs->open(_filePath,"w");

//   // use defaults if no config found
//   if (configFile) {
//     // Protect against bad data uploaded to file system
//     // We never expect the config file to get very large, so cap it.
//     size_t size = configFile.size();

//     if (size <= MAX_SETTINGS_SIZE) {
//       DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_SETTINGS_SIZE);
//       DeserializationError error = deserializeJson(jsonDocument, configFile);
//       if (error == DeserializationError::Ok && jsonDocument.is<JsonObject>()) {
//         root.set(jsonDocument.as<JsonObject>());
//         // root = jsonDocument.as<JsonObject>();
//       }
//     }
//     configFile.close();
//   }
// }

// void DataLab::writeToJsonObject(JsonObject& root) {  // Serialize conf to JSON

//   readConf(root);
//   addDriversToJsonObject(root);
//   Serial.println("Done serializing conf:");
//   serializeJsonPretty(root, Serial);
//   return;
// }
// void DataLab::applyDefaultConfig() {  // should load default file.
//   DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_SETTINGS_SIZE);
//   JsonObject root = jsonDocument.to<JsonObject>();
//   readFromJsonObject(root);
// }
