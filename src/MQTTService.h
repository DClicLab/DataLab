#include <PubSubClient.h>
//#include "CloudService.h"
#include <WiFi.h>

//class MQTTService : public CloudService
class MQTTService 
{
private:
    long lastReconnectAttempt = 0;
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    const char* _credentials;
    const char* _host;
    const char* _format;
    const char* _target;
    
    
public:
    MQTTService(const char* host, const char* credentials, const char* format, const char* target);
    void publishValue( const char* message);
    void loop();
    bool reconnect();
    bool enabled;
    ~MQTTService();
};