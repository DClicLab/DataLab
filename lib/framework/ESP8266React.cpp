#include <ESP8266React.h>

ESP8266React::ESP8266React(AsyncWebServer* server) :
    _featureService(server),
    _securitySettingsService(server, &ESPFS),
    _wifiSettingsService(server, &ESPFS, &_securitySettingsService),
    _wifiScanner(server, &_securitySettingsService),
    _wifiStatus(server, &_securitySettingsService),
    _apSettingsService(server, &ESPFS, &_securitySettingsService),
    _apStatus(server, &_securitySettingsService, &_apSettingsService),
#if FT_ENABLED(FT_NTP)
    _ntpSettingsService(server, &ESPFS, &_securitySettingsService),
    _ntpStatus(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_OTA)
    _otaSettingsService(server, &ESPFS, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
    _uploadFirmwareService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_MQTT)
    _mqttSettingsService(server, &ESPFS, &_securitySettingsService),
    _mqttStatus(server, &_mqttSettingsService, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_SECURITY)
    _authenticationService(server, &_securitySettingsService),
#endif
    _restartService(server, &_securitySettingsService),
    _factoryResetService(server, &ESPFS, &_securitySettingsService),
    _systemStatus(server, &_securitySettingsService) {
#ifdef PROGMEM_WWW
  // Serve static resources from PROGMEM
  WWWData::registerRoutes(
      [server, this](const String& uri, const String& contentType, const uint8_t* content, size_t len) {
        ArRequestHandlerFunction requestHandler = [contentType, content, len](AsyncWebServerRequest* request) {
          AsyncWebServerResponse* response = request->beginResponse_P(200, contentType, content, len);
          response->addHeader("Content-Encoding", "gzip");
          request->send(response);
        };
        server->on(uri.c_str(), HTTP_GET, requestHandler);
        // Serving non matching get requests with "/index.html"
        // OPTIONS get a straight up 200 response
        if (uri.equals("/index.html")) {
          server->onNotFound([requestHandler](AsyncWebServerRequest* request) {
            if (request->method() == HTTP_GET) {
              requestHandler(request);
            } else if (request->method() == HTTP_OPTIONS) {
              request->send(200);
            } else {
              request->send(404);
            }
          });
        }
      });
#else
  // Serve static resources from /www/

  server->serveStatic("/js/", ESPFS, "/www/js/").setCacheControl("max-age=6000");
  server->serveStatic("/css/", ESPFS, "/www/css/").setCacheControl("max-age=6000");
  server->serveStatic("/fonts/", ESPFS, "/www/fonts/").setCacheControl("max-age=6000");
  server->serveStatic("/app/", ESPFS, "/www/app/");
  server->serveStatic("/favicon.ico", ESPFS, "/www/favicon.ico");
  server->serveStatic("/index.html", ESPFS, "/www/index.html");
  // Serving all other get requests with "/www/index.htm"
  // OPTIONS get a straight up 200 response
  server->onNotFound([](AsyncWebServerRequest* request) {
    Serial.printf("got 404 request for %s/%s\n", request->host().c_str(), request->url().c_str());
    if (request->method() == HTTP_GET) {
      if (request->host().indexOf("connect") >= 0 || request->host().indexOf("msft") >= 0 ||
          request->host().startsWith("192.168") || request->url().equals("/") ||
          request->url().startsWith("/project") || request->hasHeader("Referer")) {
        request->redirect("/index.html");
      }
      // request->send(ESPFS, "/www/index.html");
    } else if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
#endif

// Disable CORS if required
#if defined(ENABLE_CORS)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", CORS_ORIGIN);
  // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
#endif
}

void ESP8266React::begin() {
#ifdef ESP32
  ESPFS.begin(false, "", 35);
#elif defined(ESP8266)
  ESPFS.begin();
#endif
  ESPFS.mkdir("/config");
  _wifiSettingsService.begin();
  _apSettingsService.begin();
#if FT_ENABLED(FT_NTP)
  _ntpSettingsService.begin();
#endif
#if FT_ENABLED(FT_OTA)
  _otaSettingsService.begin();
#endif
#if FT_ENABLED(FT_MQTT)
  _mqttSettingsService.begin();
#endif
#if FT_ENABLED(FT_SECURITY)
  _securitySettingsService.begin();
#endif
}

void ESP8266React::loop() {
  _wifiSettingsService.loop();
  _apSettingsService.loop();
#if FT_ENABLED(FT_OTA)
  _otaSettingsService.loop();
#endif
#if FT_ENABLED(FT_MQTT)
  _mqttSettingsService.loop();
#endif
}
