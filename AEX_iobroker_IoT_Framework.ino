/*

  Shared functions for iobroker IoT Framework

  Version: V5 (release)
  Date: 2020-11-30

  Supported features / sensors:

  - Wifi/HTTP Client (no tls)
  - BME280
  - BME680
  - SCD30

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

#ifdef AEX_iobroker_IoT_Framework
//######################################### generic sensor functions #######################################################

void connect_wifi() {

  Serial.println("### connect_wifi");

  WiFi.begin(ssid, password);
  Serial.print("    Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  digitalWrite(LED, HIGH);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i > 20) {
      Serial.println("    Connection failed!");
      reboot_on_error();
    }
  }

  Serial.println('\n');
  Serial.println("    Connection established!");
  Serial.print("    IP address:\t");
  Serial.println(WiFi.localIP());
}

void get_wifi_state() {

  if (debug) {
    Serial.println("### get_wifi_state");
  }

  if (WiFi.status() == WL_CONNECTED) {

    String wifiRSSI = String(WiFi.RSSI());
    if (debug) {
      Serial.println("    Wifi RSSI: " +  wifiRSSI);
    }

    HTTPClient http;

    String sendURL = URL_RSSI + wifiRSSI;
    http.begin(sendURL);
    http.GET();

    http.end();
  }
  else {
    Serial.println("    Wifi not connected, try reconnect");
    connect_wifi();
  }
}

void reboot_on_error() {

  Serial.println('\n');
  Serial.println("Performing reboot in 30 seconds....");

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
    Serial.print(++i); Serial.print(' ');
  }

  HWReset();
}

void send_ErrorLog(String Error_Msg) {

  if (debug) {
    Serial.println("### send_ErrorLog");
  }

  HTTPClient http;

  String Send_URL = URL_ErrorLog + Error_Msg;
  Send_URL.replace(" ", "+");

  http.begin(Send_URL);
  http.GET();
  http.end();

  if (debug) {
    Serial.println("    Send ErrorLog: " + Error_Msg);
  }
}

void get_interval() {

  if (debug) {
    Serial.println("### get_interval");
  }

  HTTPClient http;

  //if (debug) {Serial.println("    Get Interval URL_INT = " + URL_INT);}
  http.begin(URL_INT);
  http.GET();

  interval = http.getString().toInt();
  if (debug) {
    Serial.println("    New Interval = " + String(interval));
  }

  http.end();
}

void get_dynamic_config() {

  if (debug) {
    Serial.println("### get_dynamic_config");
  }

  HTTPClient http;

  // get LED setting

  //if (debug) {Serial.println("   Get LED setting URL_LED = " + URL_LED);}
  http.begin(URL_LED);
  http.GET();
  if (http.getString() == "true") {
    led = true;
  }
  else {
    led = false;
  }

  if (debug) {
    Serial.println("    New LED setting = " + bool_to_string(led));
  }

  // get SensorActive

  //if (debug) {Serial.println("    Get LED SensorActive URL_sensor_active = " + URL_sensor_active);}
  http.begin(URL_sensor_active);
  http.GET();

  if (http.getString() == "true") {
    sensor_active = true;
  }
  else {
    sensor_active = false;
  }

  if (debug) {
    Serial.println("    New Sensor Active State = " + bool_to_string(sensor_active));
  }

  // get DevMode

  //if (debug) {Serial.println("    Get DevMode URL_DevMode = " + URL_DevMode);}
  http.begin(URL_DevMode);
  int httpcode = http.GET();

  if (httpcode != 200) {
    Serial.println("    Error getting DevMode -> switch to DevMode = true");
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
    Serial.println("    DevMode = " + bool_to_string(DevMode));
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
      Serial.println("    Error getting baseURL_DATA_GET");
    }

    http.begin(baseURL_DEVICE_GET + "baseURL_SET_DEV");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_SET = http.getString();
    }
    else {
      Serial.println("    Error getting baseURL_DATA_SET");
    }
  }
  else {

    http.begin(baseURL_DEVICE_GET + "baseURL_GET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_GET = http.getString();
    }
    else {
      Serial.println("    Error getting baseURL_DATA_GET");
    }

    http.begin(baseURL_DEVICE_GET + "baseURL_SET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) {
      baseURL_DATA_SET = http.getString();
    }
    else {
      Serial.println("    Error getting baseURL_DATA_SET");
    }
  }

  baseURL_DATA_GET.replace("\"", "");
  baseURL_DATA_SET.replace("\"", "");

  if (debug) {
    Serial.println("    baseURL_DATA_GET = " + baseURL_DATA_GET);
    Serial.println("    baseURL_DATA_SET = " + baseURL_DATA_SET);
  }
}

void send_ip() {

  if (debug) {
    Serial.println("### send_ip");
  }

  HTTPClient http;

  String sendURL = URL_IP + WiFi.localIP().toString();

  http.begin(sendURL);
  http.GET();
  http.end();

  if (debug) {
    Serial.println("    Local IP: " + WiFi.localIP().toString());
  }
}

void send_sid() {

  byte mac[6];

  if (debug) {
    Serial.println("### send_sid");
  }
  if (debug) {
    Serial.println("    SensorID = " + SensorID);
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
    Serial.println("    Wifi MAC: " +  WifimacAddress);
  }

  String sendURL = URL_MAC + WifimacAddress;
  http.begin(sendURL);
  http.GET();

  http.end();
}

void send_rst() {

  if (debug) {
    Serial.println("### send_rst");
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
#endif

#ifdef BME280_active
//######################################### BME280 functions ########################################################

void BME280_setup() {

  if (debug) {
    Serial.println("### BME280_setup");
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
    Serial.println("### BME280_get_data");
  }

  bme280_humi = String(bme.readHumidity());
  bme280_temp = String(bme.readTemperature());
  bme280_pressure = bme.readPressure() / 100.0;
  bme280_airp = String(bme280_pressure);
  bme280_alti = bme.readAltitude(pressure_sl);
}

void BME280_get_sealevel_pressure() {

  if (debug) {
    Serial.println("### BME280_get_sealevel_pressure");
  }

  HTTPClient http;

  http.begin(URL_PRESL);  // get pressure at sea level
  http.GET();
  http.end();

  pressure_sl = http.getString().toInt();

  if (debug) {
    Serial.print("    Pressure at sea level = ");
    Serial.print(pressure_sl);
    Serial.print("\n");
  }
}

void BME280_serial_output() {

  String output = "BME280 -- ";
  output += "bme280_temp=" + bme280_temp;
  output += ", bme280_humi=" + bme280_humi;
  output += ", bme280_airp=" + bme280_airp;
  //output += ", bme280_alti=" + bme280_alti;

  Serial.println(output);
}
#endif

#ifdef BME680_active
//######################################### BME680 functions (BOSCH BSEC LP example) ########################################################

void BME680_setup() {

  if (debug) {
    Serial.println("### BME680_setup");
  }

  String output;

  EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1); // 1st address for the length
  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "    BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
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
      Serial.println(output);
      send_ErrorLog("Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
      send_ErrorLog("Warning: BME680 " + output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "    BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      send_ErrorLog("Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      send_ErrorLog("Warning: BME680 " + output);
    }
  }
  iaqSensor.status = BSEC_OK;
}

void BME680_loadState() {

  if (debug) {
    Serial.println("### BME680_loadState");
  }

  if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in EEPROM
    Serial.println("    Reading state from EEPROM: ");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      bsecState[i] = EEPROM.read(i + 1);
      if (debug) {
        Serial.print(bsecState[i], HEX);
      }
    }
    if (debug) {
      Serial.print("\n");
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
    Serial.println("    Erasing EEPROM");

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
    Serial.println("### BME680_updateState");
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

    Serial.println("    Writing state to EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE ; i++) {
      EEPROM.write(i + 1, bsecState[i]);
      if (debug) {
        Serial.print(bsecState[i], HEX);
      }
    }
    if (debug) {
      Serial.print("\n");
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
    Serial.println("### BME680_get_data");
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
    Serial.println("### BME680_reset");
  }

  HTTPClient http;

  String sendURL;

  sendURL = URL_BME680_reset_get;
  http.begin(sendURL);
  http.GET();

  String result = http.getString();

  if (debug) {
    if (debug) {
      Serial.println("    Reset: " + result);
    }
  }

  if (result == "true") {

    // Erase the EEPROM with zeroes
    if (debug) {
      Serial.println("    Erasing EEPROM");
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

  Serial.println(output);
}
#endif

#ifdef SCD30_active
//######################################### SCD30 functions ########################################################

void SCD30_setup() {

  if (debug) {
    Serial.println("### SCD30_setup");
  }

  Wire.begin();

  if (airSensor.begin(Wire) == false) {

    if (debug) {
      Serial.println("    Air sensor not detected. Please check wiring");
    }
    send_ErrorLog("Error: SCD30 Air sensor not detected. Please check wiring");
    reboot_on_error();
  }
  else {
    if (debug) {
      Serial.println("     Air sensor found.");
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
    Serial.println("### SCD30_get_data");
  }

  // collect SCD30 data

  if (airSensor.dataAvailable())
  {
    scd30_co2 = String(airSensor.getCO2());
    scd30_temp = String(airSensor.getTemperature());
    scd30_humi = String(airSensor.getHumidity());
  }
  else {
    Serial.println("    SCD30 no data");
  }

  if (debug) {
    Serial.println("    Ambient pressure = " + String(bme280_pressure));
  }
  airSensor.setAmbientPressure(bme280_pressure); // update ambient pressure preset from BME280

}

void SCD30_AutoCal() {

  if (debug) {
    Serial.println("### SCD30_AutoCal");
  }

  HTTPClient http;

  String sendURL;
  String scd30_autoCal;

  sendURL = URL_SCD30_autoCal_get;
  http.begin(sendURL);
  http.GET();
  scd30_autoCal = http.getString();
  http.end();

  if (debug) {
    Serial.println("    scd30_autoCal = " + scd30_autoCal);
  }

  if (scd30_autoCalHistory != scd30_autoCal) {
    if (scd30_autoCal == "true") {

      airSensor.setAutoSelfCalibration(true);
      if (debug) {
        Serial.println("    Enable SCD30 AutoCalibration");
      }
    }
    else {
      airSensor.setAutoSelfCalibration(false);
      if (debug) {
        Serial.println("    Disable SCD30 AutoCalibration");
      }
    }

    airSensor.reset();
    scd30_autoCalHistory = scd30_autoCal;
  }
}

void SCD30_serial_output() {

  String output = "SCD30 -- ";
  output += "CO2=" + scd30_co2;
  output += ", temp=" + scd30_temp;
  output += ", humi=" + scd30_humi;

  Serial.println(output);
}
#endif

#ifdef WindSensor_active
//######################################### wind speed sensor section #######################################################

void WindSensor_get_config() {

  if (debug) {Serial.println("### WindSensor_get_data");}

  String httpResult;
  
  HTTPClient http;

  http.begin( URL_A0_Step_Vmax);
  http.GET();

  httpResult = http.getString();
  WindSensor_A0_Step_Vmax =  httpResult.toDouble();
  if (debug) {Serial.println("    A0_Step_Vmax = " + String(WindSensor_A0_Step_Vmax, 9));}
     
  http.end();
}

void WindSensor_get_Data() {

  int Analog_Input = analogRead(analog_Pin);
  WindSensor_Speed = Analog_Input * WindSensor_A0_Step_Vmax * 3.6;
}
#endif
