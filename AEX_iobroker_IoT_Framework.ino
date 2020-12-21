/*

  Shared functions for iobroker IoT Framework

  Version: V5.3.2
  Date: 2020-12-21

  Supported features / sensors:

  - Wifi/HTTP Client (no tls)
  - BME280
  - BME680
  - SCD30
  - SPS30
  - WindSensor
  - ePaper Displays

  https://github.com/AndreasExner/ioBroker-IoT-Framework

  IMPORTANT:

  If your sketch use wind direction sensor (RS485), you HAVE to change the 
  serial output for debug and runtime information (Serial.print) to serial1. 

  
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

#ifdef AEX_iobroker_IoT_Framework
//######################################### generic sensor functions #######################################################

void connect_wifi() {

  Serial1.println("### connect_wifi");

  WiFi.begin(ssid, password);
  Serial1.print("    Connecting to ");
  Serial1.print(ssid); Serial1.println(" ...");

  digitalWrite(LED, HIGH);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial1.print(++i); Serial1.print(' ');
    if (i > 20) {
      Serial1.println("    Connection failed!");
      reboot_on_error();
    }
  }

  Serial1.println('\n');
  Serial1.println("    Connection established!");
  Serial1.print("    IP address:\t");
  Serial1.println(WiFi.localIP());
}

void get_wifi_state() {

  if (debug) {
    Serial1.println("### get_wifi_state");
  }

  if (WiFi.status() == WL_CONNECTED) {

    String wifiRSSI = String(WiFi.RSSI());
    if (debug) {
      Serial1.println("    Wifi RSSI: " +  wifiRSSI);
    }

    HTTPClient http;

    String sendURL = URL_RSSI + wifiRSSI;
    http.begin(sendURL);
    http.GET();

    http.end();
  }
  else {
    Serial1.println("    Wifi not connected, try reconnect");
    connect_wifi();
  }
}

void reboot_on_error() {

  Serial1.println('\n');
  Serial1.println("Performing reboot in 30 seconds....");

  int i = 0;
  while (i < 30) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(600);
    Serial1.print(++i); Serial1.print(' ');
  }

  HWReset();
}

void send_ErrorLog(String Error_Msg) {

  if (debug) {
    Serial1.println("### send_ErrorLog");
  }

  HTTPClient http;

  String Send_URL = URL_ErrorLog + Error_Msg;
  Send_URL.replace(" ", "+");

  http.begin(Send_URL);
  http.GET();
  http.end();

  if (debug) {
    Serial1.println("    Send ErrorLog: " + Error_Msg);
  }
}

void get_interval() {

  if (debug) {Serial1.println("### get_interval");}

  HTTPClient http;

  //if (debug) {Serial1.println("    Get Interval URL_INT = " + URL_INT);}
  http.begin(URL_INT);
  http.GET();

  interval = http.getString().toInt();
  if (debug) {Serial1.println("    New Interval = " + String(interval));}

  http.end();
}

void get_dynamic_config() {

  if (debug) {
    Serial1.println("### get_dynamic_config");
  }

  HTTPClient http;

  // get LED setting

  //if (debug) {Serial1.println("   Get LED setting URL_LED = " + URL_LED);}
  http.begin(URL_LED);
  http.GET();
  if (http.getString() == "true") {
    led = true;
  }
  else {
    led = false;
  }

  if (debug) {
    Serial1.println("    New LED setting = " + bool_to_string(led));
  }

  // get SensorActive

  //if (debug) {Serial1.println("    Get LED SensorActive URL_sensor_active = " + URL_sensor_active);}
  http.begin(URL_sensor_active);
  http.GET();

  if (http.getString() == "true") {
    sensor_active = true;
  }
  else {
    sensor_active = false;
  }

  if (debug) {
    Serial1.println("    New Sensor Active State = " + bool_to_string(sensor_active));
  }

  // get DevMode

  //if (debug) {Serial1.println("    Get DevMode URL_DevMode = " + URL_DevMode);}
  http.begin(URL_DevMode);
  int httpcode = http.GET();

  if (httpcode != 200) {
    Serial1.println("    Error getting DevMode -> switch to DevMode = true");
    DevMode = true;
  }
  else {
    if (http.getString() == "true") {
      DevMode = true;
    }
    else {
      DevMode = false;
    }
  }

  if (debug) {
    Serial1.println("    DevMode = " + bool_to_string(DevMode));
  }

  /*
     get BaseURLs based on DEV mode
     In DEV Mode all date will be written to the dev sction in iobroker
     otherwise productive data may be overwritten
  */

  if (DevMode == true) {

    http.begin(baseURL_DEVICE_GET + "baseURL_GET_DEV");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_GET = http.getString();
    }
    else {
      Serial1.println("    Error getting baseURL_DATA_GET");
    }

    http.begin(baseURL_DEVICE_GET + "baseURL_SET_DEV");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_SET = http.getString();
    }
    else {
      Serial1.println("    Error getting baseURL_DATA_SET");
    }
  }
  else {

    http.begin(baseURL_DEVICE_GET + "baseURL_GET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_GET = http.getString();
    }
    else {
      Serial1.println("    Error getting baseURL_DATA_GET");
    }

    http.begin(baseURL_DEVICE_GET + "baseURL_SET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_SET = http.getString();
    }
    else {
      Serial1.println("    Error getting baseURL_DATA_SET");
    }
  }

  baseURL_DATA_GET.replace("\"", "");
  baseURL_DATA_SET.replace("\"", "");

  if (debug) {
    Serial1.println("    baseURL_DATA_GET = " + baseURL_DATA_GET);
    Serial1.println("    baseURL_DATA_SET = " + baseURL_DATA_SET);
  }
}

