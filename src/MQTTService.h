#include <PubSubClient.h>
#include "CloudService.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP_PLATFORM)
#include <WiFi.h>
#include <AsyncTCP.h>
#endif

class MQTTService : public CloudService
{
private:
    long lastReconnectAttempt = 0;
    
    
public:
    MQTTService(std::string host, std::string credentials, std::string format, std::string target);
    void publishValue(const char * message);
    void loop();
    bool reconnect();
    bool enabled;
    ~MQTTService();
};