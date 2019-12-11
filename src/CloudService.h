#ifndef CloudService_h
#define CloudService_h
//#include <WiFi.h>


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
    std::string _credentials;
    std::string _host;
    std::string _format;
    std::string _target;
    CloudService(std::string host, std::string credentials, std::string format, std::string target) {
         
         Serial.println("Instanciating cloud service");         
         _host = host;
         _credentials= credentials;
         _format= format;
         _target= target; 
    }
    bool enabled;
    virtual void loop(){};
    virtual void publishValue( const char* message =""){};
    virtual ~CloudService() {};
};

#endif