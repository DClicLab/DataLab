#pragma once

class AirQualityIndex {
 public:
  // calculate AQI from PM2.5 concentration (ug/m3)
  static float AQIfromPM25(float value);
  // calculate Atmo France indice from PM2.5 concentration (ug/m3)
  static int AtmofromPM25(float value);
  // calculate Atmo France indice from PM2.5 concentration (ug/m3)
  static int AtmofromPM10(float value);
  // calculate Atmo France indice from PM2.5 concentration (ug/m3)
  static int AtmofromPM(float pm2_5,float pm10);
};
