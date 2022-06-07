#include "DataLab.h"
#include <Ticker.h>
#include <time.h>
#include <Wire.h>
#include "storage.h"

Ticker ticks[5];
vector<int> queue;  // Sensors needed to be pulled

Storage storage;

extern unsigned char SEMbusy;

void setBusy(AsyncWebServerRequest* request) {
  SEMbusy++;
  request->onDisconnect([] { SEMbusy--; });
}

class SemaphoreHandler : public AsyncWebHandler {
 public:
  SemaphoreHandler() {
  }
  virtual ~SemaphoreHandler() {
  }

  bool canHandle(AsyncWebServerRequest* request) {
    if (request->url().startsWith("/raw/")) {
      Serial.printf("A /raw request is comming!\n");
      SEMbusy++;
      request->onDisconnect([] {
        Serial.println("Request completed, back to sensor work");
        SEMbusy--;
      });
    };
    return false;
  }
};

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

DataLab::DataLab(AsyncWebServer* server, AsyncMqttClient* mqttClient, SensorSettingsService* sensorSettings) :
    _mqttClient(mqttClient), sensorsSS(sensorSettings) {
  Serial.println("Starting DataLab");
  memset(sensorList, 0, sizeof(sensorList));

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

  server->on("/rest/files", HTTP_GET, [](AsyncWebServerRequest* request) {
    SEMbusy++;
    char buffer[4096];
    storage.getFileList(buffer);
    request->send(200, "application/json", buffer);
    request->onDisconnect([] { SEMbusy--; });
  });

  server->on("/rest/indexes", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/data/index", "application/json");
    request->send(response);
  });



  server->on("/rest/resetconf", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "", "OK Deleting");
    request->onDisconnect([] { 
      LittleFS.open("/config/resetconf", "w");  // dealt with in begin()
      ESP.restart();
     });
  });

  server->on("/rest/deleteall", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "", "OK Deleting");
    request->onDisconnect([] { 
      LittleFS.open("/data/d/delete", "w");  // dealt with in begin()
      ESP.restart();
     });
  });

  server->on("/rest/files/delete", HTTP_POST, [](AsyncWebServerRequest* request) {
    Serial.printf("Got delete request for %s\n", request->getParam("ts", true)->value().c_str());
    int ts = request->getParam("ts", true)->value().toInt();
    if (ts != NAN) {
      storage.deleteTS(ts);
      request->send(200, "", "DELETED: " + request->getParam("ts", true)->value());
    } else {
      request->send(404);
    }
    request->onDisconnect([] { SEMbusy = false; });
  });

  AsyncCallbackJsonWebHandler* handler =
      new AsyncCallbackJsonWebHandler("/rest/settime", [](AsyncWebServerRequest* request, JsonVariant& json) {
        String timeUtc = json["time"];
        struct tm tm = {0};
        char* s = strptime(timeUtc.c_str(), "%d-%m-%Y %H:%M:%S", &tm);
        if (s != nullptr) {
          setenv("TZ", "UTC0", 1);
          tzset();
          time_t ltime = mktime(&tm);
          struct timeval now = {.tv_sec = ltime};
          settimeofday(&now, nullptr);
          Serial.printf("Got time from www: utc Time is set to %d-%d-%d %d:%d:%d\n",
                        tm.tm_year + 1900,
                        tm.tm_mon + 1,
                        tm.tm_mday,
                        tm.tm_hour,
                        tm.tm_min,
                        tm.tm_sec);

      // tzset();

#ifdef HASRTC
          RTC_TimeTypeDef TimeStruct;
          TimeStruct.Hours = tm.tm_hour;
          TimeStruct.Minutes = tm.tm_min;
          TimeStruct.Seconds = tm.tm_sec;
          M5.Rtc.SetTime(&TimeStruct);
          RTC_DateTypeDef DateStruct;
          DateStruct.Month = tm.tm_mon + 1;
          DateStruct.Date = tm.tm_mday;
          DateStruct.Year = tm.tm_year + 1900;
          M5.Rtc.SetData(&DateStruct);
          char buf[80];
          strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &tm);
          Serial.printf("RTC Time is set from WWW: %s\n", buf);
#endif
          setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
          tzset();
          time_t rawtime;
          time(&rawtime);
          struct tm ts = *localtime(&rawtime);
          strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
          Serial.printf("Local time is %s\n", buf);

          AsyncWebServerResponse* response = request->beginResponse(200);
          request->send(response);
          return;
        }
      });
  server->addHandler(handler);

  server->addHandler(new SemaphoreHandler());
  server->serveStatic("/raw/", SPIFFS, "/data/d/");

  sensorSettings->addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);

  server->addHandler(&ws);
  ws.onEvent(onWSEvent);
}

