#if !defined(Storagecpp)
#define Storagecpp
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include "storage.h"


void Storage::loadIndex()
{
    SPIFFS.begin();
    File file = SPIFFS.open("/data/index", "r");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject id : arr)
    {
        index.insert(std::make_pair(id["name"].as<std::string>(), id["index"].as<int>()));
    }
}
void Storage::saveIndex()
{
    char buffer[index.size() * 64];
    buffer[0] = '[';
    for (std::map<std::string, int>::iterator it = index.begin(); it != index.end(); ++it)
    {
        sprintf(buffer, "{\"name\":\"%s\",\"index\":\"%d\"},",it->first.c_str(),it->second);
    }
    buffer[strlen(buffer) - 1] = ']'; //replace last , by ]
    File file = SPIFFS.open("/data/index", "w");
    file.print(buffer);
}

Storage::Storage()
{
    currentTS = updateCurrentTS();
}

time_t Storage::updateCurrentTS()
{

    char buffer[11];
    buffer[0] = 0;

    File root = SPIFFS.open("/data/0/");
    File file = root.openNextFile();
    //Not sure if we could take always the last file of openNextFile()
    while (file)
    {
        char cname[11]; //current filename
        strncpy(cname, file.name(), sizeof(cname));

        for (unsigned int i = 0; i < strlen(cname) - 1; i++)
        {
            if (cname[i] > buffer[i])
                strcpy(buffer, cname);
        }
        file = root.openNextFile();
    }

    currentTS = atol(buffer);
    if (currentTS == 0L)
    { //if no file
        rotateTS();
    }
    return currentTS;
}

time_t Storage::getFirstTS()
{
    char buffer[11];

    snprintf(buffer, sizeof(buffer), "%lu", currentTS);

    File root = SPIFFS.open("/data/0/");
    File file = root.openNextFile();
    //Not sure that we could take always the first file of openNextFile()
    while (file)
    {
        char cname[11]; //current filename
        strncpy(cname, file.name(), sizeof(cname));

        for (unsigned int i = 0; i < strlen(cname) - 1; i++)
        {
            if (cname[i] < buffer[i])
                strcpy(buffer, cname);
        }
        file = root.openNextFile();
    }
    return atol(buffer);
}

void Storage::deleteTS(time_t ts)
{
    char buffer[22];
    sprintf(buffer, "/data/0/%lu", ts);
    SPIFFS.remove(buffer);
}

//CHeck if a new file is needed
void Storage::rotateTSIfNeeded()
{
    if ((now() - currentTS) > 86400L) //current file older than 24h
        rotateTS();
}

void Storage::freeSpaceIfNeeded()
{
    if ((SPIFFS.totalBytes() - SPIFFS.usedBytes()) < 40000)
    {
        deleteTS(getFirstTS()); //remove older file
    }
}

//create a new file with current time
void Storage::rotateTS()
{
    char buffer[22];
    currentTS = now();
    sprintf(buffer, "/data/0/%lu", currentTS);
    File file = SPIFFS.open(buffer, "w");
    file.close();
}

void Storage::store(int id, time_t ts, float val)
{
    datapoint point;
    point.id = id;
    point.val = val;
    if ((ts - currentTS) < 0)
    {
        Serial.printf("WARN - ts to store (%lu) is older than current ts (%lu), setting diff to 0", ts, currentTS);
        point.tsdiff = 0;
    }
    else if ((ts - currentTS) > pow(2, TSDIFFSIZE)) //the difference is too big to be stored in TSDIFFSIZE bits we need to rotate
    {
        rotateTS();
        point.tsdiff = 0;
    }
    else
    {
        point.tsdiff = ts - currentTS;
    }
    char buffer[22];
    sprintf(buffer, "/data/0/%lu", currentTS);
    File file = SPIFFS.open(buffer, "a");
    Serial.printf("store point id:%d, tsdiff:%d, val:%f",point.id,point.tsdiff,point.val);
    file.write((byte *)&point, sizeof(point));
    file.close();
}
void Storage::store(const char *name, time_t ts, float val)
{
    std::map<std::string, int>::iterator it = index.find(name);
    if (it == index.end())
    {
        Serial.printf("No index found for %s, adding one\n", name);
        index.insert(std::make_pair(name, index.size()));
        saveIndex();
    }
    int id = index[name];
    store(id, ts, val);
}

uint Storage::getNameForID(int id, char * buffer){
    for (std::map<std::string, int>::iterator it = index.begin(); it != index.end(); ++it)
    {
        if (it->second == id)
            return strlcpy(buffer,it->first.c_str(),64);
    }
    Serial.printf("ERROR - couldn't find name for index id %d",id);
    return 0;
}

long lastpos=0;
long Storage::readAsJsonStream(int id, int tsstart, time_t tsfile, uint8_t *buffer, size_t maxLen, size_t index)
{
    Serial.printf("\nDEBUG - In readAsJsonStream index is %d heap free is %d ; maxlen %d \n",index, ESP.getFreeHeap(),maxLen);
    Serial.printf("DEBUG - lastpos %lu\n",lastpos);
    if (index==0)
        lastpos=0;
    char filename[22];
    

    if (maxLen > 4096)
        maxLen = 4096;//ESPAsync crashes with bigger chunks

    sprintf(filename, "/data/0/%lu", tsfile);
    File file = SPIFFS.open(filename, "r");

    long bufferpos=0;
    char pointname[64];
    getNameForID(id,pointname);
    file.seek(lastpos);
    //no need for local buffer file.read is as fast with or without buffer.
    for (long pos = file.position(); (pos < file.size()) && (bufferpos<(maxLen)); pos += sizeof(datapoint))
    {
        lastpos = pos;
        datapoint point;
        file.read((byte*) &point, sizeof(point));//read the next point.
        if (point.id == id){
            Serial.printf("read point id:%d, tsdiff:%d, val:%f | pos:%d/%d bufferpos:%d/%d\n",point.id,point.tsdiff,point.val,pos,file.size(),bufferpos,maxLen);
            bufferpos+=sprintf((char*) buffer + bufferpos,"%s,%d,%f\n",pointname,point.tsdiff+tsfile,point.val);//format entry csv style.
        }
    }
    file.close();
    return bufferpos;
}
#endif // Storagecpp