#include <WiFi.h>


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
    void publishValue(float val, const char* name="", const char* target=""){
        char buffer[100];
        sprintf(buffer,_format,val);
        publishValue(buffer,target);
    }
    virtual void publishValue( const char* message ="" , const char* target=""){}
    ~CloudService() {}
};
