#include "MQTTService.h"

WiFiClient espClient;
PubSubClient client;
bool connecting;

MQTTService::MQTTService(const char* host, const char* credentials, const char* format, const char* target) :
  CloudService(host, credentials, format, target) {
  Serial.println("Creating mqttservice");
  strcpy(_driver,"MQTT");
  espClient.setTimeout(1);
  client = PubSubClient(espClient);
  client.setServer(_host, 1883);
  Serial.println("done.");
  
}

void MQTTService::publishValue(const char* message) {
  Serial.printf("Host is %s\n", this->_host);

  if (WiFi.status() == WL_CONNECTED) {
    if (client.connected()) {
        Serial.print("[MQTTService] sending message:");
        Serial.println(message);
        client.publish(_target, message);
      } else {
        /* code */
        Serial.println("[MQTTService] Client is not connected, cannot send message.");//Reconnection is in the loop()
      }
  } else {
    Serial.println("[MQTTService] Wifi is not connected, cannot send message.");
  }
}

  struct ConnectParams
  {
    PubSubClient* client;
    const char* user;
    const char* pass;
  };
void taskConnect(ConnectParams* psclient, const char* creds) {
  connecting = true;
  psclient->client->connect("datalab2",psclient->user,"");
  connecting = false;
  vTaskDelete(NULL);
}

bool MQTTService::reconnect() {
  if (connecting) {
    //Serial.println("[MQTTService] already connecting... skiping.");
    return false;
  }
  // ConnectParams params;
  // params.client=&client;
  // params.user=_credentials;
  // params.pass="";

  Serial.printf("[MQTTService] Connecting to server %s with creds %s.\n",_host,_credentials);
  // xTaskCreatePinnedToCore(         // MQTT Connection is blocking, let's use the second core so we don't block the rest
  //     (TaskFunction_t)taskConnect, /* Task function. */
  //     "Task2",                     /* name of task. */
  //     100000,                      /*1 Stack size of task */
  //     (void*)&params,              /* parameter of the task */
  //     1,                           /* priority of the task */
  //     NULL,                        /* Task handle to keep track of created task */
  //     0);                          /* pin task to core 1 */
  // delay(100);
  
  return client.connect("datalab2", _credentials,"");
  //return client.connected();
}

ulong firstErr=0;
uint iErr=0;//Iteration on error connection for Exponential Backoff
uint waitTime=0;//Iteration on error connection for Exponential Backoff
void MQTTService::loop() {
  
  if (WiFi.status() != WL_CONNECTED){//Wifi is not conected.
    return;
  }
  // Serial.printf("Now is %lu\n",now());

  if (!client.connected()) {

    if ((firstErr==0)||(firstErr-now()>100000))
      firstErr=now();

    if (now()>firstErr + waitTime +random(0,10) ){
      Serial.println("[MQTTService] re-try connection");
       Serial.printf("[MQTTService] First :%lu, Wait:%lu now: %lu\n",firstErr,waitTime,now());

      if (reconnect()) {//reconnected! \o/
        firstErr=0;
        iErr=0;
        Serial.println("[MQTTService] re-connected");
      }else{//reconnection failed
        iErr++;
        //new wait time
        waitTime= pow(2, iErr/2) *10 ;
        Serial.printf("%d succeding errors new wait time is %d \n",iErr,waitTime);
      }
    }

  } else {
    // Client connected
    client.loop();
  }
}

MQTTService::~MQTTService() {
}