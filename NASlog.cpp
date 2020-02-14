
#include "NASlog.h"

#include <ESP8266WiFi.h>
#include "NTPtime.h"


bool SERIAL_DEBUG = true;

const char* db_host = "192.168.1.100";   // NAS - dataseba

WiFiServer data_server(1234);

bool wifiDBSend(String params);
 

/**
 * Log a connection to the network
 */
bool nasDBLogConnection() {
   return wifiDBSend("connect_log.php?who=co2logger");
}

bool nasDBLogCO2(String kamer, int co2_ppm, int temperature){

   String arg_kamer = "kamer=" + String(kamer);
   String arg_tijd = "tijd=" + getStrDate() +"%20"+ getStrTime();
   String arg_co2 = "co2=" + String(co2_ppm);
   String arg_temp = "temp=" + String(temperature);
   return wifiDBSend("log_co2.php?"+arg_kamer +"&"+ arg_tijd+"&"+arg_co2+"&"+arg_temp);
}


/**
 * Write Data to the database
 *  params : p1=v1&p2=v2
 * 
 */
bool wifiDBSend(String params) {
  
   bool success = false;

   if (SERIAL_DEBUG) {
      Serial.print("# WIFICONNECT: wifiDBSend: data-base store:  ");
      Serial.println(params);
   }
  
   // Use WiFiClient class to create TCP connections
   WiFiClient   client;
   const int httpPort = 80;
   if (!client.connect(db_host, httpPort)) {
     Serial.println("# WIFICONNECT: wifiDBSend: connection failed");
     return success;
   }
  
   // We now create a URI for the request
   String url = "/~Roland/php/" + params;
  
   // This will send the request to the server
   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + db_host + "\r\n" + 
                "Connection: close\r\n\r\n");
   //  delay(500);

   // Read all the lines of the reply from server
   String line = "";
   while(client.available()) {
      line += client.readStringUntil('\r');
      if (line == "ok") {
         success = true;
      }
   } 
   
   if (SERIAL_DEBUG) {
       Serial.print("# WIFICONNECT: wifiDBSend: sendresult: ");
       Serial.println(line);
   }

  
  return success;
}
