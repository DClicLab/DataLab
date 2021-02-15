#include <SensorSettingsService.h>

constexpr  const char*  SensorConfig::driverList[];
StaticJsonDocument<1024> SensorConfig::jsonState;

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



void SensorSettingsService::begin() {
  _fsPersistence.readFromFS();
  onConfigUpdated();
}

void SensorSettingsService::onConfigUpdated() {
//   digitalWrite(LED_PIN, _state.ledOn ? LED_ON : LED_OFF);
Serial.println("Writing conf.");
}

