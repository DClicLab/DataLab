#include <WiFi.h>


//with time
//{"ts":1451649600512, "values":{"key1":"value1", "key2":"value2"}}

//without time
//{"key1":"value1", "key2":"value2"}

class CloudService
{
private:
    /* data */
    PubSubClient _espClient;
    const char* _host;
    const char* _credentials;
    const char* _format;
    
public:
    CloudService(WiFiClient espClient,const char* host, char* credentials, char* format) : _espClient(espClient),_host(host),_credentials(credentials),_format(format) {}
    bool enabled;
    virtual void publishValue( const char* message ="" , const char* target=""){}
    ~CloudService() {}
};
