#include "MQTTService.h"

WiFiClient espClient;
PubSubClient client;

//MQTTService::MQTTService(const char* host, const char* credentials, const char* format, const char* target): CloudService(host, credentials, format, target)
MQTTService::MQTTService(const char* host, const char* credentials, const char* format, const char* target)
{
   Serial.print("[MQTTService] Setting server:");
   Serial.println(host);
    
    this->_host = host;
    this->_credentials=credentials;
    this->_format=format;
    this->_target=target; 

   client = PubSubClient(espClient);
   client.setServer(host, 1883);    
}

void MQTTService::publishValue( const char* message ){
  Serial.print("[MQTTService] Host is:");
  Serial.println(this->_host);
  Serial.print("[MQTTService] Message is:");
  Serial.println(message);
  if (WiFi.status()== WL_CONNECTED){
    if (!client.connected()){
      reconnect();
    }
    Serial.print("[MQTTService] sending message:");
    Serial.println(message);
    client.publish(_target, message);
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
