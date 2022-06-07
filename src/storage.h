#include <HttpEndpoint.h>
#include <MqttPubSub.h>
#include <WebSocketTxRx.h>

#ifdef ESP8266
#include <FS.h>
#else
#define SPIFFS LittleFS
#include <LittleFS.h>
// #include "SPIFFS.h"
#endif

// #include <TimeLib.h>
#include <map>
#include <list>
#include <ArduinoJson.h>
#define TSDIFFSIZE 19  // bitfied for the diff in the timestamp with the current one
using namespace std;

#define STORAGE_ENDPOINT_PATH "/rest/storageState"
#define STORAGE_SOCKET_PATH "/ws/storageState"

struct __attribute__((__packed__)) Datapoint  // 7 bytes only, every bit counts! :)
{
  // id 5 bit
  unsigned int id : 5;  // the id of the sensor

  // tsdiff 19 bits
  unsigned int tsdiff : TSDIFFSIZE;  // number of second since the starting of the file where the data is logged
  float val;                         // value of the data returned by the sensor
};

class DataFile {
    public:
    char filename[32];
    time_t tsstart;
    time_t tsend;
    uint nval;
    unsigned int tsdiff : TSDIFFSIZE;
};

class Storage {
 private:
  /* data */
  std::map<std::string, int> index;
  void freeSpaceIfNeeded();
  void rotateTS();
  void rotateTSIfNeeded();
  time_t getFirstTS(time_t before);
  time_t updateCurrentTS();
  void loadIndex();
  void saveIndex();
  uint getNameForID(int id, char* buffer);
  void updateFileList();
  std::list<DataFile> fileList;


 public:
  static void read(Storage& storage, JsonObject& root);
  // static AsyncWebSocket ws;
  void getFileList(char* buffer);
  void deleteTS(time_t ts);
  void deleteAll();

  Storage(){
  }
  time_t currentTS;
  void store(int id, time_t ts, float val);
  void store(const char* name, time_t ts, float val);
  long readAsJsonStream(int id, time_t tsstart, uint8_t* buffer, size_t maxLen, size_t index);
  void begin(/* args */);
  ~Storage() {
  }
};

// class StorageService : public StatefulService<Storage> {
//  public:
//   StorageService(AsyncWebServer* server, SecurityManager* securityManager, AsyncMqttClient* mqttClient);

//  private:
//   HttpGetEndpoint<Storage> _httpGetEndpoint;
//   AsyncMqttClient* _mqttClient;
// };