void setupScreen() {
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  M5.begin();
  M5.Axp.EnableCoulombcounter();
  M5.Lcd.setRotation(1);
  M5.Axp.ScreenBreath(12);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  m5.Lcd.setTextDatum(MC_DATUM);
  int xpos = M5.Lcd.width() / 2;   // Half the screen width
  int ypos = M5.Lcd.height() / 2;  // Half the screen width
  M5.Lcd.setTextColor(TFT_DARKGREY);
  M5.Lcd.drawString("DataLab", xpos, ypos, 1);
  delay(2000);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Axp.ScreenBreath(0);
#endif
}
void DataLab::start() {
  setupScreen();
  memset(ticks, 0, sizeof(ticks));
  storage.begin();
  int i = 0;
  Serial.println("DEBUG - Preparing sensors timers");

  for (CSensor* sensor : sensorsSS->config.sensorList) {
    if (sensor == NULL) {
      sensorList[i] = NULL;
      continue;
    }

    if (sensor->enabled) {
      sensorList[i] = sensor;
      Serial.printf("Sensor %s is Enabled\n", sensor->name);
      sensor->begin();
      if (sensor->interval){
      Serial.printf("Attaching Sensor %s, interval: %d\n", sensor->name, sensor->interval);
      Serial.printf(" senor at %p \n", sensor);
      ticks[i].attach<int>(sensor->interval, getValueForSensor, i);
      }
      else{
        Serial.println("ERROR - Not attaching sensor with interval 0s");
      }
      Serial.println("Done attaching sensor");
    }
    i++;
  }
  Serial.println("Done Adding sensors");
  SEMbusy = 0;

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

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
time_t DataLab::getNow() {
  return time(nullptr);
  // time_t now;
  // struct tm timeinfo;
  // timeinfo=*gmtime(&now);

  // if (timeinfo.tm_year < (2016 - 1900)) {
  //   // printLocalTime();
  //   Serial.println("NTP Error - not returning date >2035. Returning 0 aka 1970.");
  //   return 0;
  // }
  // return now;
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
    ws.printfAll(
        "{\"type\":\"payload\",\"payload\":{\"name\":\"%s\",\"val\":\"%g\",\"ts\":\"%lu\"}}", keyname, val, now);
  }
  storage.store(keyname, now, val);
}

ulong LCDTimeout = 0;
int lcount = 0;
double lastval = 0;
int lastid = 0;
ulong lastms = 0;
bool screenup = false;
void DataLab::loop() {
  if (SEMbusy) {
    return;
  }

#ifdef ISM5
  M5.BtnA.read();
  if (M5.BtnA.wasPressed()) {  // Turn back ON screen
    screenup = true;
    M5.Axp.ScreenBreath(12);
    LCDTimeout = millis() + 7000;
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    // Status print
    M5.Lcd.printf("DataLab - Uptime %01.0f:%02.0f:%02.2f\n",
                  floor(millis() / 3600000.0),
                  floor((millis() / 1000) % 3600 / 60.0),
                  (millis() / 1000) % 60 / 1.0);
    // char buf[300];
    // storage.getFileList(buf);
    // StaticJsonDocument<200> doc;
    // deserializeJson(doc,buf);
    // M5.Lcd.printf("Current file contains:\n");
    if (lastval) {
      M5.Lcd.printf("Last value: %s:%g\n", sensorList[lastid]->name, lastval);
      uint time = (millis() - lastms) / 1000;
      M5.Lcd.printf("%01.0f min %02.0f sec ago\n", floor(time / 60) / 1.0, (time) % 60 / 1.0);
    }

    M5.Lcd.printf("\nBattery: V: %.3fv  I: %.3fma\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());

  } else if (screenup && LCDTimeout < millis()) {  // Turn screen off.
    M5.Axp.ScreenBreath(0);
  }

#endif
  for (CSensor* sensor : sensorList) {
    if (sensor != NULL && sensor->enabled) {
      sensor->loop();
    }
  }

  while ((queue.size() > 0) && (!SEMbusy)) {
    int sensorID = queue.back();
    queue.pop_back();
    CSensor* currentSensor = sensorList[sensorID];
    if (currentSensor->enabled) {
      Serial.printf("Getting value for %s\n", currentSensor->name);
      char buffer[256];
      int size = currentSensor->getValuesAsJson(buffer);
      lastms = millis();
      lastid = sensorID;
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
          lastval = kvp.value().as<float>();
        }
      } else {
        Serial.print("single Value:");
        float val = currentSensor->getValue();
        Serial.println(val);
        if (val != NAN) {
          // sensorsJValues[currentSensor->name] = val;
          processPV(currentSensor->name, getNow(), val);
          lastval = val;
        } else {
          Serial.println("return value is NAN, discard.");
        }
      }
    } else {
      Serial.println("Disabled.");
    }
  }
  // if (cloudService != NULL && cloudService->_enabled) {
  //   cloudService->loop();
  // }
}
