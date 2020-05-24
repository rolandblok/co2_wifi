
#include <SoftwareSerial.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimeLib.h>  

#include "my_wifi.h"
#include "NTPtime.h"
#include "NASlog.h"

/*
  // Using MHZ_19 && SPI on arduino nano:
  #define PIN_MHZ_19_RX 3 //be aware swap with MHZ19 itself!!
  #define PIN_MHZ_19_TX 4
  //      PIN_OLED_MISO   12   // does not exist
  #define PIN_OLED_MOSI   11   // D1
  #define PIN_OLED_CLK    13   // D0
  #define PIN_OLED_DC     8    // DC
  #define PIN_OLED_CS     5    // CS
  #define PIN_OLED_RESET  10    // RES
*/

// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#define PIN_MHZ_19_RX   4    // NODEMCU:D2   be aware connect Tx on MHZ19 itself!!
#define PIN_MHZ_19_TX   15   // NODEMCU:D8 be aware connect Rx on MHZ19 itself!!
// Using SPI on nodeMCU:
//      PIN_OLED_MISO   12   // not used : [NODEMCU:D6] : fixed
#define PIN_OLED_MOSI   13   // D1  : [NODEMCU:D7] : fixed
#define PIN_OLED_CLK    14   // D0  : [NODEMCU:D5] :fixed
#define PIN_OLED_DC     16   // DC  : [NODEMCU:D0] :free choice
#define PIN_OLED_CS      5   // CS  : [NODEMCU:D1] :fixed
#define PIN_OLED_RESET  10   // RES : [NODEMCU:SD3]: free choice


// SW SPI:
//Adafruit_SSD1306 display(PIN_OLED_MOSI, PIN_OLED_CLK, PIN_OLED_DC, PIN_OLED_RESET, PIN_OLED_CS);
// HW SPI
Adafruit_SSD1306 display(PIN_OLED_DC, PIN_OLED_RESET, PIN_OLED_CS);

// ====================
// interface to mhz-19b
SoftwareSerial serial_mhz_19(PIN_MHZ_19_RX, PIN_MHZ_19_TX); // RX, TX

// https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf
#define WHZ_19B_WARMUP_TIME_MS (120000)
boolean warming_up = true;


//------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  serial_mhz_19.begin(9600);
  Serial.println("started");

  //eeprom_clear();
  eeprom_init();
  eeprom_serial();

  // WIFI
  my_wifi_setup();

  // DISPLAY
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();

}

//------------------------------------------------------------
void loop() {
  static bool serial_logging = false;

  my_wifi_handle() ;
  serial_handle();

  // ===============
  // MHZ_19B readout
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  unsigned char response[10];
  unsigned long ppm = 0;
  boolean measure_failed = false;

  static long last_meas_time = 0;
  static long last_naslog_time = 0;
  long tijd = millis();
  if (tijd - last_meas_time >  1000) {

    serial_mhz_19.write(cmd, 9);
    serial_mhz_19.flush();
        
    delay(100);

    int count = 0;
    int waited = 0;
    while (!measure_failed && (count < 9)) { // this difficult count check is because on ESP only 8 bytes were returned..
      while (!measure_failed && (serial_mhz_19.available() == 0)) {
        Serial.print(".");
        delay(100);  // wait a short moment to avoid false reading
        if (waited++ > 10) {
          Serial.println("No response after 1 seconds");
          serial_mhz_19.flush();
          measure_failed = true;
        }
      }
      int received_bytes = serial_mhz_19.readBytes(response+count, 9);
      count += received_bytes;
    }
    
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow  = (unsigned int) response[3];
    unsigned int temperature  = (unsigned int) response[4] - 40;
    ppm = (256 * responseHigh) + responseLow;

    if (serial_logging) {
      Serial.println(" serial PPM " + String(ppm));
      Serial.println(" temp [C] " + String(temperature));
    }

    // =============
    // display status
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("CO2 " + String(ppm));
    display.println("T   " + String(temperature));

    // display warmup
    display.setTextSize(1);
    if (warming_up) {
      if (tijd > WHZ_19B_WARMUP_TIME_MS) {
        // this is to prevent millis rotation
        warming_up = false;
      } else {
        if (serial_logging) {
          Serial.println("warmup " + String(tijd / 1000) + " / " + String(WHZ_19B_WARMUP_TIME_MS / 1000));
        }
        display.println("warmup " + String(tijd / 1000) + " / " + String(WHZ_19B_WARMUP_TIME_MS / 1000));
      }
    }
    
    display.println("" + getStrDate() +" "+ getStrTime());
    
    if (my_wifi_isConnected()) {
      display.println("" + my_wifi_getMySSID());
      display.println("" + my_wifi_getMyIPAdress());

      // if warm and every minute : log to nas
      if ((!warming_up) && (tijd - last_naslog_time >  60000)) {
        nasDBLogCO2("esp8266", ppm, temperature);
        last_naslog_time = millis();
        
      }
    } else {
      display.println("no wifi connection");
    }

    if (measure_failed) {
      display.setTextSize(1);
      display.println("measurement failed");
    }

    display.display();

    last_meas_time = millis();
  }

}

