#include <DataLab.h>
#include <ESP8266React.h>
#include <FS.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP8266React esp8266React(&server, &SPIFFS);
DataLab dataLab = DataLab(&server, &SPIFFS, esp8266React.getSecurityManager());

void setup() {

  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.println("Starting spiffs");
  // start the file system (must be done before starting the framework)
#ifdef ESP32
  SPIFFS.begin(true);
#elif defined(ESP8266)
  SPIFFS.begin();
#endif

  // start the framework and data project
  esp8266React.begin();
  Serial.println("Starting data");  

  // start the data project
  dataLab.begin();
  
  Serial.println("Starting Server");  

  // start the server
  server.begin();
  
  dataLab.start();
}

void loop() {
  // run the framework's loop function
  esp8266React.loop();

  // run the data project's loop function
  dataLab.loop();
  
}
