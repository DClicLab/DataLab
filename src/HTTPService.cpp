#include <PubSubClient.h>
#include "CloudService.h"
#include <WiFi.h>
#include <HTTPClient.h>

class HTTPService : public CloudService
{
private:

    char creds[64];
    HTTPClient http; 

public:
    HTTPService(WiFiClient wclient,const char* host, char* credentials, char* format);
    virtual void publishValue( const char* message, const char* target);
    ~HTTPService();
};

    
HTTPService::HTTPService(WiFiClient wclient,const char* host, char* credentials, char* format): CloudService(wclient,host, credentials, format)
{
  strlcpy(creds,credentials,64);
}

void HTTPService::publishValue( const char* message , const char* target){
  http.begin(target); //HTTP
  http.addHeader("Content-Type", "application/json");
  
  http.POST(message); //Send the actual POST request
}




HTTPService::~HTTPService()
{
}
