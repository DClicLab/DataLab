#include <ESP8266React.h>
#include <DemoProject.h>
#include <FS.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP8266React esp8266React(&server, &SPIFFS);
DemoProject demoProject = DemoProject(&server, &SPIFFS, esp8266React.getSecurityManager());

void setup() {

  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.println("Starting spiffs");
  // start the file system (must be done before starting the framework)
  SPIFFS.begin();
  Serial.println("Starting react");  
  // start the framework and demo project
  esp8266React.begin();
  Serial.println("Starting demo");  

  // start the demo project
  demoProject.begin();
  
  Serial.println("Starting Server");  

  // start the server
  server.begin();
  
  demoProject.start();
}

void loop() {
  // run the framework's loop function
  esp8266React.loop();

  // run the demo project's loop function
  demoProject.loop();
}
