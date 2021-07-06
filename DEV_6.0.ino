/*

  DEV Template for iobroker IoT Framework

  Version: F6_6.0
  Date: 2021-07-06

  WARNING: Framework version 6 deprecates the formerly used HTTP/REST API for the
           ioBrober communication. It was replaced by MQTT. 

  This sketch is based on my ioBroker IoT Framework 6
  https://github.com/AndreasExner/ioBroker-IoT-Framework


  MIT License
  
  Copyright (c) 2021 Andreas Exner
  
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
//#define SPS30_active //WARNING the SPS30 section is not yet converted to MQTT! Please use the latest version of F5!
//#define WindSensor_active //WARNING the WindSensor section is not yet converted to MQTT! Please use the latest version of F5!
//#define ePaper_active //WARNING useable for IndoorAirSensor sketch only! Code needs to be changed for other sketches
//#define ADS1115_daylightSensorActive


//+++++++++++++++++++++++++++++++++++++++++ generic device section +++++++++++++++++++++++++++++++++++++++++++++++++

#include <ESP8266WiFi.h>

// ------------------------------------ device settings - change settings to match your requirements

String deviceID = "DEV"; //predefined sensor ID, DEV by default to prevent overwriting productive data
String deviceName = "DevTemplate"; //predefined sensor Name, DEV by default to prevent overwriting productive data

bool devMode = true; //enable DEV mode on boot (do not change)
bool debug = true; //debug to serial monitor
bool ledActive = true; //enable external status LED on boot
bool deviceActive = false; // dectivate device (all sensors) on boot (do not change until required)

int interval = 10;  // (initial) interval between measurements / actions, multiplied with the intervalDelay
int intervalDelay = 1000; // in ms

/*
 * The BSEC library for the BME680 sets the pace for the loop duration -> BSEC_LP ~ 3000 ms per request
 * For a transmission interval of 5 minutes configure an interval of 100 (*3 = 300 seconds = 5 minutes)
 * For other cases please define the intervalDelay in the loop section
 */


// ------------------------------------ MQTT section

#include <ArduinoMqttClient.h> 

int MQTT_port = 1883; // MQTT port (default 1883)
const char MQTT_broker[] = "192.168.1.240";  // IP of iobroker / MQTT broker

String MQTT_deviceRootPath = "0_userdata/0/IoT-Devices/"; // The root path for all the sensor's (devices) state and configuration subscription in your MQTT/iobroker
String MQTT_prodDataRootPath = "0_userdata/0/IoT/"; // The root path for all the sensor's productive data in your MQTT/iobroker
String MQTT_devDataRootPath = "0_userdata/0/IoT-Dev/"; // The root path for all the sensor's productive data in your MQTT/iobroker


// do not change the next lines
// topic for sensor (device) state and configuration subscription

String MQTT_topicDevice  = MQTT_deviceRootPath + deviceID + "/"; 

// topic for sensor data to send too. Choose a different path for DEV mode to avoid overwriting production data

String MQTT_topicDataDEV  = MQTT_devDataRootPath + deviceName + "/";
String MQTT_topicDataPROD  = MQTT_prodDataRootPath + deviceName + "/";
String MQTT_topicData;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// ------------------------------------ other definitions (do not change)

#define LED D4 // gpio pin for external status LED

void(* HWReset) (void) = 0; // define reset function DO NOT CHANGE
int counter;

//+++++++++++++++++++++++++++++++++++++++++ ePaper Display section +++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef ePaper_active

#include <GxEPD.h>

#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D6*/ 12);
GxEPD_Class display(io, /*RST=D6*/ 12, /*BUSY=D0*/ 16);

String ePaperDisplayLastUpdate, ePaperTemp, ePaperHumi, ePaperCO2;
bool ePaperDisplayActive = true;  // set to false if no display is attached to the device
bool ePaperDisplayActivated = false;

#endif

//+++++++++++++++++++++++++++++++++++++++++ BME680 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef BME680_active

#include <EEPROM.h>
#include "bsec.h"

bool bme680Activated = false;

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

#define STATE_SAVE_PERIOD  UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day


// Create an object of the class Bsec
Bsec iaqSensor;
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t bsecStateUpdateCounter = 0;

int bme680iaSAhistory = 0;
bool bme680Reset = false;

#endif

//+++++++++++++++++++++++++++++++++++++++++ BME280 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef BME280_active

#include <Adafruit_BME280.h>

bool bme280Activated = false;

Adafruit_BME280 bme;

int bme280PressureSeaLevel, bme280PressureSeaLevelHistory;
int bme280TempOffset = 3; 
float bme280Pressure; // raw ambient pressure, required to calibrate SCD30 Sensor