void send_ip() {

  if (debug) {
    Serial1.println("### send_ip");
  }

  HTTPClient http;

  String sendURL = URL_IP + WiFi.localIP().toString();

  http.begin(sendURL);
  http.GET();
  http.end();

  if (debug) {
    Serial1.println("    Local IP: " + WiFi.localIP().toString());
  }
}

void send_sid() {

  byte mac[6];

  if (debug) {
    Serial1.println("### send_sid");
  }
  if (debug) {
    Serial1.println("    SensorID = " + SensorID);
  }

  HTTPClient http;

  http.begin(URL_SID);
  http.GET();

  WiFi.macAddress(mac);

  String WifimacAddress = String(mac[5], HEX) + ":";
  WifimacAddress += String(mac[4], HEX) + ":";
  WifimacAddress += String(mac[3], HEX) + ":";
  WifimacAddress += String(mac[2], HEX) + ":";
  WifimacAddress += String(mac[1], HEX) + ":";
  WifimacAddress += String(mac[0], HEX);

  if (debug) {
    Serial1.println("    Wifi MAC: " +  WifimacAddress);
  }

  String sendURL = URL_MAC + WifimacAddress;
  http.begin(sendURL);
  http.GET();

  http.end();
}

void send_rst() {

  if (debug) {
    Serial1.println("### send_rst");
  }

  HTTPClient http;

  String sendURL = URL_RST + "true";

  http.begin(sendURL);
  http.GET();
  http.getString();
  http.end();

  send_ErrorLog("Info: ESP8266 restart");
}

String bool_to_string(bool input) {

  if (input == false) {
    return "false";
  }
  else if (input == true) {
    return "true";
  }
  else {
    return "false";
  }

}

String hex_to_string(uint8_t hex) {
  
  char hex_char[4];
  sprintf(hex_char, "%02x", hex);
  String hex_string = hex_char;
  hex_string.toUpperCase();
  return ("0x" + hex_string);
}
#endif

#ifdef ePaper_active
//######################################### BME280 functions ########################################################

void ePaper_setup() {

  display.init();
  display.eraseDisplay();
  ePaper_showData_1_54_3fields("Temperatur:", "Rel. Luftfeuchte:", "CO2 Gehalt:", "--.-- °C", "--.-- %", "---- ppm", "Stand: ----------");
  ePaperDisplay_activated = true;
}

