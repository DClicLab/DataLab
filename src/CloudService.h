#ifndef CloudService_h
#define CloudService_h
#include <WiFi.h>


//with time
//{"ts":1451649600512, "values":{"key1":"value1", "key2":"value2"}}

//without time
//{"key1":"value1", "key2":"value2"}


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
    CloudService(const char* host, const char* credentials, const char* format, const char* target) {
         
         Serial.println("Instanciating cloud service");
         
         this->_host = host;
         this->_credentials=credentials;
         this->_format=format;
         this->_target=target; 
    }
    const char* _credentials;
    const char* _host;
    const char* _format;
    const char* _target;
    bool enabled;
    virtual void loop(){};
    virtual void publishValue( const char* message =""){}
    ~CloudService() {}
};

#endif