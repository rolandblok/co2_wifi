#ifndef NTPlog
#define NTPlog

#include <Arduino.h>

bool nasDBLogConnection();

bool nasDBLogCO2(String kamer, int co2_ppm, int temperature);

#endif
