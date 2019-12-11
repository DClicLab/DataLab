#include "MQTTService.h"

WiFiClient espClient;
PubSubClient client;


MQTTService::MQTTService(std::string host, std::string credentials, std::string format, std::string target) : CloudService(host, credentials, format, target)
{
   client = PubSubClient(espClient);
   client.setServer(_host.c_str(), 1883);    

}

void MQTTService::publishValue(const char * message ){

  Serial.printf("Host is %s",this->_host.c_str());

  if (WiFi.status()== WL_CONNECTED){
    if (!client.connected()){
      reconnect();
    }
    Serial.print("[MQTTService] sending message:");
    Serial.println(message);
    client.publish(_target.c_str(), message );
  }
  else{
    Serial.println("[MQTTService] Wifi is not connected, cannot send message.");
  }
}


bool MQTTService::reconnect() {

  Serial.print("[MQTTService] Connecting to server.");
  client.connect("captoClient");
  
  return client.connected();
}
void MQTTService::loop(){

  if (WiFi.status()== WL_CONNECTED && !client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 3000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
        if (reconnect()) {
      Serial.println("[MQTTService] connected, cannot send message.");

        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
}


MQTTService::~MQTTService()
{
}
