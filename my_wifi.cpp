


#include "my_wifi.h"

#include <ESP8266WiFiMulti.h>

#include "NTPtime.h"
#include "my_eeprom.h"


ESP8266WiFiMulti wifiMulti;
boolean wifi_connected       = false;
boolean wifi_set_connecting  = true;   // need to be able to stop for manual connecting.

bool my_wifi_isConnected() {
  return wifi_connected;
}

String my_wifi_getMyIPAdress(){
  return WiFi.localIP().toString();
}
String my_wifi_getMySSID() {
  return WiFi.SSID();
}

//================
// setup functions

/**
 * read WIFI_aps from eeprom, and load to the wifi app
 */
void my_wifi_setup() {
  int no_wifi_apps = eeprom_getNoWifiAps();
  
  for (int ap_nr = 0; ap_nr < no_wifi_apps; ap_nr++) {
    
    WifiApEE wifi_app = eeprom_getWifiAp(ap_nr);
    
    char ssid_buf[EEPC_WIFI_AP_SSID_MAX_LEN];
    char pwd_buf[EEPC_WIFI_AP_PWD_MAX_LEN];
    wifi_app.ssid.toCharArray(ssid_buf, EEPC_WIFI_AP_SSID_MAX_LEN);
    wifi_app.pwd.toCharArray(pwd_buf, EEPC_WIFI_AP_PWD_MAX_LEN);
    wifiMulti.addAP(ssid_buf, pwd_buf);
  }
  return;
}


//================
// loop  functions

// Try to connect to one of a couple of WiFi networks
boolean my_wifi_handle() {


  //WiFi.status() == WL_connected;
  if (!wifi_connected) {
    static unsigned long last_update_ms = millis();
    unsigned long now_ms = millis();
    if (((now_ms - last_update_ms) > 3000) && (wifi_set_connecting)) {
      last_update_ms = now_ms;

      if (wifiMulti.run() == WL_CONNECTED ) {
        wifi_connected = true;
        Serial.println("SSID: " + WiFi.SSID() + "; IP address: " + WiFi.localIP().toString());
      } else {
        Serial.println(" connection failed");
      }

    }
  } else {
    NTPSetup();
  }
  
}


/**
 * always let this follow with wifi_ap_add_wifi_ap, to enable wifi connecting again.
 */
int my_wifi_scan() {
  wifi_set_connecting = false; // disable connecting, otherwize it will overwrite scanned wifis
  delay(1000);
  return WiFi.scanNetworks();
}

String my_wifi_get_scanned_ssid(int i) {
  return WiFi.SSID(i);
}
long my_wifi_get_scanned_rssi(int i) {
  return WiFi.RSSI(i);
}

/**
 * add an wifi ap, return true if succes/fits
 *    if with NULL argument : just enables wifi scan again.
 */
boolean my_wifi_add_scanned_ap(int i, String pwd_str) {
  if (i >= 0) {


    eeprom_addWifiAp(WiFi.SSID(i), pwd_str);

    char ssid_char[EEPC_WIFI_AP_SSID_MAX_LEN];
    WiFi.SSID(i).toCharArray(ssid_char, EEPC_WIFI_AP_SSID_MAX_LEN);
    char pwd_char[EEPC_WIFI_AP_PWD_MAX_LEN];
    pwd_str.toCharArray(pwd_char, EEPC_WIFI_AP_PWD_MAX_LEN);

    wifiMulti.addAP(ssid_char, pwd_char);
   
    eeprom_write();
  }
  
  wifi_set_connecting = true;
  
  return true;
}


void my_wifi_clear_aps() {
  eeprom_clearWifiAps();
}
