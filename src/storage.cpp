#if !defined(Storagecpp)
#define Storagecpp
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include "storage.h"

/*
      Storage handling in DataLab

  Each sensor value polled at a specific moment is stored in a binary format encoded with a type datapoint:
    struct __attribute__((__packed__)) datapoint // 7 bytes only, every bit counts! :)
    {
        //id 5 bit
        unsigned int id : 5;//the id of the sensor

        // tsdiff 17 bits
        unsigned int tsdiff : TSDIFFSIZE; // number of second since the starting of the file where the data is logged

        float val; // value of the data returned by the sensor
    };
  This allows to store all this information in 7 bytes, which is much less than what we would have with a text based value.
  
  Data is stored in files that rotates every 2**TSDIFFSIZE. The filename is the timestamp of creation (start)

  All data files are stored in /data/0/

  The mapping between id and sensor name is stored in /data/index

*/


void Storage::loadIndex() {
  // Serial.println("In loadIndex");
  SPIFFS.begin();
  File file = SPIFFS.open("/data/index", "w");
  StaticJsonDocument<256> doc;
  deserializeJson(doc, file);
  // DeserializationError error = deserializeJson(doc, file);
  // if (error)
  //   Serial.println(F("Failed to read file, using default configuration"));

  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject id : arr) {
    // Serial.printf("Reading index entry for %s: %d \n", id["name"].as<char*>(), id["index"].as<int>());
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
  File file = SPIFFS.open("/data/index", "w");
  file.print(buffer);
  file.close();
}

void Storage::begin() {
  loadIndex();
  currentTS = updateCurrentTS();
}

Storage::Storage() {
}
// get first file after ts after
void Storage::updateFileList() {
  // Serial.printf("GETFIRST =entering with %lu\n", after);
  File root = SPIFFS.open("/data/0","w");
  File file = root.openNextFile();
  // Not sure that we could take always the first file of openNextFile()
  while (file) {
    time_t cts = atoi(strrchr(file.name(), '/') + 1);
    fileList.push_back(cts);
    Serial.printf("Found file with TS: %li\n",cts);
    file = root.openNextFile();
  }
}

