#include <PubSubClient.h>
#include "CloudService.h"
#include <WiFi.h>

class MQTTService : public CloudService
{
private:
    char mqtt_server[64];
    PubSubClient client;

public:
    MQTTService(WiFiClient wclient,const char* host, char* credentials, char* format);
    virtual void publishValue( const char* message, const char* target);
    void reconnect();
    ~MQTTService();
};

    
MQTTService::MQTTService(WiFiClient wclient,const char* host, char* credentials, char* format): CloudService(wclient,host, credentials, format)
{
    client = PubSubClient(wclient);
    client.setServer(host, 1883);    
}

void MQTTService::publishValue( const char* message , const char* target){

    client.publish(target, message);

}


void MQTTService::reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      //client.subscribe(MQTTService::target);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


MQTTService::~MQTTService()
{
}
