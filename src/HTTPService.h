#include "CloudService.h"

// #if defined(ESP8266)
// #include <ESP8266HTTPClient.h>
// #elif defined(ESP_PLATFORM)
// #include <HTTPClient.h>
// #endif

class HTTPService : public CloudService
{
private:

    char creds[128]={0};
  
public:
    HTTPService(const char* host, const char* credentials, const char* format, const char* target);
    void publishValue( const char* message);
    void loop();
    ~HTTPService();
};