time_t Storage::updateCurrentTS() {
  time_t buffer = 0;
  // int numfiles = 0;
  Serial.println("listing files");
  updateFileList();

  for (time_t cts : fileList) {
    if (cts > buffer) {
      buffer = cts;
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

  for (time_t cts : fileList) {
    if ((cts < buffer) && (cts > after)) {
      buffer = cts;
    }
  }

  Serial.printf("First TS after %lu: %lu\n",after,buffer);
  return buffer;
}

void Storage::deleteTS(time_t ts) {
  char buffer[22];
  sprintf(buffer, "/data/0/%lu", ts);
  SPIFFS.remove(buffer);
  updateFileList();
}

void Storage::freeSpaceIfNeeded() {
  if ((SPIFFS.totalBytes() - SPIFFS.usedBytes()) < 40000) {
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
  sprintf(buffer, "/data/0/%lu", currentTS);
  File file = SPIFFS.open(buffer, "w");
  file.close();
  updateFileList();
}

void Storage::store(int id, time_t ts, float val) {
  datapoint point;
  point.id = id;
  point.val = val;
  if ((ts - currentTS) < 0) {
    Serial.printf("WARN - ts to store (%lu) is older than current ts (%lu), setting diff to 0", ts, currentTS);
    point.tsdiff = 0;
  } else if ((ts - currentTS) >
             pow(2, TSDIFFSIZE))  // the difference is too big to be stored in TSDIFFSIZE bits we need to rotate
  {
    Serial.printf(
        "INFO - ts diff to store (%lu) is bigger than possible on current ts (%lu), rotating file.", ts, currentTS);
    rotateTS();
    point.tsdiff = 0;
  } else {
    point.tsdiff = ts - currentTS;
  }
  char buffer[22];
  sprintf(buffer, "/data/0/%lu", currentTS);
  File file = SPIFFS.open(buffer, "a");
  Serial.printf("store point id:%d, tsdiff:%d, val:%f\n", point.id, point.tsdiff, point.val);
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
  return 0;
}

uint lastpos = 0;
int id_s = 0;       // save val between stream calls
time_t tsstart_s = 0;  // save val between stream calls
time_t tsfile_s = 0;   // save val between stream calls

uint reqfiles = 0;
uint totalbytes = 0;
bool last;
// Read all values of a specific sensor id and format is as CSV
// run through all files and output the sensor's values.
long Storage::readAsJsonStream(int id, time_t tsstart, uint8_t* buffer, size_t maxLen, size_t index) {
  time_t tsfile = 0;

  if (index == 0) {  // new request
    id_s = id;
    tsstart_s = tsstart;
    lastpos = 0;
    tsfile = getFirstTS(tsstart);
    reqfiles = 0;
    totalbytes = 0;
  } else {  // new chunk
    if (last){
      last = 0;
      return 0;
    }
    id = id_s;
    tsstart = tsstart_s;
    tsfile = tsfile_s;
  }
  // Serial.printf(" DEBUG - In readAsJsonStream id is %d, tsstart is %lu, index is %d heap free is %d ; maxlen %d \n",
  //               id,
  //               tsstart,
  //               index,
  //               ESP.getFreeHeap(),
  //               maxLen);
  // Serial.printf("DEBUG - lastpos %lu\n", lastpos);

  maxLen = maxLen*.95; // Limit to 80% of available buffer. Using it all lead to heap corruption. 


  if (maxLen < 40) {
    return sprintf((char*)buffer, "!%d\n", maxLen);
  }

  if (lastpos == 0)
    reqfiles++;

  char filename[22];

  // Serial.printf("GETJSON - loading tsfile: %lu for start at %lu\n", tsfile, tsstart);

  sprintf(filename, "/data/0/%lu", tsfile);
  File file = SPIFFS.open(filename, "r");

  uint bufferpos = 0;
  char pointname[64];
  getNameForID(id, pointname);
  file.seek(lastpos);
  uint fsize = file.size();
  // Serial.printf("GETJSON - file size is %lu last pos is %u\n", fsize, lastpos);
  uint pos;
  bufferpos += sprintf((char*)buffer + bufferpos, "===File %lu\n", tsfile);
  
  // vTaskDelay(50);
  for (pos = file.position(); (pos < fsize) && (bufferpos < (maxLen - 40)); pos += sizeof(datapoint)) {
    lastpos = pos;
    datapoint point;
    // yield();
    // Serial.printf("GETJSON - reading point at %d\n",pos);
    // no need for local buffer file.read is as fast with or without buffer.
    file.read((byte*)&point, sizeof(point));  // read the next point.
    if ((point.id == id) && (point.tsdiff + tsfile > tsstart)) {
      bufferpos += sprintf((char*)buffer + bufferpos,
                           "%s,%li,%f\n",
                           pointname,
                           point.tsdiff + tsfile,
                           point.val);  // format entry csv style.
    }
  }
  // Serial.printf("GETJSON - wrote %d until %d out of %d\n", bufferpos, pos, fsize);

  file.close();
  lastpos += sizeof(datapoint);//this should be +=pos
  totalbytes += bufferpos;
  // if file is finished and we're not on the last file, check if there is another one and set tsfile_s on it.
  if (lastpos >= fsize) {
    // Serial.printf("GETJSON - %lu finished, ", tsfile);
    if (tsfile != currentTS) {
      lastpos = 0;
      tsfile_s = getFirstTS(tsfile);
      Serial.printf("next file available %lu\n", tsfile_s);
      // if (totalbytes<(maxLen-40)){//if we have room in the buffer start processing next file before returning
      if (bufferpos == 0) {  // if we have not returned anything on this file...
//        Serial.printf("GETJSON - We still have free buffer, let's process the next file\n");
        bufferpos += readAsJsonStream(id, tsstart, buffer, maxLen - totalbytes, bufferpos);
      }
    } else {  // No more files to process
      last = true;
      Serial.printf("It was the last file\n");
      bufferpos +=sprintf((char*)buffer + bufferpos, "== EOT! ==");
    }
  }
  // Serial.printf("GETJSON - returned  %d bytes \n", bufferpos);
  // Serial.printf("GETJSON - returned total %d bytes for %d files\n", totalbytes, reqfiles);
  
  return bufferpos;
}
#endif  // Storagecpp