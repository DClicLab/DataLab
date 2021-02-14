#include <SensorSettingsService.h>

constexpr  const char*  SensorConfig::driverList[];

SensorSettingsService::SensorSettingsService(AsyncWebServer* server, FS* fs,
                                     SecurityManager* securityManager) :
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
    _fsPersistence(SensorConfig::read, SensorConfig::update, this, fs, SENSOR_SETTINGS_FILE,4096) {
    
  // configure settings service update handler to update LED state
  addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}


void SensorConfig::read(SensorConfig& settings, JsonObject& root){
      Serial.println("In read with root as");
      serializeJsonPretty(root,Serial);
      
      File configFile = LITTLEFS.open(SENSOR_SETTINGS_FILE,"r");
      if (configFile) {
        DynamicJsonDocument jsonDocument = DynamicJsonDocument(4096);
        DeserializationError error = deserializeJson(jsonDocument, configFile);
        if (error == DeserializationError::Ok && jsonDocument.is<JsonObject>()) {
          root.set(jsonDocument.as<JsonObject>());
          Serial.println("Got conf from file in read:");
          serializeJsonPretty(jsonDocument,Serial);

          configFile.close();
          return;
        }
        configFile.close();
      }
      else{
        Serial.println("error could not find settings file");
      }


      root.remove("drivers");
      JsonArray driverJList = root.createNestedArray("drivers");
      for (const char* driver : driverList) {
        if (driver == NULL)
          continue;
        StaticJsonDocument<200> doc;
        deserializeJson(doc, driver);
        driverJList.add(doc);
      }
      Serial.println("root is now:");
      serializeJsonPretty(root,Serial);
  }


void SensorSettingsService::begin() {
  _fsPersistence.readFromFS();
  onConfigUpdated();
}

void SensorSettingsService::onConfigUpdated() {
//   digitalWrite(LED_PIN, _state.ledOn ? LED_ON : LED_OFF);
}