String bme280AirPressureTopicMQTT = "daswetter/0/NextHours/Location_1/Day_1/pressure_value"; //source for air pressure at sealevel, required for altitude calculation

#endif

//+++++++++++++++++++++++++++++++++++++++++ SCD30 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef SCD30_active

#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

bool scd30Activated = false;

SCD30 airSensor;
int scd30Interval = 3; // intervall in sec.
int scd30Offset = 3; // temperature offset
bool scd30AutoCal = true;

#endif

//+++++++++++++++++++++++++++++++++++++++++ ADS1115 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef ADS1115_daylightSensorActive
#define ADS1115_active
#endif

#ifdef ADS1115_active

#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads1115(0x48);

bool ads1115Activated = false;
bool ads1115DaylightSensorActivated = false;


#endif

//######################################### specific device functions #######################################################

void mqtt_parseSpecificConfig(String mqttTopic, String mqttMessage) {

  // parse sensor specific config here. This function is called automatically on every interval. 

#ifdef BME680_active
  if (mqttTopic == MQTT_topicData + "BME680_reset" && mqttMessage == "false") {bme680Reset = false;}
  else if (mqttTopic == MQTT_topicData + "BME680_reset" && mqttMessage == "true") {bme680Reset = true;}
  if (debug) {Serial.println("    bme680Reset: " + bool_to_string(bme680Reset));}
#endif

#ifdef BME280_active
  if (mqttTopic == bme280AirPressureTopicMQTT) {bme280PressureSeaLevel = mqttMessage.toInt();}
  if (debug) {Serial.println("    bme280PressureSeaLevel: " + String(bme280PressureSeaLevel));}
#endif
  
#ifdef SCD30_active
  if (mqttTopic == MQTT_topicData + "SCD30_autoCal" && mqttMessage == "false") {scd30AutoCal = false;}
  else if (mqttTopic == MQTT_topicData + "SCD30_autoCal" && mqttMessage == "true") {scd30AutoCal = true;}
  if (debug) {Serial.println("    scd30AutoCal: " + bool_to_string(scd30AutoCal));}
#endif
  
#ifdef ePaper_active  
  if (mqttTopic == MQTT_topicDataPROD + "LastUpdate") {ePaperDisplayLastUpdate = mqttMessage;}
  if (debug) {Serial.println("    ePaperDisplayLastUpdate: " + ePaperDisplayLastUpdate);}
#endif    
  
}

void mqtt_subscribeSpecificSensorConfig() {

  if (debug) {Serial.println("### MQTT_subscribeSpecificSensorConfig");}

  String subscribeConfig;
  
#ifdef BME280_active  
  subscribeConfig = bme280AirPressureTopicMQTT;
  mqttClient.subscribe(subscribeConfig);
  if (debug) {Serial.println("    subscribe topic = " + subscribeConfig);}
#endif

#ifdef SCD30_active
  subscribeConfig = MQTT_topicData + "SCD30_autoCal";
  mqttClient.subscribe(subscribeConfig);
  if (debug) {Serial.println("    subscribe topic = " + subscribeConfig);}
#endif

#ifdef ePaper_active 
  subscribeConfig = MQTT_topicData + "LastUpdate";
  mqttClient.subscribe(subscribeConfig);
  if (debug) {Serial.println("    subscribe topic = " + subscribeConfig);}
#endif

  delay(2000); // give MQTT time to subscribe/receive. 2000ms are a good point to start.

}


//######################################### setup ##############################################################

void setup(void) {

  Serial.begin(115200);  
  delay(1000);
  
  pinMode(LED, OUTPUT);

  wifi_connect();
  wifi_getState();
  mqtt_connect();

// initial communication with iobroker on boot

  bool debug_state = debug;  //debug output during setup
  debug = true;

  mqtt_getState();
  delay(1000); // give MQTT time to finish the connection
  
  mqtt_sendDeviceState(); // send initial device state 
  mqtt_send(MQTT_topicDevice + "ErrorLog", "Info: ESP8266 restart");
  mqtt_send(MQTT_topicDevice + "Reset", "true");  //set the last reset timestamp
  
  mqtt_subscribeDeviceConfig(); // subscribe device config
  mqtt_getDeviceConfig(); // parse device and sensor config
  
  mqtt_subscribeSpecificSensorConfig(); // subscribe specific sensor config
  mqtt_getDeviceConfig(); // parse device and sensor config
      
  counter = interval; // countdown for next interval
  
  debug = debug_state;

// setup sensors

#ifdef BME680_active
  if (deviceActive) {BME680_setup();}
#endif
#ifdef BME280_active
  if (deviceActive) {BME280_setup();}
#endif
#ifdef SCD30_active
  if (deviceActive) {SCD30_setup();}
#endif
#ifdef ePaper_active
  if (deviceActive && ePaperDisplayActive) {ePaper_setup();}
#endif
#ifdef ADS1115_daylightSensorActive
  if (deviceActive) {ADS1115_daylightSensorSetup();}
#endif

}

 
//####################################################################
// Loop
  
