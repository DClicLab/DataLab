#ifdef ESP8266
#include <FS.h>
#else
#include "SPIFFS.h"
#endif

#include <TimeLib.h>
#include <map>
#include <ArduinoJson.h>
#define TSDIFFSIZE 17 //bitfied for the diff in the timestamp with the current one
using namespace std;

struct __attribute__((__packed__)) datapoint // 7 bytes only, every bit counts! :)
{
    //id 5 bit
    unsigned int id : 5;

    // tsdiff 17 bits
    unsigned int tsdiff : TSDIFFSIZE;
    float val;
};


class Storage
{
private:
    /* data */
    std::map<std::string, int> index;
    void freeSpaceIfNeeded();
    void rotateTS();
    void rotateTSIfNeeded();
    void deleteTS(time_t ts);
    time_t getFirstTS();
    time_t updateCurrentTS();
    void loadIndex();
    void saveIndex();
    uint getNameForID(int id, char * buffer);

public:
    time_t currentTS;
    void store(int id, time_t ts, float val);
    void store(const char* name, time_t ts, float val);
    long readAsJsonStream(int id, int tsstart, time_t tsfile, uint8_t *buffer, size_t maxLen, size_t index);
    Storage(/* args */);
    ~Storage() {}
};