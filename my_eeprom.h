#ifndef MY_EEPROM
#define MY_EEPROM

#include <Arduino.h>

//==========================
// Rolands versie hard coded

#define EEPC_WIFI_AP_MAX_APS      (5)
#define EEPC_WIFI_AP_SSID_MAX_LEN (33)
#define EEPC_WIFI_AP_PWD_MAX_LEN  (64)
typedef struct WifiApEE { 
  String ssid;
  String pwd;
} WifiApEE;

void   eeprom_addWifiAp(String ssid, String pwd);
int    eeprom_getNoWifiAps();
WifiApEE eeprom_getWifiAp(int id);
void   eeprom_clearWifiAps();

boolean eeprom_init();
boolean eeprom_write();
boolean eeprom_clear();
void    eeprom_serial();

#endif