void ePaper_get_LastUpdate() {

  if (debug) {Serial1.println("### get_ePaper_LastUpdate");}

  HTTPClient http;

  http.begin(URL_LastUpdate);
  http.GET();
  LastUpdate = http.getString();
  http.end();

  LastUpdate.remove(0,1);
  LastUpdate.remove(LastUpdate.length() - 1, 1);

  if (debug) {Serial1.println("    LastUpdate = " + LastUpdate);}
}

void ePaper_get_dynamic_config() {

  if (debug) {Serial1.println("### ePaper_get_dynamic_config");}

  HTTPClient http;

  // Get ePaperDisplay_active

  http.begin(URL_ePaperDisplay_active);
  http.GET();
  if (http.getString() == "true") {
    ePaperDisplay_active = true;
  }
  else {
    ePaperDisplay_active = false;
  }

  if (debug) {
    Serial1.println("    New SePaperDisplay_active setting = " + bool_to_string(ePaperDisplay_active));  
  }
  http.end();
}

void ePaper_showData_1_54_3fields(String field1_name, String field2_name, String field3_name, String field1_value, String field2_value, String field3_value, String ts_update) {
  
  const GFXfont* font_b = &FreeMonoBold18pt7b;
  const GFXfont* font_a = &FreeMonoBold9pt7b;
  const GFXfont* font_c = &FreeSans9pt7b;
  
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(0, 10);
  display.setFont(font_a);
  display.println(field1_name);
  display.setCursor(10, 45);
  display.setFont(font_b);
  display.println(field1_value);

  display.setCursor(0, 70);
  display.setFont(font_a);
  display.println(field2_name);
  display.setCursor(10, 102);  
  display.setFont(font_b);
  display.println(field2_value);

  display.setCursor(0, 132);
  display.setFont(font_a);
  display.println(field3_name);
  display.setCursor(10, 163);
  display.setFont(font_b);
  display.println(field3_value);

  display.setCursor(10, 197);
  display.setFont(font_c);
  display.println(ts_update);
  
  display.update();
}

#endif

#ifdef BME280_active
//######################################### BME280 functions ########################################################

void BME280_setup() {

  if (debug) {
    Serial1.println("### BME280_setup");
  }

  bme.begin(0x76);

  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16,  // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X16,  // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_125 );

  BME280_activated = true;

}

void BME280_get_data() {

  if (debug) {
    Serial1.println("### BME280_get_data");
  }

  bme280_humi = String(bme.readHumidity());
  bme280_temp = String(bme.readTemperature());
  bme280_pressure = bme.readPressure() / 100.0;
  bme280_airp = String(bme280_pressure);
  bme280_alti = bme.readAltitude(pressure_sl);
}

void BME280_get_sealevel_pressure() {

  if (debug) {
    Serial1.println("### BME280_get_sealevel_pressure");
  }

  HTTPClient http;

  http.begin(URL_PRESL);  // get pressure at sea level
  http.GET();
  http.end();

  pressure_sl = http.getString().toInt();

  if (debug) {
    Serial1.print("    Pressure at sea level = ");
    Serial1.print(pressure_sl);
    Serial1.print("\n");
  }
}

void BME280_serial_output() {

  String output = "BME280 -- ";
  output += "bme280_temp=" + bme280_temp;
  output += ", bme280_humi=" + bme280_humi;
  output += ", bme280_airp=" + bme280_airp;
  //output += ", bme280_alti=" + bme280_alti;

  Serial1.println(output);
}
#endif

#ifdef BME680_active
//######################################### BME680 functions (BOSCH BSEC LP example) ########################################################

void BME680_setup() {

  if (debug) {
    Serial1.println("### BME680_setup");
  }

  String output;

  EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1); // 1st address for the length
  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "    BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial1.println(output);
  BME680_checkIaqSensorStatus();

  iaqSensor.setConfig(bsec_config_iaq);
  BME680_checkIaqSensorStatus();

  BME680_loadState();

  bsec_virtual_sensor_t sensorList[5] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 5, BSEC_SAMPLE_RATE_LP);
  BME680_checkIaqSensorStatus();

  BME680_activated = true;
}

