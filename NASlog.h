#ifndef NTPlog
#define NTPlog

#include <Arduino.h>

bool nasDBLogConnection();

bool nasDBLogCO2(String kamer, int co2_ppm, int temperature);

bool influxDBsetup(String ssid, String ip_adr, long rssi);
bool influxDBLogCO2(String kamer, int co2_ppm, int temperature);

#endif