//==========================================
void serial_handle() {

  enum serial_input {NO_INPUT = 0, WIFI_SSID, WIFI_PWD} ;
  static enum serial_input input_on = NO_INPUT;
  
  static int number_of_networks = 0;
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil(10);
    static int wifi_selected_id = -1;

    if (command.equals("wifi")) {
      Serial.println("Scanning WiFi");
      number_of_networks = my_wifi_scan();
      for (int i = 0; i < number_of_networks; i++ ) {
        Serial.println("" + String(i) + " " + my_wifi_get_scanned_ssid(i) + " " + String(my_wifi_get_scanned_rssi(i)));
      }
      input_on = WIFI_SSID;
      Serial.println("SELECT SSID no");

    } else if ( input_on == WIFI_SSID) {
      wifi_selected_id = command.toInt();
      if (number_of_networks > wifi_selected_id) {
        Serial.print(" network selected " + String(wifi_selected_id));
        Serial.println(" " + my_wifi_get_scanned_ssid(wifi_selected_id) + " " + String(my_wifi_get_scanned_rssi(wifi_selected_id)));
        Serial.println("ENTER PASSWORD");
        input_on = WIFI_PWD;
      } else {
        Serial.println(" not available");
        my_wifi_add_scanned_ap(-1, "");
        input_on = NO_INPUT;
      }
    } else if (input_on == WIFI_PWD) {
      my_wifi_add_scanned_ap(wifi_selected_id, command);
      Serial.println("----------");
      Serial.println(" " + my_wifi_get_scanned_ssid(wifi_selected_id) + " - " + command);
      Serial.println("----------");
      input_on = NO_INPUT;
      
    } else if (command.equals("wlist")) {
      Serial.println("----------");
      Serial.println("Stored SSID + pwds : " + String(eeprom_getNoWifiAps()));
      for (int i = 0; i < eeprom_getNoWifiAps(); i++) {
        WifiApEE wifi_ap = eeprom_getWifiAp(i);
        Serial.println(" " + String(i+1) + " : " + wifi_ap.ssid + " - " + wifi_ap.pwd);
      }
      Serial.println("----------");

    } else if (command.equals("wclear")) {
      Serial.println("clearing stored SSID + pwds");
      my_wifi_clear_aps();
      Serial.println("----------");
      
    } else if (command.equals("eepl")) {
      eeprom_serial();
    } else if (command.equals("eepc")) {
      eeprom_clear();
      eeprom_serial();
    } else if (command.equals("time")) {
      Serial.println(" " + getStrTime() + " " + getStrDate());
    } else if (command.equals("restart")) {
      ESP.restart();
    } else  {
      Serial.println("commands: ");
      Serial.println("  wifi   : scan available wifi and select");
      Serial.println("  wlist  : list stored ssid + pwd");
      Serial.println("  wclear : clear stored ssid + pwd");
      Serial.println("  eepl   : list eeprom stored data");
      Serial.println("  eepc   : clear eeprom");
      Serial.println("  time   : show time");
      Serial.println("  restart: restart micro controller");
    }
  }

}