void BME680_checkIaqSensorStatus() {

  String output;

  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "    BSEC error code : " + String(iaqSensor.status);
      Serial1.println(output);
      send_ErrorLog("Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BSEC warning code : " + String(iaqSensor.status);
      Serial1.println(output);
      send_ErrorLog("Warning: BME680 " + output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "    BME680 error code : " + String(iaqSensor.bme680Status);
      Serial1.println(output);
      send_ErrorLog("Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial1.println(output);
      send_ErrorLog("Warning: BME680 " + output);
    }
  }
  iaqSensor.status = BSEC_OK;
}

void BME680_loadState() {

  if (debug) {
    Serial1.println("### BME680_loadState");
  }

  if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in EEPROM
    Serial1.println("    Reading state from EEPROM: ");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      bsecState[i] = EEPROM.read(i + 1);
      if (debug) {
        Serial1.print(bsecState[i], HEX);
      }
    }
    if (debug) {
      Serial1.print("\n");
    }

    iaqSensor.setState(bsecState);
    BME680_checkIaqSensorStatus();

    HTTPClient http;  //send load state to iobroker
    String sendURL = URL_BME680_loadState + "true";
    http.begin(sendURL);
    http.GET();
    http.end();

  } else {
    // Erase the EEPROM with zeroes
    Serial1.println("    Erasing EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
      EEPROM.write(i, 0);

    EEPROM.commit();

    HTTPClient http;  //send erase EEPROM to iobroker
    String sendURL = URL_BME680_eraseEEPROM + "true";
    http.begin(sendURL);
    http.GET();
    http.end();
  }
}

void BME680_updateState() {

  if (debug) {
    Serial1.println("### BME680_updateState");
  }

  bool update = false;
  
  // Set a trigger to save the state. Here, the state is saved every STATE_SAVE_PERIOD with the first state being saved once the algorithm achieves full calibration, i.e. iaqAccuracy = 3
  
  if (stateUpdateCounter == 0) {
    if (iaqSensor.iaqAccuracy >= 3) {
      update = true;
      stateUpdateCounter++;
    }
  } else {

    // Update every STATE_SAVE_PERIOD milliseconds
    
    if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
      update = true;
      stateUpdateCounter++;
    }
  }

  if (update) {
    iaqSensor.getState(bsecState);
    BME680_checkIaqSensorStatus();

    Serial1.println("    Writing state to EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE ; i++) {
      EEPROM.write(i + 1, bsecState[i]);
      if (debug) {
        Serial1.print(bsecState[i], HEX);
      }
    }
    if (debug) {
      Serial1.print("\n");
    }

    EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
    EEPROM.commit();

    HTTPClient http;  //send update state to iobroker
    String sendURL = URL_BME680_updateState + "true";
    http.begin(sendURL);
    http.GET();
    http.end();
  }
}

void BME680_get_data() {

  if (debug) {
    Serial1.println("### BME680_get_data");
  }

  iaqD = String(iaqSensor.iaq);
  iaDA = String(iaqSensor.iaqAccuracy);
  iaqS = String(iaqSensor.staticIaq);
  iaSA = String(iaqSensor.staticIaqAccuracy);
  VOCe = String(iaqSensor.breathVocEquivalent);
  temp = String(iaqSensor.temperature);
  humi = String(iaqSensor.humidity);

  BME680_updateState();

  // if accuracy changed send data ASAP to iobroker (update accuracy value)

  if (iaqSensor.staticIaqAccuracy != iaSAhistory) {
    counter = 0;
    iaSAhistory = iaqSensor.staticIaqAccuracy;
  }
}

void BME680_reset() {

  if (debug) {
    Serial1.println("### BME680_reset");
  }

  HTTPClient http;

  String sendURL;

  sendURL = URL_BME680_reset_get;
  http.begin(sendURL);
  http.GET();

  String result = http.getString();

  if (debug) {
    if (debug) {
      Serial1.println("    Reset: " + result);
    }
  }

  if (result == "true") {

    // Erase the EEPROM with zeroes
    if (debug) {
      Serial1.println("    Erasing EEPROM");
    }

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
      EEPROM.write(i, 0);

    EEPROM.commit();

    sendURL = URL_BME680_reset_set + "false";
    http.begin(sendURL);
    http.GET();
    sendURL = URL_BME680_eraseEEPROM + "true";
    http.begin(sendURL);
    http.GET();
  }

  http.end();
}

