#include <SensorSettingsService.h>

constexpr const char* SensorConfig::driverList[];
// StaticJsonDocument<4096> SensorConfig::jsonState;
CSensor* SensorConfig::sensorList[5];
char  SensorConfig::jsonstring[4096];
I2CConf SensorConfig::i2cconf;


SensorSettingsService::SensorSettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    _httpEndpoint(SensorConfig::read,
                  SensorConfig::update,
                  this,
                  server,
                  SENSOR_SETTINGS_ENDPOINT_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED,4096),
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
  
  Serial.printf("root size is %d\n", root.size());
  StaticJsonDocument<4096> doc;
  deserializeJson(doc, (const char*) jsonstring);
  root["sensors"]=doc["sensors"];
  root["i2c"]["conf"]=i2cconf.conf;
  root["i2c"]["enabled"]=i2cconf.enabled;
  root["i2c"]["sda"]=i2cconf.sda;
  root["i2c"]["scl"]=i2cconf.scl;

  JsonArray driverJList = root.createNestedArray("drivers");
  for (const char* driver : driverList) {
    if (driver == NULL)
      continue;
    StaticJsonDocument<1024> docx;
    deserializeJson(docx, driver);
    serializeJsonPretty(docx,Serial);
    driverJList.add(docx);
  }
  Serial.printf("All added.\n");
}

// Received updated settings from file/UI and update JsonObject
StateUpdateResult  SensorConfig::update(JsonObject& root, SensorConfig& settings) {
  Serial.println("iN update");
  root.remove("drivers");
  SEMbusy = 1;
  serializeJson(root, jsonstring);
  if (!root["i2c"]["conf"].isNull())
    strncpy(i2cconf.conf,root["i2c"]["conf"].as<char*>(),8);
  else
    strncpy(i2cconf.conf,"Custom",8);
  i2cconf.enabled=root["i2c"]["enabled"];
  i2cconf.sda=root["i2c"]["sda"].isNull()==false?root["i2c"]["sda"]:-1;
  i2cconf.scl=root["i2c"]["scl"].isNull()==false?root["i2c"]["scl"]:-1;

  if (i2cconf.enabled){
    Serial.printf("I2C Enabled - starting i2c bus on sda:%d, scl:%d\n",i2cconf.sda,i2cconf.scl);
    Wire.begin(i2cconf.sda,i2cconf.scl);
  }
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

  Serial.println("Done update(). RETURNING CHANGED");
  return StateUpdateResult::CHANGED;
}

void SensorSettingsService::begin() {
  _fsPersistence.readFromFS();
  onConfigUpdated();
}

void SensorSettingsService::onConfigUpdated() {
}
