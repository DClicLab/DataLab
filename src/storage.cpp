#if !defined(Storagecpp)
#define Storagecpp
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include "storage.h"

/*
      Storage handling in DataLab

  Each sensor value polled at a specific moment is stored in a binary format encoded with a type Datapoint:
    struct __attribute__((__packed__)) Datapoint // 7 bytes only, every bit counts! :)
    {
        //id 5 bit
        unsigned int id : 5;//the id of the sensor

        // tsdiff 17 bits
        unsigned int tsdiff : TSDIFFSIZE; // number of second since the starting of the file where the data is logged

        float val; // value of the data returned by the sensor
    };
  This allows to store all this information in 7 bytes, which is much less than what we would have with a text based
  value.

  Data is stored in files that rotates every 2**TSDIFFSIZE sec. The filename is the timestamp of creation (start)

  All data files are stored in /data/d/

  The mapping between id and sensor name is stored in /data/index

*/

void Storage::loadIndex() {
  Serial.println("In loadIndex");

  File file = LittleFS.open("/data/index", "r");
  StaticJsonDocument<1024> doc;
  // deserializeJson(doc, file);
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.printf("error deserialising index file: %s\n", error.c_str());
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject id : arr) {
    Serial.printf("Reading index entry for %s: %d \n", id["name"].as<char*>(), id["index"].as<int>());
    index.insert(std::make_pair(id["name"].as<std::string>(), id["index"].as<int>()));
  }
  file.close();
}
void Storage::saveIndex() {
  char buffer[index.size() * 64] = {0};
  buffer[0] = '[';
  for (std::map<std::string, int>::iterator it = index.begin(); it != index.end(); ++it) {
    sprintf(buffer + strlen(buffer), "{\"name\":\"%s\",\"index\":\"%d\"},", it->first.c_str(), it->second);
  }
  buffer[strlen(buffer) - 1] = ']';  // replace last , by ]
  Serial.printf("Saved index: %s\n", buffer);
  File file = LittleFS.open("/data/index", "w+");
  file.write((const uint8_t*)buffer, strlen(buffer));
  Serial.printf("File size %d\n", file.size());
  file.close();

  loadIndex();
}

void Storage::begin() {
  LittleFS.mkdir("/data");
  LittleFS.mkdir("/data/d");
  if (LittleFS.exists("/data/d/delete")) {
    File root = LittleFS.open("/data/d");
    File file;
    while (file = root.openNextFile()) {
      char buf[40];
      strcpy(buf, file.name());
      file.close();
      Serial.printf("deleting %s\n", buf);
      LittleFS.remove(buf);
    }
    root.close();
  }
  if( LittleFS.exists("/config/resetconf")){
      LittleFS.remove("/config/sensorSettings.json");
      LittleFS.remove("/config/resetconf");
      esp_restart();
  }

  loadIndex();
  currentTS = updateCurrentTS();
}

time_t getLastTsDiff(File f) {
  int i = 0;
  Datapoint point;
  point.tsdiff = 0;
  // when time is not available, the last points can be tsdiff=0 so let's see the one before
  // but not too much as it is inside an http request.
  while (point.tsdiff == 0 && i < 300 && (f.size() - (i * sizeof(Datapoint)) > 0)) {
    i++;
    f.seek(f.size() - (i * sizeof(Datapoint)));
    // Serial.printf("seeked 100 times... Pos %d / %d\n",f.size() - i*sizeof(Datapoint),f.size());
    f.read((byte*)&point, sizeof(point));
  }

  Serial.printf("GetLastTsDiff: at position -%d -- Got last point sensor index %d val %g with tsdiff: %d\n",
                i,
                point.id,
                point.val,
                point.tsdiff);
  return point.tsdiff;
}

// get first file after ts after
void Storage::updateFileList() {
  // Serial.printf("GETFIRST =entering with %lu\n", after);
  fileList.clear();
  File root = LittleFS.open("/data/d", "w");
  File file = root.openNextFile();
  // Not sure that we could take always the first file of openNextFile()
  while (file) {
    if (file.isDirectory())
      continue;
    DataFile datafile;
    strcpy(datafile.filename, file.name());
    datafile.tsstart = (time_t)atol(strrchr(file.name(), '/') + 1);
    datafile.tsdiff = getLastTsDiff(file);
    datafile.tsend = datafile.tsstart + datafile.tsdiff;
    datafile.nval = file.size() / sizeof(struct Datapoint);
    fileList.push_back(datafile);
    file = root.openNextFile();
  }
  file.close();
}