void BME680_serial_output() {

  String output = "BME680 -- ";
  output += "iaqD=" + iaqD;
  output += ", iaDA=" + iaDA;
  output += ", iaqS=" + iaqS;
  output += ", iaSA=" + iaSA;
  output += ", VOCe=" + VOCe;
  output += ", Temp=" + temp;
  output += ", Humi=" + humi;

  Serial1.println(output);
}
#endif

#ifdef SCD30_active
//######################################### SCD30 functions ########################################################

void SCD30_setup() {

  if (debug) {
    Serial1.println("### SCD30_setup");
  }

  Wire.begin();

  if (airSensor.begin(Wire) == false) {

    if (debug) {
      Serial1.println("    Air sensor not detected. Please check wiring");
    }
    send_ErrorLog("Error: SCD30 Air sensor not detected. Please check wiring");
    reboot_on_error();
  }
  else {
    if (debug) {
      Serial1.println("     Air sensor found.");
    }
  }

  airSensor.setMeasurementInterval(scd30_interval);
  //airSensor.setAltitudeCompensation(scd30_altitude);
  airSensor.setAmbientPressure(1013);
  airSensor.setTemperatureOffset(scd30_offset);

  SCD30_activated = true;
}

void SCD30_get_data() {

  if (debug) {
    Serial1.println("### SCD30_get_data");
  }

  // collect SCD30 data

  if (airSensor.dataAvailable())
  {
    scd30_co2 = String(airSensor.getCO2());
    scd30_temp = String(airSensor.getTemperature());
    scd30_humi = String(airSensor.getHumidity());
  }
  else {
    Serial1.println("    SCD30 no data");
  }

  if (debug) {
    Serial1.println("    Ambient pressure = " + String(bme280_pressure));
  }
  airSensor.setAmbientPressure(bme280_pressure); // update ambient pressure preset from BME280

}

void SCD30_AutoCal() {

  if (debug) {Serial1.println("### SCD30_AutoCal");}

  String scd30_autoCal_get = bool_to_string(airSensor.getAutoSelfCalibration());
  if (debug) {Serial1.println("    scd30_autoCal_get = " + scd30_autoCal_get);}

  HTTPClient http;

  String sendURL;
  sendURL = URL_SCD30_autoCal;
  http.begin(sendURL);
  http.GET();

  String scd30_autoCal = http.getString();
  
  http.end();

  if (debug) {Serial1.println("    scd30_autoCal = " + scd30_autoCal);}

  if (scd30_autoCal_get != scd30_autoCal) {
    if (scd30_autoCal == "true") {

      airSensor.setAutoSelfCalibration(true);
      if (debug) {Serial1.println("    Enable SCD30 AutoCalibration");}
    }
    else {
      airSensor.setAutoSelfCalibration(false);
      if (debug) {Serial1.println("    Disable SCD30 AutoCalibration");}
    }
    airSensor.reset();
  }
}

void SCD30_serial_output() {

  String output = "SCD30 -- ";
  output += "CO2=" + scd30_co2;
  output += ", temp=" + scd30_temp;
  output += ", humi=" + scd30_humi;

  Serial1.println(output);
}
#endif

#ifdef SPS30_active
//######################################### SPS30 functions ########################################################

void SPS30_setup() {

  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;

  sensirion_i2c_init();

   while (sps30_probe() != 0) {
    Serial1.println("SPS sensor probing failed");
    send_ErrorLog("Error: SPS30 sensor probing failed");
    delay(1000);
  }

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial1.println("error setting the auto-clean interval: " + String(ret));
    send_ErrorLog("Error: SPS30 setting the auto-clean interval failed");
  }

  #ifdef SPS30_LIMITED_I2C_BUFFER_SIZE
    Serial1.println("Your Arduino hardware has a limitation that only allows reading the mass concentrations.");
    Serial1.println("For more information, please check:");
    Serial1.println("https://github.com/Sensirion/arduino-sps#esp8266-partial-legacy-support");
    send_ErrorLog("Warning: SPS30 Your Arduino hardware has a limitation that only allows reading the mass concentrations.");
    delay(2000);
  #endif

  SPS30_activated = true;

}

