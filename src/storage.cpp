#if !defined(Storagecpp)
#define Storagecpp
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include "storage.h"


void Storage::loadIndex()
{
    Serial.println("In loadIndex");
    SPIFFS.begin();
    File file = SPIFFS.open("/data/index", "r");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject id : arr)
    {
        Serial.printf("Reading index entry for %s: %d \n",id["name"].as<char*>(),id["index"].as<int>() );
        index.insert(std::make_pair(id["name"].as<std::string>(), id["index"].as<int>()));
    }
    file.close();
}
void Storage::saveIndex()
{

    char buffer[index.size() * 64] = {0};
    buffer[0] = '[';
    for (std::map<std::string, int>::iterator it = index.begin(); it != index.end(); ++it)
    {
        sprintf(buffer + strlen(buffer), "{\"name\":\"%s\",\"index\":\"%d\"},",it->first.c_str(),it->second);
    }
    buffer[strlen(buffer) - 1] = ']'; //replace last , by ]
    Serial.printf("New index file content is: %s \n",buffer );
    File file = SPIFFS.open("/data/index", "w");
    file.print(buffer);
    file.close();
}

void Storage::begin(){
    loadIndex();
    currentTS = updateCurrentTS();

}

Storage::Storage()
{
    
}

time_t Storage::updateCurrentTS()
{
    time_t buffer=0;

    Serial.println("listing files");
    File root = SPIFFS.open("/data/0");
    File file = root.openNextFile();
    //Not sure if we could take always the last file of openNextFile()
    while (file)
    {

        time_t cts = atoi(strrchr(file.name(),'/')+1);
        Serial.printf("got ts %lu for name %s and substr %s",cts,file.name(),strrchr(file.name(),'/')+1);
        if (cts > buffer){
            buffer = cts;
        }
        file = root.openNextFile();
    }
    Serial.printf("Last ts is: %lu",buffer);
    currentTS = buffer;

    if (currentTS == 0L)
    { //if no file
        rotateTS();
    }
    return currentTS;
}

//get first file after ts after
time_t Storage::getFirstTS(time_t after=0L)
{
    
    // Serial.printf("GETFIRST =entering with %lu\n", after);
    File root = SPIFFS.open("/data/0");
    File file = root.openNextFile();
    //Not sure that we could take always the first file of openNextFile()
    time_t buffer=currentTS;

    while (file)
    {
    // Serial.printf("GETFIRST =got file\n");
        time_t cts = atoi(strrchr(file.name(),'/')+1);
        // Serial.printf("GETFIRST = %lu vs %lu and after %lu...",cts, buffer, after);

        if ((cts < buffer)&&(cts>=after)){
            buffer = cts;

        }

        Serial.printf("%lu.\n", buffer);
        file = root.openNextFile();
    }
    return buffer;
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
    char buffer[25];
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
    else if ((ts - currentTS) > pow(2, 8)) //the difference is too big to be stored in TSDIFFSIZE bits we need to rotate
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
long id_s=0; //save val between stream calls
long tsstart_s=0; //save val between stream calls
long Storage::readAsJsonStream(int id, time_t tsstart, uint8_t *buffer, size_t maxLen, size_t index)
{
    Serial.printf("\nDEBUG - In readAsJsonStream id is %d, tsstart is %lu, index is %d heap free is %d ; maxlen %d \n",id,tsstart, index, ESP.getFreeHeap(),maxLen);
    Serial.printf("DEBUG - lastpos %lu\n",lastpos);
    if (index==0){
        id_s=id;
        tsstart_s = tsstart;
        lastpos=0;
    }else
    {
        id = id_s;
        tsstart = tsstart_s;
    }
    

    char filename[22];
    
    time_t tsfile = getFirstTS(tsstart);
    Serial.printf("GETJSON - loading tsfile: %lu for start %lu\n",tsfile, tsstart);

    if (maxLen > 4096)
        maxLen = 4096;//ESPAsync crashes with bigger chunks

    sprintf(filename, "/data/0/%lu", tsfile);
    File file = SPIFFS.open(filename, "r");

    uint bufferpos=0;
    char pointname[64];
    getNameForID(id,pointname);
    file.seek(lastpos);
    //no need for local buffer file.read is as fast with or without buffer.
    Serial.printf("GETJSON - file size is %lu last pos is %lu\n",file.size(), lastpos);
    for (uint pos = file.position(); (pos < file.size()) && (bufferpos<(maxLen)); pos += sizeof(datapoint))
    {
        lastpos = pos;
        datapoint point;
        Serial.printf("GETJSON - reading point at %d\n",pos);
        file.read((byte*) &point, sizeof(point));//read the next point.
        if ((point.id == id)&&(point.tsdiff+tsfile > tsstart)){
            bufferpos+=sprintf((char*) buffer + bufferpos,"%s,%li,%f\n",pointname,point.tsdiff+tsfile,point.val);//format entry csv style.
        }
    }
    file.close();
    lastpos +=sizeof(datapoint);
    Serial.printf("GETJSON - returned  %d bytes \n",bufferpos);
    return bufferpos;
}
#endif // Storagecpp