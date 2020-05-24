#ifndef WIFI_APS
#define WIFI_APS

#include <Arduino.h>

#include "my_eeprom.h"


// https://github.com/arendst/Tasmota/issues/1943


//=====================
// setup&loop functions

void    my_wifi_setup() ;
boolean my_wifi_handle() ;

int     my_wifi_scan();   // NOTE : disables wifi handling, always enable again.
String  my_wifi_get_scanned_ssid(int i);
long    my_wifi_get_scanned_rssi(int i);
boolean my_wifi_add_scanned_ap(int i, String pwd);      // enables wifi handling, 

bool    my_wifi_isConnected();
String  my_wifi_getMySSID();
String  my_wifi_getMyIPAdress();


void    my_wifi_clear_aps();

#endif 
