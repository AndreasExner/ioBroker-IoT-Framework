/*

  Shared functions for iobroker IoT Framework

  Version: V6.0.0
  Date: 2021-07-06

  Supported features / sensors:

  - Wifi
  - MQTT (NEW in 6.0)
  - BME280
  - BME680
  - SCD30
  - SPS30 WARNING the SPS30 section is not yet converted to MQTT! Please use the latest version of F5!
  - WindSensor WARNING the WindSensor section is not yet converted to MQTT! Please use the latest version of F5!
  - ePaper Displays (code for IndoorAirSensor)
  - ADS1115 (code for DaylightSensor)

  https://github.com/AndreasExner/ioBroker-IoT-Framework

  IMPORTANT:

  If your sketch use wind direction sensor (RS485), you HAVE to change the 
  serial output for debug and runtime information (Serial.print) to serial1 ! 

  
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

void wifi_connect() {

  #include "WiFi_secret.h"
  
  Serial.println("### connect_wifi");

  WiFi.begin(ssid, password);
  Serial.print("    connecting to ");
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

void wifi_getState() {

  if (debug) {
    Serial.println("### get_wifi_state");
  }

  if (WiFi.status() == WL_CONNECTED) {

    String wifiRSSI = String(WiFi.RSSI());
    if (debug) {
      Serial.println("    Wifi RSSI: " +  wifiRSSI);
    }
  }
  else {
    Serial.println("    Wifi not connected, try reconnect");
    wifi_connect();
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


//######################################### MQTT functions ########################################################

void mqtt_connect() {
  
  #include "MQTT_secret.h"

  Serial.println("### MQTT_connect");
  Serial.println("    connecting to broker: " + String(MQTT_broker) + ":" + String(MQTT_port));

  mqttClient.setUsernamePassword(MQTT_user, MQTT_pass);

  int i = 0;
  while (!mqttClient.connect(MQTT_broker, MQTT_port)) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i > 20) {
      Serial.println("    Connection failed!");
      Serial.print("      MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      reboot_on_error();
    }
  }

  Serial.println('\n');
  Serial.println("    Connection established!");
 
}

void mqtt_getState() {

  if (debug) {Serial.println("### MQTT_getState");}

  if (mqttClient.connected() == 0) {
  
    if (debug) {Serial.println("    MQTT NOT connected");}
    wifi_getState();
    mqtt_connect();
  }  
  else {if (debug) {Serial.println("    MQTT connected");}}
}

void mqtt_send(String topic, String message) {

  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();

  if (debug) {
    Serial.print("    topic: " + topic);
    Serial.println(", message: " + message);
  }
}

void mqtt_subscribeDeviceConfig() {

  String subscribeConfig;

  if (debug) {Serial.println("### MQTT_subscribeDeviceConfig");}

  subscribeConfig = MQTT_topicDevice + "Config/#";
  mqttClient.subscribe(subscribeConfig);
  if (debug) {Serial.println("    subscribe topic = " + subscribeConfig);}

  delay(2000); // give MQTT time to subscribe/receive. 2000ms are a good point to start.
}

void mqtt_getDeviceConfig() {

  if (debug) {Serial.println("### MQTT_getDeviceConfig");}

  String mqttTopic, mqttMessage;
  int mqttMessageSize = mqttClient.parseMessage();
  int intervalHistory = interval;
 
  while (mqttMessageSize) {

    mqttTopic = mqttClient.messageTopic();
    mqttMessage = "";

    while (mqttClient.available()) {
    
      mqttMessage += char(mqttClient.read());
    }

    if (debug) {Serial.println("    topic: " + mqttTopic + " - message: " + mqttMessage);}

    if (mqttTopic == MQTT_topicDevice + "Config/LED" && mqttMessage == "true") {ledActive = true;}
    else if (mqttTopic == MQTT_topicDevice + "Config/LED" && mqttMessage == "false") {ledActive = false;}
    else if (mqttTopic == MQTT_topicDevice + "Config/Debug" && mqttMessage == "true") {debug = true;}
    else if (mqttTopic == MQTT_topicDevice + "Config/Debug" && mqttMessage == "false") {debug = false;}
    else if (mqttTopic == MQTT_topicDevice + "Config/DevMode" && mqttMessage == "true") {devMode = true;}
    else if (mqttTopic == MQTT_topicDevice + "Config/DevMode" && mqttMessage == "false") {devMode = false;}
    else if (mqttTopic == MQTT_topicDevice + "Config/DeviceActive" && mqttMessage == "true") {deviceActive = true;}
    else if (mqttTopic == MQTT_topicDevice + "Config/DeviceActive" && mqttMessage == "false") {deviceActive = false;}
    else if (mqttTopic == MQTT_topicDevice + "Config/Interval") {interval = mqttMessage.toInt();}
    else if (mqttTopic == MQTT_topicDevice + "Config/Delay") {intervalDelay = mqttMessage.toInt();}
    else {mqtt_parseSpecificConfig(mqttTopic, mqttMessage);} // handover sensor specific config
    
    mqttMessageSize = mqttClient.parseMessage();
  }

  if (debug) {Serial.println("    Interval: " + String(interval) + " - Delay: " + String(intervalDelay) + " - Debug: " + bool_to_string(debug) + " - LED: " + bool_to_string(ledActive) + " - DEV: " + bool_to_string(devMode) + " - deviceActive: " + bool_to_string(deviceActive));}

  // switch to prod mode if configured in iobroker. MUST be executed after the first run of mqtt_getSensorConfig!
  if (devMode) {MQTT_topicData = MQTT_topicDataDEV;}
  else {MQTT_topicData = MQTT_topicDataPROD;}

  if (interval != intervalHistory) {counter = 0;}

}

void mqtt_sendDeviceState() {

  if (debug) {Serial.println("### MQTT_sendDeviceState");}

  byte mac[6];
  WiFi.macAddress(mac);
  String WifimacAddress = String(mac[5], HEX) + ":";
  WifimacAddress += String(mac[4], HEX) + ":";
  WifimacAddress += String(mac[3], HEX) + ":";
  WifimacAddress += String(mac[2], HEX) + ":";
  WifimacAddress += String(mac[1], HEX) + ":";
  WifimacAddress += String(mac[0], HEX);

  String deviceIP = WiFi.localIP().toString();
  String WifiRSSI = String(WiFi.RSSI());

  if (debug) {
    Serial.println("    deviceID = " + deviceID);
    Serial.println("    deviceName = " + deviceName);
    Serial.println("    deviceIP = " + deviceIP);
    Serial.println("    Wifi MAC: " +  WifimacAddress);
    Serial.println("    Wifi RSSI: " +  WifiRSSI);
  }

  mqtt_send(MQTT_topicDevice + "DeviceName", deviceName);
  mqtt_send(MQTT_topicDevice + "DeviceIP", deviceIP);
  mqtt_send(MQTT_topicDevice + "MAC", WifimacAddress);
  mqtt_send(MQTT_topicDevice + "RSSI", WifiRSSI);
  mqtt_send(MQTT_topicData + "DeviceID", deviceID);
}

#endif


#ifdef ePaper_active
//######################################### ePaper functions ########################################################

// code useable for indoor air sensor only!

void ePaper_setup() {

  if (debug) {Serial.println("### ePaper_setup");}
  
  display.init();
  display.eraseDisplay();
  ePaper_showData_1_54_3fields("Temperatur:", "Rel. Luftfeuchte:", "CO2 Gehalt:", "--.-- °C", "--.-- %", "---- ppm", "Stand: ----------");
  ePaperDisplayActivated = true;

}

void ePaper_showData_1_54_3fields(String field1_name, String field2_name, String field3_name, String field1_value, String field2_value, String field3_value, String ts_update) {
  
  if (debug) {Serial.println("### ePaper_showData");}

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

  if (debug) {Serial.println("### BME280_setup");}

  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16,  // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X16,  // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_125 );
  bme280Activated = true;

}

void BME280_getData() {

  String bme280Temp, bme280Humi, bme280Airp, bme280Alti;
  
  if (debug) {Serial.println("### BME280_get_data");}

  bme280Humi = String(bme.readHumidity());
  bme280Temp = String(bme.readTemperature());
  bme280Pressure = bme.readPressure() / 100.0;
  bme280Airp = String(bme280Pressure);
  bme280Alti = bme.readAltitude(bme280PressureSeaLevel);

  if (debug) {BME280_serialOutput(bme280Temp, bme280Humi, bme280Airp, bme280Alti);}
  if (counter <=0) {
    BME280_sendData(bme280Temp, bme280Humi, bme280Airp, bme280Alti);
    
    ePaperTemp = bme280Temp;
    ePaperHumi = bme280Humi;
  }
}

void BME280_serialOutput(String bme280Temp, String bme280Humi, String bme280Airp, String bme280Alti) {

  String output = "    BME280 -- ";
  output += "bme280Temp=" + bme280Temp;
  output += ", bme280Tumi=" + bme280Humi;
  output += ", bme280Airp=" + bme280Airp;
  output += ", bme280Alti=" + bme280Alti;

  Serial.println(output);
}

void BME280_sendData(String bme280Temp, String bme280Humi, String bme280Airp, String bme280Alti) {

  if (debug) {Serial.println("    Send Data");}

  mqtt_send(MQTT_topicData + "Temperature", bme280Temp);
  mqtt_send(MQTT_topicData + "Humidity", bme280Humi);    
  mqtt_send(MQTT_topicData + "Airpressure", bme280Airp);
  mqtt_send(MQTT_topicData + "Altitude", bme280Alti);        
  
}

#endif


#ifdef BME680_active
//######################################### BME680 functions (BOSCH BSEC LP example) ########################################################

void BME680_setup() {

  if (debug) {Serial.println("### BME680_setup");}

  String output;

  EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1); // 1st address for the length
  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "    BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  if (debug) {Serial.println(output);}
  
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

  bme680Activated = true;
}

void BME680_checkIaqSensorStatus() {

  String output;

  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "    BSEC error code : " + String(iaqSensor.status);
      if (debug) {Serial.println(output);}
      mqtt_send(MQTT_topicDevice + "ErrorLog", "Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
      mqtt_send(MQTT_topicDevice + "ErrorLog", "Warning: BME680 " + output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "    BME680 error code : " + String(iaqSensor.bme680Status);
      if (debug) {Serial.println(output);}
      mqtt_send(MQTT_topicDevice + "ErrorLog", "Error: BME680 " + output);
      reboot_on_error();
    } else {
      output = "    BME680 warning code : " + String(iaqSensor.bme680Status);
      if (debug) {Serial.println(output);}
      mqtt_send(MQTT_topicDevice + "ErrorLog", "Warning: BME680 " + output);
    }
  }
  iaqSensor.status = BSEC_OK;
}

void BME680_loadState() {

  if (debug) {Serial.println("### BME680_loadState");}

  if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in EEPROM
    Serial.println("    Reading state from EEPROM: ");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      bsecState[i] = EEPROM.read(i + 1);
      if (debug) {
        Serial.print(bsecState[i], HEX);
      }
    }
    if (debug) {Serial.print("\n");}

    iaqSensor.setState(bsecState);
    BME680_checkIaqSensorStatus();

    mqtt_send(MQTT_topicData + "BME680_loadState", "true"); //send load state to iobroker

  } else {
    // Erase the EEPROM with zeroes
    if (debug) {Serial.println("    Erasing EEPROM");}

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
      EEPROM.write(i, 0);

    EEPROM.commit();

    mqtt_send(MQTT_topicData + "BME680_eraseEEPROM", "true"); //send erase EEPROM to iobroker
    
  }
}

void BME680_updateState() {

  if (debug) {Serial.println("### BME680_updateState");}

  bool update = false;
  
  // Set a trigger to save the state. Here, the state is saved every STATE_SAVE_PERIOD with the first state being saved once the algorithm achieves full calibration, i.e. iaqAccuracy = 3
  
  if (bsecStateUpdateCounter == 0) {
    if (iaqSensor.iaqAccuracy >= 3) {
      update = true;
      bsecStateUpdateCounter++;
    }
  } else {

    // Update every STATE_SAVE_PERIOD milliseconds
    
    if ((bsecStateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
      update = true;
      bsecStateUpdateCounter++;
    }
  }

  if (update) {
    iaqSensor.getState(bsecState);
    BME680_checkIaqSensorStatus();

    if (debug) {Serial.println("    Writing state to EEPROM");}

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE ; i++) {
      EEPROM.write(i + 1, bsecState[i]);
      if (debug) { Serial.print(bsecState[i], HEX);}
    }
    if (debug) {Serial.print("\n");}

    EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
    EEPROM.commit();

    mqtt_send(MQTT_topicData + "BME680_updateState", "true"); //send update state to iobroker
  
  }
}

void BME680_getData() {

  String bme680iaqD, bme680iaDA, bme680iaqS, bme680iaSA, bme680VOCe, bme680Temp, bme680Humi;

  if (debug) {Serial.println("### BME680_get_data");}

  bme680iaqD = String(iaqSensor.iaq);
  bme680iaDA = String(iaqSensor.iaqAccuracy);
  bme680iaqS = String(iaqSensor.staticIaq);
  bme680iaSA = String(iaqSensor.staticIaqAccuracy);
  bme680VOCe = String(iaqSensor.breathVocEquivalent);
  bme680Temp = String(iaqSensor.temperature);
  bme680Humi = String(iaqSensor.humidity);

  // if accuracy changed send data ASAP to iobroker (update accuracy value)

  if (iaqSensor.staticIaqAccuracy != bme680iaSAhistory) {
    counter = 0;
    bme680iaSAhistory = iaqSensor.staticIaqAccuracy;
  }

  if (debug) {BME680_serialOutput(bme680iaqD, bme680iaDA, bme680iaqS, bme680iaSA, bme680VOCe, bme680Temp, bme680Humi);}
  if (counter <=0) {
    BME680_sendData(bme680iaqD, bme680iaDA, bme680iaqS, bme680iaSA, bme680VOCe, bme680Temp, bme680Humi);
    BME680_updateState();  
  }
  
}

void BME680_reset() {

  if (debug) {Serial.println("### BME680_reset");}

  if (debug) {Serial.println("    Erasing EEPROM");}

  for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++) {EEPROM.write(i, 0);}

  EEPROM.commit();

  mqtt_send(MQTT_topicData + "BME680_reset", "false"); // update reset flag in iobroker
  mqtt_send(MQTT_topicData + "BME680_eraseEEPROM", "true"); // send earase EEPROM to iobroker

}

void BME680_serialOutput(String bme680iaqD, String bme680iaDA, String bme680iaqS, String bme680iaSA, String bme680VOCe, String bme680Temp, String bme680Humi) {

  String output = "    BME680 -- ";
  output += "iaqD=" + bme680iaqD;
  output += ", iaDA=" + bme680iaDA;
  output += ", iaqS=" + bme680iaqS;
  output += ", iaSA=" + bme680iaSA;
  output += ", VOCe=" + bme680VOCe;
  output += ", Temp=" + bme680Temp;
  output += ", Humi=" + bme680Humi;

  Serial.println(output);
}

void BME680_sendData(String bme680iaqD, String bme680iaDA, String bme680iaqS, String bme680iaSA, String bme680VOCe, String bme680Temp, String bme680Humi) {

  if (debug) {Serial.println("    Send Data");}

  mqtt_send(MQTT_topicData + "iaDA", bme680iaDA);
  mqtt_send(MQTT_topicData + "iaSA", bme680iaSA);
  mqtt_send(MQTT_topicData + "iaqD", bme680iaqD);
  mqtt_send(MQTT_topicData + "iaqS", bme680iaqS);
  mqtt_send(MQTT_topicData + "VOCe", bme680VOCe);

}

#endif


#ifdef SCD30_active
//######################################### SCD30 functions ########################################################

void SCD30_setup() {

  if (debug) {Serial.println("### SCD30_setup");}

  Wire.begin();

  if (airSensor.begin(Wire) == false) {

    if (debug) {Serial.println("    Air sensor not detected. Please check wiring");}
    
    mqtt_send(MQTT_topicDevice + "ErrorLog", "Error: SCD30 Air sensor not detected. Please check wiring");
    reboot_on_error();
  }
  else {
    if (debug) {Serial.println("    Air sensor found.");}
  }

  airSensor.setMeasurementInterval(scd30Interval);
  //airSensor.setAltitudeCompensation(scd30_altitude);
  airSensor.setAmbientPressure(1013);
  airSensor.setTemperatureOffset(scd30Offset);

  scd30Activated = true;
}

void SCD30_getData() {

  String scd30Co2, scd30Humi, scd30Temp;
  
  if (debug) {Serial.println("### SCD30_getData");}

  // collect SCD30 data

  if (airSensor.dataAvailable())
  {
    scd30Co2 = String(airSensor.getCO2());
    scd30Temp = String(airSensor.getTemperature());
    scd30Humi = String(airSensor.getHumidity());
    ePaperCO2 = scd30Co2;

    if (debug) {SCD30_serialOutput(scd30Co2, scd30Humi, scd30Temp);}
    
    if (counter <=0) {
    
      SCD30_SendData(scd30Co2);
      airSensor.setAmbientPressure(bme280Pressure); // update ambient pressure preset from BME280
      if (debug) {Serial.println("    Ambient pressure = " + String(bme280Pressure));}
    }
  }
  else {
    if (debug) {Serial.println("    SCD30 no data");}
  }

}

void SCD30_AutoCal() {

  if (debug) {Serial.println("### SCD30_AutoCal");}

  bool scd30_autoCal_get = airSensor.getAutoSelfCalibration();
  
  if (debug) {
    Serial.println("    scd30_autoCal_get = " + bool_to_string(scd30_autoCal_get));
    Serial.println("    scd30AutoCal = " + bool_to_string(scd30AutoCal));
  
  }

  if (scd30_autoCal_get != scd30AutoCal) {
    if (scd30AutoCal) {

      airSensor.setAutoSelfCalibration(true);
      if (debug) {Serial.println("    Enable SCD30 AutoCalibration");}
    }
    else {
      airSensor.setAutoSelfCalibration(false);
      if (debug) {Serial.println("    Disable SCD30 AutoCalibration");}
    }
    airSensor.reset();
  }
}

void SCD30_serialOutput(String scd30Co2, String scd30Humi, String scd30Temp) {

  String output = "    SCD30 -- ";
  output += "CO2=" + scd30Co2;
  output += ", temp=" + scd30Temp;
  output += ", humi=" + scd30Humi;

  Serial.println(output);
}

void SCD30_SendData(String scd30Co2) {

  if (debug) {Serial.println("    Send Data");}

  mqtt_send(MQTT_topicData + "co2", scd30Co2);
}

#endif


#ifdef SPS30_active
//######################################### SPS30 functions ########################################################

// WARNING the SPS30 section is not yet converted to MQTT! Please use the latest version of F5!

void SPS30_setup() {

  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;

  sensirion_i2c_init();

   while (sps30_probe() != 0) {
    Serial.println("SPS sensor probing failed");
    send_ErrorLog("Error: SPS30 sensor probing failed");
    delay(1000);
  }

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.println("error setting the auto-clean interval: " + String(ret));
    send_ErrorLog("Error: SPS30 setting the auto-clean interval failed");
  }

  #ifdef SPS30_LIMITED_I2C_BUFFER_SIZE
    Serial.println("Your Arduino hardware has a limitation that only allows reading the mass concentrations.");
    Serial.println("For more information, please check:");
    Serial.println("https://github.com/Sensirion/arduino-sps#esp8266-partial-legacy-support");
    send_ErrorLog("Warning: SPS30 Your Arduino hardware has a limitation that only allows reading the mass concentrations.");
    delay(2000);
  #endif

  SPS30_activated = true;

}

void SPS30_start_fan() {

  int16_t ret;
  
  Serial.println("starting measurement...");
  
  ret = sps30_start_measurement(); //start measurement
  if (ret < 0) {
    Serial.println("error starting measurement");
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
      Serial.println("error reading data-ready flag: " + String(ret));
      sps30_stop_measurement();
      send_ErrorLog("Error: SPS30 error reading data-ready flag");
      break;
    } 
    else if (!data_ready)
    {
      Serial.println("data not ready, no new measurement available");
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
    Serial.println("error reading measurement");
    sps30_stop_measurement();
    send_ErrorLog("Error: SPS30 error reading measurement");
    reboot_on_error();
  } 
  else {
    Serial.println("stopping measurement...");
    ret = sps30_stop_measurement();
    if (ret < 0) {
      Serial.println("error stopping measurement");
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
 
  Serial.println(output);
}

void SPS30_control_heater() {

  if (debug) {Serial.println("### SPS30_control_heater");}

  HTTPClient http;
  
  int humi_int = humi.toInt();
  String send_url;

  //------------------------------------ Test mode

  if (humi_test > 0) {
    humi_int = humi_test;
  }
  
  // -----------------------------------------

  if (debug) {
      Serial.print("    Humidity/high/low: " + String(humi_int) + "/" + String(humi_high) + "/" + String(humi_low));
  }

  if (humi_int >= humi_high) {
    if (debug) {
      Serial.print(" -- Activate heater");
      Serial.println(" -- Switch GPIO to low: " + String(Relay_A));
    }
    digitalWrite(Relay_A, LOW);
    send_url = URL_heater + "true";
  }
  else if (humi_int < humi_high && humi_int > humi_low) {
    if (debug) {Serial.println(" -- Do nothing");}
  }
  else if (humi_int <= humi_low) {
    if (debug) {
      Serial.print(" -- Deactivate heater (low)");
      Serial.println(" -- Switch GPIO to high: " + String(Relay_A));
    }
    digitalWrite(Relay_A, HIGH);
    send_url = URL_heater + "false";
  }
  else {
    if (debug) {
      Serial.print(" -- Out of range - deactivate heater");
      Serial.println(" -- Switch GPIO to high: " + String(Relay_A));
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

  if (debug) {Serial.println("### SPS30_get_dynamic_config");}

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
    Serial.println("    New SPS30_sensor_active setting = " + bool_to_string(SPS30_sensor_active));  
  }
  http.end();
}

#endif


#ifdef ADS1115_active
//######################################### ADS1115 Section #######################################################

void ADS1115_setup() {

  if (debug) {Serial.println("    ADS1115_setup");}

  ads1115.begin();  // Initialize ads1115
  ads1115.setGain(GAIN_ONE); 

  ads1115Activated = true;
 
}

#endif


#ifdef ADS1115_daylightSensorActive
//----------------------------------------- ADS1115 DaylightSensor SubSection -------------------------------------------------------

void ADS1115_daylightSensorSetup() {

  if (debug) {Serial.println("### ADS1115_daylightSensorSetup");}
  
  if (!ads1115Activated) {ADS1115_setup();}
  ads1115DaylightSensorActivated = true;
  
}

void ADS1115_daylightSensorGetData() {

  if (debug) {Serial.println("### ADS1115_daylightSensorGetData");}

  int16_t ads1115Daylight01 = ads1115.readADC_SingleEnded(0);
  int16_t ads1115Daylight02 = ads1115.readADC_SingleEnded(1);

  if (debug) {ADS1115_daylightSensorSerialOutput(ads1115Daylight01, ads1115Daylight02);}
  if (counter <=0) {ADS1115_daylightSensorSendData(ads1115Daylight01, ads1115Daylight02);}
}

void ADS1115_daylightSensorSerialOutput(int16_t ads1115Daylight01, int16_t ads1115Daylight02) {

  if (debug) {
    
    String output = "    ads1115Daylight01 = ";
    output += String(ads1115Daylight01);
    output += " -- ads1115Daylight02 = ";
    output += String(ads1115Daylight02);
    
    Serial.println(output);
  }
}

void ADS1115_daylightSensorSendData(int16_t ads1115Daylight01, int16_t ads1115Daylight02) {

  if (debug) {Serial.println("    Send Data");}

  mqtt_send(MQTT_topicData + "DayLight01", String(ads1115Daylight01));
  mqtt_send(MQTT_topicData + "DayLight02", String(ads1115Daylight02));    

}

#endif


#ifdef WindSensor_active
//######################################### wind speed sensor section #######################################################

void WindSensor_get_config() {

  if (debug) {Serial.println("### WindSensor_get_data");}

  String httpResult;
  
  HTTPClient http;

  http.begin(URL_A0_Step_Voltage);
  http.GET();

  httpResult = http.getString();
  WindSensor_A0_Step_Voltage =  httpResult.toDouble();
  if (debug) {Serial.println("    A0_Step_Vmax = " + String(WindSensor_A0_Step_Voltage, 9));}
     
  http.end();
}

void WindSpeed_get_data() {

  if (debug) {Serial.println("### WindSpeed_get_data");}

  int Analog_Input = analogRead(analog_Pin);
  double WindSensor_Speed = Analog_Input * WindSensor_A0_Step_Voltage * 3.6;

  int x = interval - counter;
  WindSpeedArray[x] = WindSensor_Speed;

  if (debug) {Serial.println("    Analog_Input = " + String(Analog_Input));}
  if (debug) {Serial.println("    WindSpeedArray[" + String(x) + "] = " + String(WindSpeedArray[x]));}
   
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

  if (debug) {Serial.println("### WindDirection_get_data");}
  
  int readBufferSize;

// ..... send request to sensor
  
  if (debug) {Serial.print("    Write: ");}
  
  digitalWrite(RTS, HIGH);     // init Transmit
  int write_length = Serial.write(request_buffer, request_buffer_length);

  double ByteTime = (10000000 / SERIAL_BAUD_RATE); // RTS HIGH time for one byte in µs
  
  delayMicroseconds((write_length * ByteTime));
  digitalWrite(RTS, LOW);

  if (debug) {
    Serial.print(write_length);
    Serial.println(" byte");
  }
   
// ..... read answer

  if (debug) {Serial.print("    Read: ");}
  
  int x = 5;
  if (debug) {Serial.print("    Wait for data ");} //max 500ms
  while (x > 0) {
  
      readBufferSize = Serial.available();
      if (readBufferSize >= expectedFrameSize) {break;}
      if (debug) {Serial.print(x - 1);}
      delay(100);
      x--;
  }
  if (debug) {Serial.println(" ");}  

// ..... proceed whe buffer is big enogh to expect a full frame

  if (readBufferSize >= expectedFrameSize) {

    byte readBuffer[readBufferSize];
    
    Serial.readBytes(readBuffer, readBufferSize);
    
    if (debug) {
      Serial.println("RX Buffer Size: " + String(readBufferSize));
      Serial.print("RX Buffer : ");
      for (int i = 0; i < readBufferSize; i++ ) {
        Serial.print(hex_to_string(readBuffer[i]));
        Serial.print(" ");
      }
      Serial.println("end");
    }

// ..... trim leading zeros (errors) from frame

    while (readBuffer[0] == 0) {
      for (int i = 1; i < readBufferSize; i++) {
        readBuffer[i - 1] = readBuffer[i];
      }
      readBufferSize--;
    }

    if (debug) {
      Serial.println("Trimmed frame size: " + String(readBufferSize));
      Serial.print("Frame : ");
        for ( byte i = 0; i < readBufferSize; i++ ) {
          Serial.print(hex_to_string(readBuffer[i]));
          Serial.print(" ");
        }
        Serial.println("end");
    }

// ..... proceed whe buffer is still big enogh to expect a full frame

    if (readBufferSize >= expectedFrameSize) {
      
// ..... CRC Check
  
      if (CRC_Check(readBuffer, expectedFrameSize)) {
  
        WindSensor_Direction = ((uint16_t)readBuffer[3] << 8) | readBuffer[4];
      }
      else {
        
        if (debug){Serial.println("    RX CRC ERROR");}
        crcErrors++;
      }
    }

// ..... timeouts id frame to short
    
    else {
      rxTimeOuts++;
      if (debug){Serial.println("    TIMEOUT");}
    }
  }
  else {
    rxTimeOuts++;
    if (debug){Serial.println("    TIMEOUT");}
  }
  
  
// ..... write wind direction into arry
  
  int a = interval - counter;
  WindDirectionArray[a] = WindSensor_Direction;
  if (debug) {Serial.println("    WindDirectionArray[" + String(a) + "] = " + String(WindDirectionArray[a]));}
}  

void WindSensor_serial_output() {

  int x = interval - counter;
  String output;

  output = "WindSensor -- "; 
  output += "WindSpeed = " + String(WindSpeedArray[x]);
  output += ", WindDirection = " + String(WindDirectionArray[x]);
 
  Serial.println(output);
}
#endif