void SPS30_start_fan() {

  int16_t ret;
  
  Serial1.println("starting measurement...");
  
  ret = sps30_start_measurement(); //start measurement
  if (ret < 0) {
    Serial1.println("error starting measurement");
    sps30_stop_measurement();
    send_ErrorLog("Error: SPS30 error starting measurement (fan)");
    reboot_on_error();
  }
}

void SPS30_get_data() {

  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
  int16_t ret;

  do {
    ret = sps30_read_data_ready(&data_ready);

    if (ret < 0) {
      Serial1.println("error reading data-ready flag: " + String(ret));
      sps30_stop_measurement();
      send_ErrorLog("Error: SPS30 error reading data-ready flag");
      break;
    } 
    else if (!data_ready)
    {
      Serial1.println("data not ready, no new measurement available");
      send_ErrorLog("Error: SPS30 data not ready, no new measurement available");
      break;
    } 
    else
    {
      break;
    }
     
    delay(100); /* retry in 100ms */
  } while (1);

  ret = sps30_read_measurement(&m);

  if (ret < 0) {
    Serial1.println("error reading measurement");
    sps30_stop_measurement();
    send_ErrorLog("Error: SPS30 error reading measurement");
    reboot_on_error();
  } 
  else {
    Serial1.println("stopping measurement...");
    ret = sps30_stop_measurement();
    if (ret < 0) {
      Serial1.println("error stopping measurement");
      send_ErrorLog("Error: SPS30 error stopping measurement");
      reboot_on_error();
    }
  }

  // convert 

  char m_char[7];
  String untrimmed;

  dtostrf(m.mc_1p0, 6, 2, m_char);
  mc_1p0 = String(m_char);
  mc_1p0.trim();
  
  dtostrf(m.mc_2p5, 6, 2, m_char);
  mc_2p5 = String(m_char);
  mc_2p5.trim();
  
  dtostrf(m.mc_4p0, 6, 2, m_char);
  mc_4p0 = String(m_char);
  mc_4p0.trim();
  
  dtostrf(m.mc_10p0, 6, 2,m_char); 
  mc_10p0 = String(m_char);
  mc_10p0.trim();

  dtostrf(m.nc_0p5, 6, 2, m_char);
  nc_0p5 = String(m_char);
  nc_0p5.trim();

  dtostrf(m.nc_1p0, 6, 2, m_char);
  nc_1p0 = String(m_char);
  nc_1p0.trim();

  dtostrf(m.nc_2p5, 6, 2, m_char);
  nc_2p5 = String(m_char);
  nc_2p5.trim();

  dtostrf(m.nc_4p0, 6, 2, m_char);
  nc_4p0 = String(m_char);
  nc_4p0.trim();

  dtostrf(m.nc_10p0, 6, 2, m_char);    
  nc_10p0 = String(m_char);
  nc_10p0.trim();

  dtostrf(m.typical_particle_size, 6, 2, m_char);
  typical_particle_size = String(m_char);
  typical_particle_size.trim();
}

void SPS30_zero_data() {

  mc_1p0 = "0";
  mc_2p5 = "0";
  mc_4p0 = "0";
  mc_10p0 = "0";
  nc_0p5 = "0";
  nc_1p0 = "0";
  nc_2p5 = "0";
  nc_4p0 = "0";
  nc_10p0 = "0";
  typical_particle_size = "0";
  
}

void SPS30_serial_output() {

String output;

  if (SPS30_sensor_active) {

    output = String("SPS30 -- "); 
    output += "mc_1p0 = " + mc_1p0;
    output += ", mc_2p5 = " + mc_2p5;
    output += ", mc_4p0 = " + mc_4p0;
    output += ", mc_10p0 = " + mc_10p0;
    output += ", nc_0p5 = " + nc_0p5;
    output += ", nc_1p0 = " + nc_1p0;
    output += ", nc_2p5 = " + nc_2p5;
    output += ", nc_4p0 = " + nc_4p0;
    output += ", nc_10p0 = " + nc_10p0;
    output += ", tps = " + typical_particle_size;    
  }
  else {output = String("SPS30 -- inactive");}
 
  Serial1.println(output);
}

