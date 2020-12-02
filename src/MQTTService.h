#define MQTT_SOCKET_TIMEOUT 1
#include <PubSubClient.h>
#include "CloudService.h"
// #include <TimeLib.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP_PLATFORM)
#include <WiFi.h>
#endif

class MQTTService : public CloudService
{
private:
    long lastReconnectAttempt = 0;
    
    
public:
    MQTTService(const char* host, const char* credentials, const char* format, const char* target);
    void publishValue(const char * message);
    void loop();
    bool reconnect();
    bool enabled;
    ~MQTTService();
};