void Storage::getFileList(char* buffer) {
  updateFileList();
  int pos = sprintf(
      buffer, "{\"space\":{\"total\":\"%d\",\"used\":\"%d\"},\"files\":[", LittleFS.totalBytes(), LittleFS.usedBytes());
  for (auto&& file : fileList) {
    pos += sprintf(buffer + pos,
                   "{\"name\":\"%s\",\"start\":\"%lu\",\"end\":\"%lu\",\"nval\":\"%d\",\"diff\":\"%d\"},",
                   file.filename,
                   file.tsstart,
                   file.tsend,
                   file.nval,
                   file.tsdiff);
  }

  strcpy(buffer + strlen(buffer) - 1, "]}\0");
}

time_t Storage::updateCurrentTS() {
  time_t buffer = 0;
  // int numfiles = 0;
  Serial.println("listing files");
  updateFileList();

  for (DataFile df : fileList) {
    if (df.tsstart > buffer) {
      buffer = df.tsstart;
    }
    // numfiles++;
  }
  // Serial.printf("%d Files, last ts is: %lu\n", numfiles, buffer);
  currentTS = buffer;

  if (currentTS == 0L) {  // if no file
    rotateTS();
  }
  return currentTS;
}

// get first file after ts after
time_t Storage::getFirstTS(time_t after = 0L) {
  time_t buffer = currentTS;

  for (DataFile df : fileList) {
    if ((df.tsstart < buffer) && (df.tsstart > after)) {
      buffer = df.tsstart;
    }
  }

  Serial.printf("First TS after %lu: %lu\n", after, buffer);
  return buffer;
}

void Storage::deleteTS(time_t ts) {
  char buffer[22];
  sprintf(buffer, "/data/d/%lu", ts);
  File f = LittleFS.open(buffer);
  f.close();
  LittleFS.remove(buffer);
  updateFileList();
}

void Storage::freeSpaceIfNeeded() {
  if ((LittleFS.totalBytes() - LittleFS.usedBytes()) < 40000) {
    deleteTS(getFirstTS());  // remove older file
  }
}

// create a new file with current time
void Storage::rotateTS() {
  Serial.printf("DEBUG - rotating ts");
  char buffer[25];
  time_t now;
  time(&now);
  currentTS = now;
  sprintf(buffer, "/data/d/%lu", currentTS);
  File file = LittleFS.open(buffer, "w+");
  file.close();
  updateFileList();
}

void Storage::store(int id, time_t ts, float val) {
  Datapoint point;
  point.id = id;
  point.val = val;
  char indexname[16];
  getNameForID(id, indexname);
  if ((ts - currentTS) < 0) {
    Serial.printf("WARN - ts to store (%lu) is older than current ts (%lu), setting diff to 0", ts, currentTS);

    time_t rawtime;
    time(&rawtime);
    struct tm ts = *localtime(&rawtime);
    char buf[80];
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    Serial.printf("Local time is %s\n", buf);

    point.tsdiff = 0;
  } else if ((ts - currentTS) > 3600 * 24 * 5)  // every 5 days
  //  pow(2, TSDIFFSIZE))  // the difference is too big to be stored in TSDIFFSIZE bits we need to rotate
  {
    Serial.printf(
        "INFO - ts diff to store (%lu) is bigger than possible on current ts (%lu), rotating file.", ts, currentTS);
    rotateTS();
    point.tsdiff = 0;
  } else {
    point.tsdiff = ts - currentTS;
  }
  char buffer[22];

  sprintf(buffer, "/data/d/%lu", currentTS);
  File file = LittleFS.open(buffer, "a");
  if (!file) {
    Serial.println("file open failed");
    return;
  }
  Serial.printf("store point id:%d, tsdiff:%d, val:%g\n", point.id, point.tsdiff, point.val);
  file.write((byte*)&point, sizeof(point));
  file.close();
}
void Storage::store(const char* name, time_t ts, float val) {
  std::map<std::string, int>::iterator it = index.find(name);
  if (it == index.end()) {
    Serial.printf("No index found for %s, adding one\n", name);
    index.insert(std::make_pair(name, index.size()));
    saveIndex();
  }
  int id = index[name];
  store(id, ts, val);
}

uint Storage::getNameForID(int id, char* buffer) {
  for (std::map<std::string, int>::iterator it = index.begin(); it != index.end(); ++it) {
    if (it->second == id)
      return strlcpy(buffer, it->first.c_str(), 64);
  }
  Serial.printf("ERROR - couldn't find name for index id %d", id);
  strcpy(buffer, "Unknown");
  return 0;
}

// uint lastpos = 0;
// int id_s = 0;          // save val between stream calls
// time_t tsstart_s = 0;  // save val between stream calls
// time_t tsfile_s = 0;   // save val between stream calls

// uint reqfiles = 0;
// uint totalbytes = 0;
// bool last;

// // Read all values of a specific sensor id and format is as CSV
// // run through all files and output the sensor's values.
// long Storage::readAsJsonStream(int id, time_t tsstart, uint8_t* buffer, size_t maxLen, size_t index) {
//   time_t tsfile = 0;

