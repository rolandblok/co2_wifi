#include "my_eeprom.h"

#include <Arduino.h>
#include <EEPROM.h>

//=================================
// ROLAND IMPLEMENTATION hard coded
//=================================


#define EEPC_WIFI_AP_SSID_MAX_LEN (33)
#define EEPC_WIFI_AP_PWD_MAX_LEN  (64)

typedef struct WifiApChar { 
    char ssid_buf[EEPC_WIFI_AP_SSID_MAX_LEN];
    char pwd_buf[EEPC_WIFI_AP_PWD_MAX_LEN];
} WifiApChar;

typedef struct EepromMem_struct {
  byte   valid;
  byte   no_wifi_aps;
  WifiApChar wifi_aps[EEPC_WIFI_AP_MAX_APS] ; 
  byte   checksum;
} EepromMem;


//extern EepromMem eeprom_mem_glb;
EepromMem eeprom_mem_glb;

// ===============
// getters setters
// ===============
int    eeprom_getNoWifiAps() {
  return eeprom_mem_glb.no_wifi_aps;
}

void   eeprom_addWifiAp(String ssid, String pwd) {
  if ( eeprom_mem_glb.no_wifi_aps == 5 ) {
    eeprom_mem_glb.no_wifi_aps = 4; //we overwrite the last one :-)
  }
  ssid.toCharArray(eeprom_mem_glb.wifi_aps[eeprom_mem_glb.no_wifi_aps].ssid_buf, EEPC_WIFI_AP_SSID_MAX_LEN);
  pwd.toCharArray(eeprom_mem_glb.wifi_aps[eeprom_mem_glb.no_wifi_aps].pwd_buf, EEPC_WIFI_AP_PWD_MAX_LEN);
  
  eeprom_mem_glb.no_wifi_aps++;
  eeprom_write();
}

WifiApEE eeprom_getWifiAp(int id) {
  WifiApEE wifi_app = {};
  if ( (id >= 0) && (id < eeprom_mem_glb.no_wifi_aps)) {
    wifi_app.ssid = String(eeprom_mem_glb.wifi_aps[id].ssid_buf);
    wifi_app.pwd  = String(eeprom_mem_glb.wifi_aps[id].pwd_buf);
  }
  return wifi_app;
}

void eeprom_clearWifiAps() {
  eeprom_mem_glb.no_wifi_aps = 0;
  eeprom_write();
}

byte checksum(EepromMem eeprom_memo_arg) {
  return eeprom_memo_arg.no_wifi_aps;
}

/** 
 *  Use this in debugging to reset your eeprom
 */
boolean eeprom_clear() {
  EepromMem eeprom_mem_tmp = {};
  eeprom_mem_glb = eeprom_mem_tmp;
  eeprom_write();
  
  return true;
}

boolean eeprom_init() {
  EepromMem eeprom_mem_tmp = {};
  EEPROM.begin(sizeof(EepromMem));
  EEPROM.get(0, eeprom_mem_tmp);
  EEPROM.end();
  if (eeprom_mem_tmp.valid == 1) {
    if (eeprom_mem_tmp.checksum == checksum(eeprom_mem_tmp)) {
      eeprom_mem_glb = eeprom_mem_tmp;
    } else {
      Serial.println("eeprom checksum invalid");
      return false;
    }
  } else {
    Serial.println("eeprom read invalid");
    return false;
  }

  return true;
}

boolean eeprom_write() {
  eeprom_mem_glb.valid = 1;
  eeprom_mem_glb.checksum = checksum(eeprom_mem_glb);
  EEPROM.begin(sizeof(eeprom_mem_glb));
  EEPROM.put(0, eeprom_mem_glb);
  EEPROM.commit();
  EEPROM.end();

  return true;
}

void eeprom_serial() {
  Serial.println("----------");
  Serial.println("n.o. wifi aps " + String(eeprom_mem_glb.no_wifi_aps));
  for (int i = 0; i < eeprom_mem_glb.no_wifi_aps; i ++) {
    Serial.println(" " + String(i+1) + " " +String(eeprom_mem_glb.wifi_aps[i].ssid_buf));
    Serial.println("     " + String(eeprom_mem_glb.wifi_aps[i].pwd_buf));
  }
  Serial.println("----------");
}
