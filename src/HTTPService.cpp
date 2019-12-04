#include <PubSubClient.h>
#include "CloudService.h"
#include <HTTPClient.h>

class HTTPService : public CloudService
{
private:

    char creds[64];
    HTTPClient http; 

public:
    HTTPService(const char* host, const char* credentials, const char* format,  const char* target);
    virtual void publishValue( const char* message);
    ~HTTPService();
};

    
HTTPService::HTTPService(const char* host, const char* credentials, const char* format, const char* target) : CloudService(host, credentials, format, target)
{
  strlcpy(creds,credentials,64);
}

void HTTPService::publishValue( const char* message){
  //concat host and target
  http.begin(_target); //HTTP
  http.addHeader("Content-Type", "application/json");
  if (creds[0]!='\0'){
    char * pch;
    char header[64];
    pch = strtok (creds,":");
    strlcpy(header,pch,64);
    pch = strtok (NULL,":");
    http.addHeader(header,pch);
  }
  http.POST(message); //Send the actual POST request
}




HTTPService::~HTTPService()
{
}
