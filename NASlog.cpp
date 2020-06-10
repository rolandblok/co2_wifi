
#include "NASlog.h"

#include <ESP8266WiFi.h>
#include <InfluxDb.h>
#include "NTPtime.h"


bool SERIAL_DEBUG = true;

const char* db_host = "192.168.1.100";   // NAS - dataseba

bool wifiDBSend(String params);
 



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

/**
 * INFLUX version
 *    needs influx db from tobias installed in arduino IDE
 *   create database: https://docs.influxdata.com/influxdb/v1.8/introduction/get-started/
 *   arduino clinet : https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino
 */
 
// InfluxDB server url, e.g. http://192.168.1.48:8086 (don't use localhost, always server name or ip address)
#define INFLUXDB_URL "http://192.168.1.227:8086"
// InfluxDB database name 
#define INFLUXDB_DB_NAME "co2_db"

bool influx_connected = false;

// Single InfluxDB instance
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

bool influxDBsetup(String ssid, String ip_adr, long rssi) {

  if (!influx_connected) {
    
    client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME);
    
    if (client.validateConnection()) {

      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());

      Point influx_point_co2_connect("co2_connect");
      influx_point_co2_connect.addTag("device", "co2_1");
      influx_point_co2_connect.addTag("kamer", "computer");
      influx_point_co2_connect.addTag("SSID", ssid);
      influx_point_co2_connect.addField("ip", ip_adr);
      influx_point_co2_connect.addField("rssi", rssi);
      client.writePoint(influx_point_co2_connect);

      influx_connected = true;
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

  }
  return influx_connected;
}  


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

  Point influx_point_co2_data("co2_data");
  influx_point_co2_data.addTag("kamer", String(kamer));
  influx_point_co2_data.addField("temperature", temperature);
  influx_point_co2_data.addField("co2_ppm", co2_ppm);
  client.writePoint(influx_point_co2_data);
   
}
