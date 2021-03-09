#define DEFAULT_BUFFER_SIZE 4096
#define DYNAMIC_JSON_DOCUMENT_SIZE 4096
#include <semaphore.h>
#include <DataLab.h>
#include <ESP8266React.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <SensorSettingsService.h>
#define CONFIG_LITTLEFS_SPIFFS_COMPAT 1
#define SERIAL_BAUD_RATE 115200

#ifdef HASRTC
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
#endif
AsyncWebServer server(80);
ESP8266React esp8266React(&server);
// LightMqttSettingsService lightMqttSettingsService =
//     LightMqttSettingsService(&server, esp8266React.getFS(), esp8266React.getSecurityManager());
// LightStateService lightStateService = LightStateService(&server,
//                                                         esp8266React.getSecurityManager(),
//                                                         esp8266React.getMqttClient(),
//                                                         &lightMqttSettingsService);

SensorSettingsService sensorService =
    SensorSettingsService(&server, esp8266React.getFS(), esp8266React.getSecurityManager());
DataLab datalab = DataLab(&server, &sensorService);
void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp8266React.begin();

#ifdef ISM5
//setting time from RTC
  M5.Rtc.begin();
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);
  struct tm tm;
  tm.tm_year = RTC_DateStruct.Year-1900;
  tm.tm_mon = RTC_DateStruct.Month-1;
  tm.tm_mday = RTC_DateStruct.Date;
  tm.tm_hour = RTC_TimeStruct.Hours;
  tm.tm_min = RTC_TimeStruct.Minutes;
  tm.tm_sec = RTC_TimeStruct.Seconds;
  time_t time = mktime(&tm);
  Serial.printf("Got time from RTC: Time is set to %d-%d-%d %d:%d:%d\n",tm.tm_year+1900,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
  Serial.printf("UTC is %lu\n",time);

  struct timeval now = {.tv_sec = time};
  settimeofday(&now, nullptr);
  
  struct tm ts = *localtime(&time);
  char       buf[80];
  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
  Serial.println(buf);
#endif

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
