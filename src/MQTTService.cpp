#include "MQTTService.h"

WiFiClient espClient;
PubSubClient client;
bool connecting;

MQTTService::MQTTService(std::string host, std::string credentials, std::string format, std::string target) : CloudService(host, credentials, format, target)
{
  espClient.setTimeout(1);
   client = PubSubClient(espClient);
   client.setServer(_host.c_str(), 1883);    
  
}

void MQTTService::publishValue(const char * message ){

  Serial.printf("Host is %s\n",this->_host.c_str());

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

void taskConnect(PubSubClient* psclient){
  connecting=true;
  psclient->connect("captoClient");
  connecting=false;
  vTaskDelete(NULL);
}

bool MQTTService::reconnect() {

  if (connecting){
    Serial.println("[MQTTService] already connecting... skiping.");
    return false;
  }
  Serial.println("[MQTTService] Connecting to server.");
  xTaskCreatePinnedToCore(// MQTT Connection is blocking, let's use the second core so we don't block the rest
  (TaskFunction_t) taskConnect,   /* Task function. */
            "Task2",     /* name of task. */
            100000,       /*1 Stack size of task */
            (void *) &client,        /* parameter of the task */
            1,           /* priority of the task */
            NULL
            ,      /* Task handle to keep track of created task */
            0);          /* pin task to core 1 */
  delay(100);
  return client.connected();
}

void MQTTService::loop(){
  if (WiFi.status()== WL_CONNECTED && !client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 3000) {
      lastReconnectAttempt = now;
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