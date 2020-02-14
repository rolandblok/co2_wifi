#include "my_wifi.h"
#include "NASlog.h"


#define WIFI_AP_SSID_MAX_LEN (33)
#define WIFI_AP_PWD_MAX_LEN  (64)


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;


bool wifi_connected       = false;
bool wifi_set_connecting  = true;   // need to be able to stop for manual connecting.

bool isMyWifiConnected() {
  return wifi_connected;
}

String getMyIPAdress(){
  return WiFi.localIP().toString();
}
String getMySSID() {
  return WiFi.SSID();
}
/**
 * Try to connect to one of a couple of WiFi networks
 * return true if connected
 */
bool handleWifi() {
  
  //WiFi.status() == WL_connected;
  if (!wifi_connected) {
  //if (WiFi.status() != WL_CONNECTED) { 
    static unsigned long last_update_ms = millis();
    unsigned long now_ms = millis();
    if (((now_ms - last_update_ms) > 3000) && (wifi_set_connecting)) {
      last_update_ms = now_ms;

      if (wifiMulti.run() == WL_CONNECTED ) {
        wifi_connected = true;
        Serial.println("SSID: " + WiFi.SSID() + "; IP address: " + WiFi.localIP().toString());
        nasDBLogConnection();
      } else {
        Serial.println(">>> WIFI connection failed");
      }
      
    }
  }  
  return wifi_connected;
}


/**
 * blocking call, to use serial connection to select a wifi chanel. 
 * - first scan wifi network
 * - display option on serial
 * - wait for selection or break
 * - wait for password or break
 * - add network
 */
void connectSerialWifi() {
  // find and print available networks
  Serial.println("Scanning WiFi");
  delay(1000);  // make sure the ESP is not doing wifi stuff
  int number_of_networks = WiFi.scanNetworks();
  for (int i = 0; i < number_of_networks; i++ ) {
     Serial.println("" + String(i) + " " + WiFi.SSID(i) + " " + WiFi.RSSI(i));
  }
  Serial.println("SELECT SSID no");

  // wait for network selection, or break
  int input_time_s = 0;  // block everything for 10 seconds to get wifi selection
  while (Serial.available() <= 0) {
    delay(1000); 
    input_time_s += 1;
    Serial.print(".");
    if (input_time_s > 5) {
      Serial.println("no input - quit.");
      return; // we stop this, no input
    }
  }
  Serial.println("");

  // select and show network selection, ask pwd , or break
  String command = Serial.readStringUntil(10);
  long network_selected = command.toInt();  // this is not rubust, as non-valid return zero, which is valid.
  if ((network_selected >= 0) && (network_selected < number_of_networks)) {
    Serial.print(" network selected " + String(network_selected));
    Serial.println(" " + WiFi.SSID(network_selected));
    Serial.println("ENTER PASSWORD");
  } else {
    Serial.println(" not valid");
    return; // we stop this, not valid
  }

  // wait for password, or break
  input_time_s = 0;  // block everything for x seconds to get wifi selection
  while (Serial.available() <= 0) {
    delay(1000); 
    input_time_s += 1;
    Serial.print(".");
    if (input_time_s > 5) {
      Serial.println("no input - quit.");
      return; // we stop this, no input
    }
  }
  Serial.println("");

  // set the new wifi in ESP.
  String pwd = Serial.readStringUntil(10);
  Serial.println(" " + pwd);
  Serial.println("----------");


  char ssid_buf[WIFI_AP_SSID_MAX_LEN];
  char pwd_buf[WIFI_AP_PWD_MAX_LEN];
  WiFi.SSID(network_selected).toCharArray(ssid_buf, WIFI_AP_SSID_MAX_LEN);
  pwd.toCharArray(pwd_buf, WIFI_AP_PWD_MAX_LEN);
  wifiMulti.addAP(ssid_buf, pwd_buf);
}
