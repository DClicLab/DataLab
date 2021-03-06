#include <SensorSettingsService.h>

constexpr const char* SensorConfig::driverList[];
// StaticJsonDocument<4096> SensorConfig::jsonState;
CSensor* SensorConfig::sensorList[5];
char  SensorConfig::jsonstring[4096];


SensorSettingsService::SensorSettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    _httpEndpoint(SensorConfig::read,
                  SensorConfig::update,
                  this,
                  server,
                  SENSOR_SETTINGS_ENDPOINT_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _webSocket(SensorConfig::read,
               SensorConfig::update,
               this,
               server,
               SENSOR_SETTINGS_SOCKET_PATH,
               securityManager,
               AuthenticationPredicates::IS_AUTHENTICATED),
    _fsPersistence(SensorConfig::read, SensorConfig::update, this, fs, SENSOR_SETTINGS_FILE, 4096) {
  addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}


void  SensorConfig::read(SensorConfig& settings, JsonObject& root) {
  Serial.printf("Json stirng is %s\n", jsonstring);
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, (const char*) jsonstring);
  root["sensors"]=doc["sensors"];

  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char* driver : driverList) {
    if (driver == NULL)
      continue;
    StaticJsonDocument<200> doc;
    
    deserializeJson(doc, driver);
    driverJList.add(doc);
  }
}

// Received updated settings from file/UI and update JsonObject
StateUpdateResult  SensorConfig::update(JsonObject& root, SensorConfig& settings) {
  root.remove("drivers");
  SEMbusy = true;
  serializeJson(root, jsonstring);

  for (size_t i = 0; i < (sizeof(sensorList) / sizeof(CSensor*)); i++) {
    if (sensorList[i] != NULL) {
      sensorList[i]->end();
      Serial.println("Deleting sensor");
      delete (sensorList[i]);
      sensorList[i] = NULL;
    }
  }
  
  int i = 0;
  Serial.println("Adding sensors from json conf:");
  JsonArray jsensors = root.getMember("sensors");
  if (jsensors.size() > 0) {
    for (JsonObject jsensor : jsensors) {
      // settings.sensorList[i] = getSensor(jsensor);
      sensorList[i] = getSensor(jsensor);
      i++;
    }
  }

  Serial.println("RETURNING CHANGED");
  return StateUpdateResult::CHANGED;
}

void SensorSettingsService::begin() {
  _fsPersistence.readFromFS();
  onConfigUpdated();
}

void SensorSettingsService::onConfigUpdated() {
}