//   if (index == 0) {  // new request
//     id_s = id;
//     tsstart_s = tsstart;
//     lastpos = 0;
//     tsfile = getFirstTS(tsstart);
//     reqfiles = 0;
//     totalbytes = 0;
//   } else {  // new chunk
//     if (last) {
//       last = 0;
//       return 0;
//     }
//     id = id_s;
//     tsstart = tsstart_s;
//     tsfile = tsfile_s;
//   }
//   // Serial.printf(" DEBUG - In readAsJsonStream id is %d, tsstart is %lu, index is %d heap free is %d ; maxlen %d
//   \n",
//   //               id,
//   //               tsstart,
//   //               index,
//   //               ESP.getFreeHeap(),
//   //               maxLen);
//   // Serial.printf("DEBUG - lastpos %lu\n", lastpos);

//   maxLen = maxLen * .95;  // Limit to 80% of available buffer. Using it all lead to heap corruption.

//   if (maxLen < 40) {
//     return sprintf((char*)buffer, "!%d\n", maxLen);
//   }

//   if (lastpos == 0)
//     reqfiles++;

//   char filename[22];

//   // Serial.printf("GETJSON - loading tsfile: %lu for start at %lu\n", tsfile, tsstart);

//   sprintf(filename, "/data/d/%lu", tsfile);
//   File file = LittleFS.open(filename, "r");

//   uint bufferpos = 0;
//   char pointname[64];
//   file.seek(lastpos);
//   uint fsize = file.size();
//   // Serial.printf("GETJSON - file size is %lu last pos is %u\n", fsize, lastpos);
//   uint pos;
//   bufferpos += sprintf((char*)buffer + bufferpos, "===File %lu\n", tsfile);

//   // vTaskDelay(50);
//   for (pos = file.position(); (pos < fsize) && (bufferpos < (maxLen - 40)); pos += sizeof(Datapoint)) {
//     lastpos = pos;
//     Datapoint point;
//     // yield();
//     // Serial.printf("GETJSON - reading point at %d\n",pos);
//     // no need for local buffer file.read is as fast with or without buffer.
//     file.read((byte*)&point, sizeof(point));  // read the next point.
//     getNameForID(point.id, pointname);
//     if ((id==-1 || point.id == id) && (point.tsdiff + tsfile > tsstart)) {
//       bufferpos += sprintf((char*)buffer + bufferpos,
//                            "%s,%li,%g\n",
//                            pointname,
//                            point.tsdiff + tsfile,
//                            point.val);  // format entry csv style.
//     }
//   }
//   // Serial.printf("GETJSON - wrote %d until %d out of %d\n", bufferpos, pos, fsize);

//   file.close();
//   lastpos += sizeof(Datapoint);  // this should be +=pos
//   totalbytes += bufferpos;
//   // if file is finished and we're not on the last file, check if there is another one and set tsfile_s on it.
//   if (lastpos >= fsize) {
//     // Serial.printf("GETJSON - %lu finished, ", tsfile);
//     if (tsfile != currentTS) {
//       lastpos = 0;
//       tsfile_s = getFirstTS(tsfile);
//       Serial.printf("next file available %lu\n", tsfile_s);
//       // if (totalbytes<(maxLen-40)){//if we have room in the buffer start processing next file before returning
//       if (bufferpos == 0) {  // if we have not returned anything on this file...
//         //        Serial.printf("GETJSON - We still have free buffer, let's process the next file\n");
//         bufferpos += readAsJsonStream(id, tsstart, buffer, maxLen - totalbytes, bufferpos);
//       }
//     } else {  // No more files to process
//       last = true;
//       Serial.printf("It was the last file\n");
//       bufferpos += sprintf((char*)buffer + bufferpos, "== EOT! ==");
//     }
//   }
//   // Serial.printf("GETJSON - returned  %d bytes \n", bufferpos);
//   // Serial.printf("GETJSON - returned total %d bytes for %d files\n", totalbytes, reqfiles);

//   return bufferpos;
// }

// StorageService::StorageService(AsyncWebServer* server, SecurityManager* securityManager, AsyncMqttClient* mqttClient)
// :
//     _httpGetEndpoint(Storage::read,
//                   this,
//                   server,
//                   STORAGE_ENDPOINT_PATH,
//                   securityManager,
//                   AuthenticationPredicates::IS_AUTHENTICATED),

//     _mqttClient(mqttClient) {

// }

// void Storage::read(Storage& storage, JsonObject& root){
//   JsonArray jfl = root.createNestedArray("filelist");

//   for (DataFile df : storage.fileList) {
//     JsonObject jf = jfl.createNestedObject();
//     jf["name"] = df.filename;
//     jf["tsstart"] = df.tsstart;
//     jf["tsend"] = df.tsend;
//   }
// }

#endif  // Storagecpp