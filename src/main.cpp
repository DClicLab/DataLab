#define DEFAULT_BUFFER_SIZE 4096
#define DYNAMIC_JSON_DOCUMENT_SIZE 4096
#include <semaphore.h>
#include <ESP8266React.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <SensorSettingsService.h>
#include <DataLab.h>
#define CONFIG_LITTLEFS_SPIFFS_COMPAT 1
#define SERIAL_BAUD_RATE 115200
AsyncWebServer server(80);
ESP8266React esp8266React(&server);
// LightMqttSettingsService lightMqttSettingsService =
//     LightMqttSettingsService(&server, esp8266React.getFS(), esp8266React.getSecurityManager());
// LightStateService lightStateService = LightStateService(&server,
//                                                         esp8266React.getSecurityManager(),
//                                                         esp8266React.getMqttClient(),
//                                                         &lightMqttSettingsService);                                                        

SensorSettingsService sensorService = SensorSettingsService(&server,esp8266React.getFS(),esp8266React.getSecurityManager());
DataLab datalab= DataLab(&server,&sensorService);
void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp8266React.begin();


  // File root = ESPFS.open("/www");
    
  // File file = root.openNextFile();
 
  // while(file){
 
  //     Serial.print("FILE: ");
  //     Serial.println(file.name());
 
  //     file = root.openNextFile();
  // }

  // load the initial light settings
  // lightStateService.begin();

  

  sensorService.begin();
  // start the light service
  // lightMqttSettingsService.begin();

  // start the server
  datalab.start();
  server.begin();
}

void loop() {
  // run the framework's loop function
  esp8266React.loop();
  datalab.loop();
}
