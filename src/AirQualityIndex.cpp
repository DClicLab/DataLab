#include "AirQualityIndex.h"

// https://en.wikipedia.org/wiki/NowCast_(air_quality_index)
struct AQIBreakpoint {
  float c;  // PM2.5 concentration
  float i;  // AQI
} aqiBreakpoints[] = {
    {0.0f, 0},      // bottom line
    {12.0f, 50},    // good
    {35.4f, 100},   // moderate
    {55.4f, 150},   // unhealthy for sensitive groups
    {150.4f, 200},  // unhealthy
    {250.4f, 300},  // very unhealthy
    {320.4f, 400},  // hazardous
    {500.4f, 500}   // hazardous
};

//https://atmo-france.org/wp-content/uploads/2020/12/guide_calcul_nouvel_indice_ATMO_VF_version14decembre2020.pdf
struct AtmoFrancePM2_5Breakpoint {
  float c;  // PM2.5 concentration
  int i;  // Atmo France PM2_5 indice
} AFPM2_5Breakpoints[] = {
    {0.0f, 1},      // Bon
    {11.0f, 2},    // Moyen
    {21.0f, 3},   // Dégradé
    {26.0f, 4},   // Mauvais
    {51.0f, 5},  // Très Mauvais
    {75.0f, 6}  // Extrêmement Mauvais
};

//https://atmo-france.org/wp-content/uploads/2020/12/guide_calcul_nouvel_indice_ATMO_VF_version14decembre2020.pdf
const struct AtmoFrancePM10Breakpoint {
  float c;  // PM10 concentration
  int i;  // Atmo France PM10 indice
} AFPM10Breakpoints[] = {
    {0.0f, 1},      // Bon
    {21.0f, 2},    // Moyen
    {41.0f, 3},   // Dégradé
    {51.0f, 4},   // Mauvais
    {101.0f, 5},  // Très Mauvais
    {150.0f, 6}  // Extrêmement Mauvais
};

float AirQualityIndex::AQIfromPM25(float value) {
  // capping
  if (value >= 500) {
    value = 500;
  }

  for (int i = 1; i <= 7; i++) {
    float ch = aqiBreakpoints[i].c;
    if (value < ch) {
      float cl = aqiBreakpoints[i - 1].c;
      float ih = aqiBreakpoints[i].i;
      float il = aqiBreakpoints[i - 1].i;
      return (value - cl) / (ch - cl) * (ih - il) + il;
    }
  }
  return 500;
}

int AirQualityIndex::AtmofromPM25(float value) {

  for (int i = 1; i <= 5; i++) {
    float ch = AFPM2_5Breakpoints[i].c;
    if (value < ch) {
      return AFPM2_5Breakpoints[i-1].i;
    }
  }

  return AFPM2_5Breakpoints[5].i;
}


int AirQualityIndex::AtmofromPM10(float value) {

  for (int i = 1; i <= 5; i++) {
    float ch = AFPM10Breakpoints[i].c;
    if (value < ch) {
      return AFPM10Breakpoints[i-1].i;
    }
  }

  return AFPM10Breakpoints[5].i;
}

int AirQualityIndex::AtmofromPM(float pm2_5,float pm10) {
  const int afpm2_5 = AirQualityIndex::AtmofromPM25(pm2_5);
  const int afpm10 = AirQualityIndex::AtmofromPM10(pm10);
  return (afpm10 > afpm2_5) ? afpm10 : afpm2_5;
}
