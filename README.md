# DataLab

Based on [ESP8266-react](https://github.com/rjwats/esp8266-react)

DataLab is a simple and extensible solution for data logging based on ESP32. It comes with an easy to use interface: just plug your sensors to your board, configure the web interface and voila your data is being logged locally and on any MQTT or HTTP online service. 

(add screenshots here)

## Features

Provides many of the features required for IoT/sensor data projects:
* Real time graph of sensor data
* Support for BMP180, DHT11 (more to come)
* Push json sensor values to HTTP or MQTT server
* Easy to extend sensor framework

And from [ESP8266-react](https://github.com/rjwats/esp8266-react):
* Configurable WiFi - Network scanner and WiFi configuration screen
* Configurable Access Point - Can be continuous or automatically enabled when WiFi connection fails
* Network Time - Synchronization with NTP
* Remote Firmware Updates - Enable secured OTA updates
* Security - Protected RESTful endpoints and a secured user interface

## Getting Started

### Prerequisites

You will need the following before you can get started.

* An ESP32 (Tested successfully on: Wemos, Stick5c, ttgo tdisplay)
* Some (supported) sensors 

### Uploading the firmware

The [release](https://github.com/DClicLab/DataLab/releases/latest/) includes an easy to use uploader.
1. get the latest [release](https://github.com/DClicLab/DataLab/releases/latest/)
2. unzip everything in a folder
3. connect your ESP32 to your computer
4. run ESPUploader.exe
5. select the firmware DataLab.bin and press Flash

Once the firmware is uploaded, updates can be done over-the-air (wifi).

### Getting started

(place user doc here)

### Building the firmware

You can also get the code in platformio and compile for your board.
Currently only ESP32 is supported as some code takes advantage of its multicores.
The code will be updated to also support esp8266.

Information on the interface, rest connectors and general structure can be found in the excellent [ESP8266-react](https://github.com/rjwats/esp8266-react) upon which is based DataLab.

### Supporting your own sensor

Documentation will come.

