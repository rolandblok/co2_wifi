#ifndef MY_WIFI
#define MY_WIFI

#include <Arduino.h>

      
bool   isMyWifiConnected();
void   handleWifi();
void   connectSerialWifi();
String getMySSID();
String getMyIPAdress();

#endif