void loop(void) {

// +++++++++++++++++++++ Start loop

  bool bme680DataReady = false;

  if (debug) {Serial.println(String(counter) + "/" + String(interval));}
  
  mqtt_getState(); // check mqtt connection state
  mqtt_getDeviceConfig(); // update sensor config

  // activate inactive sensors and run sensor specific actions on every loop (interval)
  
#ifdef BME680_active
  if (deviceActive && !bme680Activated) {BME680_setup();}
  if (deviceActive && bme680Activated && bme680Reset) {BME680_reset();}
#endif
#ifdef BME280_active
  if (deviceActive && !bme280Activated) {BME280_setup();}
#endif
#ifdef SCD30_active
  if (deviceActive && !scd30Activated) {SCD30_setup();}
  if (deviceActive && scd30Activated && counter <=0) {SCD30_AutoCal();} //(interval)
#endif
#ifdef ePaper_active
  if (deviceActive && ePaperDisplayActive && !ePaperDisplayActivated) {ePaper_setup();}
#endif
#ifdef ADS1115_active
  if (deviceActive && !ads1115DaylightSensorActivated) {ADS1115_daylightSensorSetup();}
#endif


// +++++++++++++++++++++ Delay on every loop

  /* run BME680 sample and wait for new data
   *  
   * The BSEC library sets the pace for the loop interval -> LP = 3000 ms pre run
   * results in round about 0,3Hz. For a transmission interval of 5 minutes configure 
   * an interval of 100 (x3 = 300 seconds)
   * EVERY command has to be included in the if-clause!!
   * 
   * Otherwise use intervalDelay to control the loop speed
   */

#ifdef BME680_active
  if (deviceActive && bme680Activated) {
    
    int timeout = 10;
    if (debug) {Serial.print("    waiting for BME680 data: ");}
    
    while (bme680DataReady == false && timeout > 0) {
      
      bme680DataReady = iaqSensor.run();
      if (debug) {Serial.print(String(timeout) + " ");}
      --timeout;
      delay(intervalDelay);
    }
    
    if (timeout <= 0) {
      if (debug) {Serial.println(" timeout");}
      BME680_checkIaqSensorStatus(); // check sensor if BME680 run fails
    } 
    else {
      if (debug) {Serial.println(" done");}
    }
  }
#endif

  if (!bme680DataReady) {delay(intervalDelay);}


// +++++++++++++++++++++ Update device data every interval (n-th loop)

  if (counter <= 0) { 
    if (deviceActive) {mqtt_sendDeviceState();}
  }


// +++++++++++++++++++++ Process data on every loop, but transmit on every n-th interval

#ifdef BME680_active
  if (deviceActive && bme680Activated && bme680DataReady) {BME680_getData();}
#endif
#ifdef BME280_active
  if (deviceActive && bme280Activated) {BME280_getData();}
#endif
#ifdef SCD30_active
  if (deviceActive && scd30Activated) {SCD30_getData();}
#endif
#ifdef ADS1115_daylightSensorActive
  if (deviceActive && ads1115Activated) {ADS1115_daylightSensorGetData();}
#endif


// +++++++++++++++++++++ Reset Counter every n-th interval
  
  if (counter <= 0) { 
    counter = interval;

    if (deviceActive) {

// +++++++++++++++++++++ Refresh ePaper Display every interval (n-th loop)
      
      // write data to ePaper if active
  
#ifdef ePaper_active
      if (ePaperDisplayActivated) {
 
        mqtt_getDeviceConfig(); // refresh ePaperDisplayLastUpdate timestamp from iobroker
        ePaper_showData_1_54_3fields("Temperatur:", "Rel. Luftfeuchte:", "CO2 Gehalt:", ePaperTemp + " °C", ePaperHumi + " %", ePaperCO2 + " ppm", "Stand: " + ePaperDisplayLastUpdate);
      }
#endif


// +++++++++++++++++++++ finalize the interval, reset ePaper if device is not active
       
    }
    else {

#ifdef ePaper_active      
      ePaper_showData_1_54_3fields("Temperatur:", "Rel. Luftfeuchte:", "CO2 Gehalt:", "--.-- °C", "--.-- %", "---- ppm", "Stand: ----------");
#endif

    }
  } 

// +++++++++++++++++++++ finalize the loop
  
  else {counter--;}

  // Blink LED

  if (ledActive && (counter % 2)) {
    digitalWrite(LED, LOW);         
  }
  else {
    digitalWrite(LED, HIGH);
  }
}
