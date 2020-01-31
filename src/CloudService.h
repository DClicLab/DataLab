#ifndef CloudService_h
#define CloudService_h
#include <cstring>
#include <WiFi.h>

//TODO: add sensing of the wifi status as in the NTP lib.
//publishvalue should then check the status and call the internal virtual function _publishvalue.
class CloudService
{
private:
    /* data */
    //WiFiClient _espClient;
protected:    
    bool wifiEstablished;


public:
    char _credentials[40];
    char _host[140];
    char _format[40];
    char _driver[16];
    char _target[140];
    bool _enabled;
    
    CloudService(const char* host, const char* credentials, const char* format, const char* target, bool enabled = true) {
         _enabled = enabled;    
         strncpy(_host , host,sizeof(_host ));
         strncpy(_credentials, credentials,sizeof(_credentials));
         strncpy(_format, format,sizeof(_format));
         strncpy(_target, target,sizeof(_target)); 
    }
    virtual void loop()=0;
    virtual void publishValue( const char* message =""){};
    virtual ~CloudService(){};
};









#endif