void SPS30_control_heater() {

  if (debug) {Serial1.println("### SPS30_control_heater");}

  HTTPClient http;
  
  int humi_int = humi.toInt();
  String send_url;

  //------------------------------------ Test mode

  if (humi_test > 0) {
    humi_int = humi_test;
  }
  
  // -----------------------------------------

  if (debug) {
      Serial1.print("    Humidity/high/low: " + String(humi_int) + "/" + String(humi_high) + "/" + String(humi_low));
  }

  if (humi_int >= humi_high) {
    if (debug) {
      Serial1.print(" -- Activate heater");
      Serial1.println(" -- Switch GPIO to low: " + String(Relay_A));
    }
    digitalWrite(Relay_A, LOW);
    send_url = URL_heater + "true";
  }
  else if (humi_int < humi_high && humi_int > humi_low) {
    if (debug) {Serial1.println(" -- Do nothing");}
  }
  else if (humi_int <= humi_low) {
    if (debug) {
      Serial1.print(" -- Deactivate heater (low)");
      Serial1.println(" -- Switch GPIO to high: " + String(Relay_A));
    }
    digitalWrite(Relay_A, HIGH);
    send_url = URL_heater + "false";
  }
  else {
    if (debug) {
      Serial1.print(" -- Out of range - deactivate heater");
      Serial1.println(" -- Switch GPIO to high: " + String(Relay_A));
    }
    digitalWrite(Relay_A, HIGH);
    send_url = URL_heater + "false";
    send_ErrorLog("Warning: SPS30_control_heater out of range Humidity/high/low: " + String(humi_int) + "/" + String(humi_high) + "/" + String(humi_low));
  }
  http.begin(send_url);
  http.GET();
  http.end();  
}

void SPS30_get_heater_config() {

  HTTPClient http;

  http.begin(URL_humi_low);
  http.GET();
  humi_low = http.getString().toInt();  

  http.begin(URL_humi_high);
  http.GET();
  humi_high = http.getString().toInt();  

  http.begin(URL_humi_test);
  http.GET();
  humi_test = http.getString().toInt();  

  http.end();
}

void SPS30_get_dynamic_config() {

  if (debug) {Serial1.println("### SPS30_get_dynamic_config");}

  HTTPClient http;

  // Get SPS30_sensor_active

  http.begin(URL_SPS30_sensor_active);
  http.GET();
  if (http.getString() == "true") {
    SPS30_sensor_active = true;
  }
  else {
    SPS30_sensor_active = false;
  }

  if (debug) {
    Serial1.println("    New SPS30_sensor_active setting = " + bool_to_string(SPS30_sensor_active));  
  }
  http.end();
}

#endif

#ifdef WindSensor_active
//######################################### wind speed sensor section #######################################################

void WindSensor_get_config() {

  if (debug) {Serial1.println("### WindSensor_get_data");}

  String httpResult;
  
  HTTPClient http;

  http.begin(URL_A0_Step_Voltage);
  http.GET();

  httpResult = http.getString();
  WindSensor_A0_Step_Voltage =  httpResult.toDouble();
  if (debug) {Serial1.println("    A0_Step_Vmax = " + String(WindSensor_A0_Step_Voltage, 9));}
     
  http.end();
}

void WindSpeed_get_data() {

  if (debug) {Serial1.println("### WindSpeed_get_data");}

  int Analog_Input = analogRead(analog_Pin);
  double WindSensor_Speed = Analog_Input * WindSensor_A0_Step_Voltage * 3.6;

  int x = interval - counter;
  WindSpeedArray[x] = WindSensor_Speed;

  if (debug) {Serial1.println("    Analog_Input = " + String(Analog_Input));}
  if (debug) {Serial1.println("    WindSpeedArray[" + String(x) + "] = " + String(WindSpeedArray[x]));}
   
}

void WindDirection_setup() {

  pinMode(RTS, OUTPUT);
  digitalWrite(RTS, LOW);
//  RS485.begin(BAUD_RATE, SWSERIAL_8N1, RX, TX, false, 128, 128);
  
}

