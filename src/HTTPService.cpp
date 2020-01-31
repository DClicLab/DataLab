#include "HTTPService.h"

    
HTTPService::HTTPService(const char* host, const char* credentials, const char* format, const char* target) : CloudService(host, credentials, format, target)
{
  strcpy(_driver,"HTTP");
  strlcpy(creds,credentials,128);
  Serial.println("creating HTTP Service");
  Serial.println(creds);
}

void HTTPService::loop(){

}

void HTTPService::publishValue(const char* message){

  if (WiFi.status() != WL_CONNECTED){//Wifi is not conected.
    Serial.println("[HTTPService] - Not sending. Wifi is not connected.");
    return;
  }

  Serial.println("[HTTPService] - sending. Wifi connected.");
  //concat host and target
  //char buft[150];


  char * pch;
  Serial.println("[HTTPService] - 1");
  uint port=80;
  char chost[strlen(_host)+10];
  sscanf(_host, "%99[^:]:%99d", chost, &port);
  
  Serial.printf("Connecting to %s port %d....\n",chost,port);

  WiFiClient client;

  if (!client.connect(chost,port)){
    Serial.printf("Connection to %s port %d failed.",chost,port);
    return;
  }
  
  char header[180] = {0};
  if (creds[0]!='\0'){// split headername:value by :
    
    char headername[64];
    char value[64];
    sscanf(creds, "%99[^:]:%99[^\n]", headername, value);//mixing 2 thgs FIX  
    sprintf(header,"%s: %s\r\n",headername,value);
  }
  
  Serial.printf("POST %s HTTP/1.0\r\nContent-Type: application/json\r\n%sContent-Length: %d\r\n\r\n%s",
                _target,header,strlen(message),message);
  client.printf("POST %s HTTP/1.0\r\nContent-Type: application/json\r\n%sContent-Length: %d\r\n\r\n%s\r\n\r\n",
                _target,header,strlen(message),message);
  client.stop();
}

HTTPService::~HTTPService(){};