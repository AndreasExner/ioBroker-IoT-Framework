/*

  DEV Template for iobroker IoT Framework

  Version: F5_5.0
  Date: 2020-12-01

  This sketch is based on my ioBroker IoT Framework V5
  https://github.com/AndreasExner/ioBroker-IoT-Framework


  MIT License
  
  Copyright (c) 2020 Andreas Exner
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

//+++++++++++++++++++++++++++++++++++++++++ enable sensor specific functions +++++++++++++++++++++++++++++++++++++++++++++++++

#define AEX_iobroker_IoT_Framework //generic functions DO NOT CHANGE

// uncomment required sections

//#define BME280_active
//#define BME680_active
//#define SCD30_active
//#define WindSensor_active

//+++++++++++++++++++++++++++++++++++++++++ generic device section +++++++++++++++++++++++++++++++++++++++++++++++++

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// device settings - change settings to match your requirements

const char* ssid     = "<ssid>"; // Wifi SSID
const char* password = "<password>"; //Wifi password

String SensorID = "DEV"; //predefinded sensor ID, DEV by default to prevent overwriting productive data

int interval = 10;  // waiting time for the first masurement and fallback on error reading interval from iobroker

bool DevMode = true; //enable DEV mode on boot (do not change)
bool debug = true; //debug to serial monitor
bool led = true; //enable external status LED on boot
bool sensor_active = false; // dectivate sensor(s) on boot (do not change)

/*
 * build base URL's
 * Change IP/FQND and path to match your environment
 */

String baseURL_DEVICE_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Devices." + SensorID + ".";
String baseURL_DEVICE_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Devices." + SensorID + ".";

// end of device settings - don not change anything below the line until required

// define generic device URL's

String URL_IP = baseURL_DEVICE_SET + "SensorIP?value=";
String URL_RST = baseURL_DEVICE_SET + "Reset?value=";
String URL_LED = baseURL_DEVICE_GET + "LED";
String URL_MAC = baseURL_DEVICE_SET + "MAC?value=";
String URL_RSSI = baseURL_DEVICE_SET + "RSSI?value=";
String URL_DevMode = baseURL_DEVICE_GET + "DevMode";
String URL_ErrorLog = baseURL_DEVICE_SET + "ErrorLog?value=";
String URL_sensor_active = baseURL_DEVICE_GET + "SensorActive";

String baseURL_DATA_GET, baseURL_DATA_SET; // URL's for data 
String URL_SID, URL_INT; // URL's for sensor ID and interval

// other definitions

#define LED D4 // gpio pin for external status LED
void(* HWReset) (void) = 0; // define reset function DO NOT CHANGE
int counter = interval;  // countdown for next interval

//+++++++++++++++++++++++++++++++++++++++++ specific sensor section +++++++++++++++++++++++++++++++++++++++++++++++++


//######################################### setup ##############################################################

void setup(void) {

  Serial.begin(115200);  
  delay(1000);
  
  pinMode(LED, OUTPUT);

  connect_wifi();
  get_wifi_state();

// initial communication with iobroker on boot

  bool debug_state = debug;  //debug output during setup
  debug = true;

  get_dynamic_config();
  build_urls();
  send_ip();
  send_sid();
  send_rst();
     
  debug = debug_state;

// setup sensors


}

//######################################### specific device functions #######################################################

void build_urls() {

  URL_SID = baseURL_DATA_SET + "SensorID?value=" + SensorID;
  URL_INT = baseURL_DATA_GET + "Interval";
}

void send_data() {

  Serial.println("send data to iobroker");
/*
  HTTPClient http;
  
  String sendURL;

  sendURL = URL_iaDA + iaDA;
  http.begin(sendURL);
  http.GET();
  
   
  http.end();
  */
} 

//####################################################################
// Loop
  
void loop(void) {

  if (counter == 0) { 

    get_wifi_state();
    send_ip();
    
    get_dynamic_config();
    build_urls();

    get_interval();
    counter = interval;
    
  } 
  else {
    counter--;  
    Serial.println(counter);
  }

  // Blink LED

  if (led && (counter % 2)) {
    digitalWrite(LED, LOW);         
  }
  else {
    digitalWrite(LED, HIGH);
  }
  delay(1000);
}