uint16_t Calc_CRC(byte buf[], int len) {

  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < (len - 2); pos++) {  
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

bool CRC_Check(byte buf[], int len) {

  uint16_t CRC;
  uint8_t CRC_MSB;
  uint8_t CRC_LSB;
  
  CRC = Calc_CRC(buf, len);
  CRC_MSB = CRC >> 8;
  CRC_LSB = CRC;

  if (buf[len -2] == CRC_LSB && buf[len -1] == CRC_MSB) {return true;}
  else {return false;}
}

void WindDirection_get_data() {

  if (debug) {Serial1.println("### WindDirection_get_data");}
  
  int readBufferSize;

// ..... send request to sensor
  
  if (debug) {Serial1.print("    Write: ");}
  
  digitalWrite(RTS, HIGH);     // init Transmit
  int write_length = Serial.write(request_buffer, request_buffer_length);

  double ByteTime = (10000000 / SERIAL_BAUD_RATE); // RTS HIGH time for one byte in µs
  
  delayMicroseconds((write_length * ByteTime));
  digitalWrite(RTS, LOW);

  if (debug) {
    Serial1.print(write_length);
    Serial1.println(" byte");
  }
   
// ..... read answer

  if (debug) {Serial1.print("    Read: ");}
  
  int x = 5;
  if (debug) {Serial1.print("    Wait for data ");} //max 500ms
  while (x > 0) {
  
      readBufferSize = Serial.available();
      if (readBufferSize >= expectedFrameSize) {break;}
      if (debug) {Serial1.print(x - 1);}
      delay(100);
      x--;
  }
  if (debug) {Serial1.println(" ");}  

// ..... proceed whe buffer is big enogh to expect a full frame

  if (readBufferSize >= expectedFrameSize) {

    byte readBuffer[readBufferSize];
    
    Serial.readBytes(readBuffer, readBufferSize);
    
    if (debug) {
      Serial1.println("RX Buffer Size: " + String(readBufferSize));
      Serial1.print("RX Buffer : ");
      for (int i = 0; i < readBufferSize; i++ ) {
        Serial1.print(hex_to_string(readBuffer[i]));
        Serial1.print(" ");
      }
      Serial1.println("end");
    }

// ..... trim leading zeros (errors) from frame

    while (readBuffer[0] == 0) {
      for (int i = 1; i < readBufferSize; i++) {
        readBuffer[i - 1] = readBuffer[i];
      }
      readBufferSize--;
    }

    if (debug) {
      Serial1.println("Trimmed frame size: " + String(readBufferSize));
      Serial1.print("Frame : ");
        for ( byte i = 0; i < readBufferSize; i++ ) {
          Serial1.print(hex_to_string(readBuffer[i]));
          Serial1.print(" ");
        }
        Serial1.println("end");
    }

// ..... proceed whe buffer is still big enogh to expect a full frame

    if (readBufferSize >= expectedFrameSize) {
      
// ..... CRC Check
  
      if (CRC_Check(readBuffer, expectedFrameSize)) {
  
        WindSensor_Direction = ((uint16_t)readBuffer[3] << 8) | readBuffer[4];
      }
      else {
        
        if (debug){Serial1.println("    RX CRC ERROR");}
        crcErrors++;
      }
    }

// ..... timeouts id frame to short
    
    else {
      rxTimeOuts++;
      if (debug){Serial1.println("    TIMEOUT");}
    }
  }
  else {
    rxTimeOuts++;
    if (debug){Serial1.println("    TIMEOUT");}
  }
  
  
// ..... write wind direction into arry
  
  int a = interval - counter;
  WindDirectionArray[a] = WindSensor_Direction;
  if (debug) {Serial1.println("    WindDirectionArray[" + String(a) + "] = " + String(WindDirectionArray[a]));}
}  

void WindSensor_serial_output() {

  int x = interval - counter;
  String output;

  output = "WindSensor -- "; 
  output += "WindSpeed = " + String(WindSpeedArray[x]);
  output += ", WindDirection = " + String(WindDirectionArray[x]);
 
  Serial1.println(output);
}
#endif
