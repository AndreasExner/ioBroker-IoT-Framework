# ioBroker-IoT-Framework

**ioBroker IoT Framework (based on NodeMCU / ESP8266)**

**Current version: 6.0.0 (F6)**



Working with IoT sensors and actors in ioBroker often needs a lot of development and testing. Every improvement needs a staging environment as well to keep the productive data untouched. I’ve started to setup a more or less universal dev environment for IoT devices like NodeMCU or Arduino. It enables a convenient and easy way to switch between dev and prod without uploading a new image to the device. 

With the **3rd generation** of the Framework I stared to include modules for hardware sensors like the BME280 to make them reusable in further projects.

With the **5th generation**, the Framework was completely restructured because the long code became more and more unhandy. All generic and hardware specific sensors functions are moved the extension file AEX_iobroker_IoT_Framework.ino. 

With the **6th generation**, I've started to replace HTTP-Rest with MQTT. **F5 and F6 are no longer compatible!** 

**Important:**

Be sure to have this file in the same folder like the primary .ino file. Opening the primary .ino file in Arduino editor should loaded  the extension automatically.



#### **Released projects**

- [Highly precise indoor air sensor (HTP, CO2 and IAQ)](https://github.com/AndreasExner/ioBroker-IoT-IndoorAirSensor)
- [Outdoor partical matter sensor](https://github.com/AndreasExner/ioBroker-IoT-PM_AQI-Sensor) (to be documented)
- Highly precise weather monitoring (HTP and rain) (to be documented)
- [Wind speed and direction monitoring](https://github.com/AndreasExner/iobroker-IoT-WindSensor)
- Water tank monitoring (to be revised in spring CY21)
- Garden sprinkler valve and soil moisture monitoring (to be revised in spring CY21)

#### Auxiliary projects

- [RS485-UART-EchoTest](https://github.com/AndreasExner/RS485-UART-EchoTest)



## Important: Serial output

 If your sketch use wind direction sensor (or RS485 in general), you **have** to change the  serial output for debug and runtime information (Serial.print and Serial.println) to **serial1**. 



## History

**6.0.0: Initial Release for MQTT**

- HTTP-Rest replaced with MQTT
- general code improvements
- added ADS1115 (16 Bit 4 channel ADC)
- Not all sensors are moved to F6. These sensors are supported (please use F5 for the others at the moment):
  - BME280
  - BME680
  - SCD30
  - ADS1115
  - ePaper (for indoor air sensor only)

**5.3.2: UART Update**

- Major improvements and bugfixes for the UART communication: CRC check, remove zero bytes etc.
- Softwareserial replaced with hardware serial
- changed wind speed data transfer (for de-noising)
- minor fixes and improvements

**5.2.0: sensor update**

- added SPS30
- added Wind speed and direction sensor (including software serial)
- minor bugfixes and improvements

**5.0.0: initial V5 release**



## Supported sensors (F5/F6)

- BME280 (based on the [Adafruit_BME280_Library version 2.1.2 ](https://github.com/adafruit/Adafruit_BME280_Library))
- BME680 (based on the [BoschSensortec Arduino library for BSEC 1.6.1480](https://github.com/BoschSensortec/BSEC-Arduino-library))
- SCD30 (based on the [SparkFun SCD30 Arduino library release 9](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library))
- SPS30 (based on the [Sensirion embedded SPS library 3.1.0](https://github.com/Sensirion/embedded-sps))
- ADS1115 (based on the [Adafruit ADS1X15 library 1.1.1](https://github.com/adafruit/Adafruit_ADS1X15))
- WindSensor

**Important:**

The Bosch BSEC library uses precompiled libraries. See the appendix for some changes to the ESP8266 plattform.txt file.



## Tested environment

- Software
  - Arduino IDE 1.8.13 (Windows)
  - ESP8266 hardware package 2.7.4
  - Arduino MQTT Client 0.1.5
  - Adafruit_BME280_Library version 2.1.2
  - BSEC-Arduino-library 1.6.1480
  - SparkFun_SCD30_Arduino_Library release 9
  - Sensirion embedded SPS library 3.1.0
  - ESPSoftwareSerial library 6.10.0
  - Adafruit ADS1X15 library 1.1.1
- Hardware
  - NodeMCU Lolin V3 (ESP8266MOD 12-F)
  - NodeMCU D1 Mini (ESP8266MOD 12-F)
  - GY-BME280
  - CJMCU-680 BME680
  - Sensirion SCD30
  - Sensirion SPS30
  - 5 V MAX485 / RS485 Modul TTL to RS-485 MCU
  - FT232 UART to USB Adapter
  - Adafruit ADS1115



## Prerequisites

* You need a running Arduino IDE and at least basic knowledge how it works. 
* You also need a running ioBroker instance and a little more than basic knowledge on that stuff.
* You need the ioBroker MQTT broker adapter up and running
* You need a userdata section in your ioBroker objects
* You should know how to work with IoT devices and electronics



## Setup

- Create a folder in your Arduino library folder
- Copy the primary sketch (e.g. DEV_6.0.ino) and the extension file (AEX_iobroker_IoT_Framework.ino) into the folder
  - be sure to include the secret files (MQTT_secret.h and WiFi_secret.h) as well
- Open the primary sketch (e.g. DEV_6.0.ino) 
  - edit code  as needed and don't forget to change the secrets as well
- Optional: install required libraries into your Arduino IDE
- Create (import) the datapoints in iobroker
- Set values for datapoints (see iobroker datapoints)



## Configuration

#### Enable / disable sensor specific functions

```c++
//+++++++++++++++++++++++++++++++++++++++++ enable sensor specific functions +++++++++++++++++++++++++++++++++++++++++++++++++

#define AEX_iobroker_IoT_Framework //generic functions DO NOT CHANGE

// uncomment required sections

//#define BME280_active
//#define BME680_active
//#define SCD30_active
//#define WindSensor_active
```

The sensor specific functions are disabled (commented) by default. 

**Do not enable these function until required.** Additional code might be required as well. You can find examples in my other projects.



#### Generic device section

```c++
// device settings - change settings to match your requirements

String deviceID = "DEV"; //predefinded sensor ID, DEV by default to prevent overwriting productive data
String deviceName = "DaylightSensor"; //predefinded sensor ID, DEV by default to prevent overwriting productive data

bool devMode = true; //enable DEV mode on boot (do not change)
bool debug = true; //debug to serial monitor
bool ledActive = true; //enable external status LED on boot
bool deviceActive = false; // dectivate device (all sensors) on boot (do not change until required)

int interval = 10;  // (initial) interval between measurements / actions, multiplied with the intervalDelay
int intervalDelay = 1000; // in ms
```

- The Wifi information are located in the file WiFi_secret.h
- The MQTT authentication information are located in the file MQTT_secret.h
- The **`deviceID`** is part of the MQTT topic and important for the the iobroker communications. It **must** be equal to the datapoint path in your ioBroker!
- The **`deviceName`** is part of the MQTT topic and important for the the iobroker communications. It **must** be equal to the datapoint path in your ioBroker!
- The **`devMode`** switch prevents the device from sending data into your productive datapoints. It is enabled by default and can be overwritten dynamically from iobroker
- **`debug`** enables the detailed serial output
- **`ledActive`** enables the onboard led (status)
- **`interval`** defines the time between two data transmissions (interval * intervalDelay). This value is used initially after boot. The interval can dynamically updated from iobroker
- **`intervalDelay`** defines the waiting time at the end of each loop. This value is used initially after boot. The interval can dynamically updated from iobroker (this value has no effect if the BME680 sensor is activated!)
- The **`deviceActive`** switch enables sensors and data transmissions. This is very useful to test a sketch on the bread board without the connected hardware. It is disabled by default and gets dynamically changed by the iobrocker, as long as nothing else is configured.

#### MQTT section

```c++
int MQTT_port = 1883; // MQTT port (default 1883)
const char MQTT_broker[] = "192.168.1.240";  // IP of iobroker / MQTT broker

String MQTT_deviceRootPath = "0_userdata/0/IoT-Devices/"; // The root path for all the sensor's (devices) state and configuration subscription in your MQTT/iobroker
String MQTT_prodDataRootPath = "0_userdata/0/IoT/"; // The root path for all the sensor's productive data in your MQTT/iobroker
String MQTT_devDataRootPath = "0_userdata/0/IoT-Dev/"; // The root path for all the sensor's productive data in your MQTT/iobroker
```

- **`MQTT_port`** is 1883 by default
- **`MQTT_broker`** is usually the IP address of your ioBroker. The sketch does not use DNS!
- **`MQTT_deviceRootPath`**, **`MQTT_prodDataRootPath`** and **`MQTT_devDataRootPath`** must correspond to the ioBroker datapoints! 



## iobroker datapoints

#### Devices section

The device section contains the configuration and status information about the IoT device (hardware board) itself. All Sensor data and config will be stored in the data section.

The MQTT topic for the device section is a combination of the MQTT_deviceRootPath and the deviceID. For example:

```
0_userdata/0/IoT-Devices + /DEV -> topic for device state
0_userdata/0/IoT-Devices + /DEV + /Config -> topic for device config (subscription)
```

The default path for the devices root folder is: **`0_userdata.0.IoT-Devices`**. When the path is changed in ioBroker, it has to be changed in the sketch as well.

**It is mandatory to setup the following datapoints prior the first device boot:**

- States
  - **`/DeviceIP`** the current IP address of the device
  - **`/DeviceName`** the current Name of the device
  - **`/ErrorLog`** the last error message
  - **`/MAC`** the current MAC address of the device
  - **`/RSSI`** the current WiFi RSSI of the device
  - **`/Reset`** the timestamp of the last reset / boot (depricated)
- Config
  - **`/Debug`** [false] enables / disables serial debug output
  - **`/Delay`** [1000] loop delay (in ms) (ignored if BME680 is active)
  - **`/DevMode`** [false] enables / disables DevMode. In DevMode, all data will be written into a different data path on the ioBroker
  - **`/DeviceActive`** [true] enables / disables **all** sensors and data transmits
  - **`/Interval`** [10] data transmit every n-th loop
  - **`/LED`** [true] enables / disables status LED



#### Data section

In the data section, all the sensor data will be stored. In addition, some optional, specific sensor configuration can be stored here as well. Depending on the DevMode option, the data will be stored in the production or the development path.

The MQTT topic for the data section is a combination of the MQTT_????DataRootPath and the deviceName. For example:

```
0_userdata/0/IoT + /DaylightSensor -> topic for production data
0_userdata/0/IoT-Dev + /DaylightSensor -> topic for development data
```



The default path for the devices root folder is: **`0_userdata.0.IoT-Devices`**. When the path is changed in ioBroker, it has to be changed in the sketch as well.

The data section must contain at least one state:

- **`/DeviceIP`** the current ID of the device



#### Sensor specific datapoints

##### ADS1115 (DaylightSensor)

- Sensor data
  - **`Daylight01`** ADC output for daylight sensor 1
  - **`Daylight02`** ADC output for daylight sensor 2

##### Indoor Air Sensor (BME280, BME680, SCD30)

- Sensor data
  - **`Airpressure`** Ambient air pressure in mBar (BME280)
  - **`Altitude`** Altitude (BME280)
  - **`BME680_eraseEEPROM`** Timestamp for the last eraseEEPROM event
  - **`BME680_loadState`** Timestamp for the last loadState event
  - **`BME680_updateStatet`** Timestamp for the last updateState event
  - **`Humidity`** Humidity in % (BME280)
  - **`Temperatur`** Temperature in °C (BME280)
  - **`iaqS, iaqD`** IAQ index static, IAQ index dynamic (BME680)
  - **`iaSA, iaDA`** IAQ accuracy static, IAQ accuracy dynamic (BME680)
  - **`VOCe`** VOC equivalent, in mg/m³ (BME680)
  - **`co2`** CO2 concentration in ppm (SCD30)
- Sensor config

  - **`BME680_reset`** [false] Triggers a reset (EraseEEPROM) of the BME680 sensor when true. Flips back to false when the reset was done 
  - **`SCD30_autoCal`** [true] Enables the SCD30 ASC
  - **`LastUpdate`** LastUpdate string for the ePaper display, automatically updated by JavaScript in ioBroker

##### Wind Sensor - input:

- Sensor data
  - **`WindDirectionArray`** Array of wind direction values
  - **`WindSpeedArray`** Array of wind speed values
  - **`crcErrors`** CRC errors during the last interval
  - **`rxTimeOuts`** RX timeouts (missed frames) during the last interval
- Sensor config
  - **`A0_Step_Voltage`** The voltage per step of the A/D converter for the  windspeed sensor. Due to the line resistance it might be necessary to adjust these values individually. A good point to start is 0.03 V.



## How it works

#### Boot phase / setup

- Connect Wifi / Get Wifi State
- Connect MQTT / Get MQTT State
- Send device state
- Get initial configuration from ioBroker
  - Subscribe MQTT device config topics
  - Get (parse) MQTT messages (device & sensor specific config)
  - Subscribe MQTT sensor specific config topics
  - Get (parse) MQTT messages (device & sensor specific config)
- Run sensor setup (if deviceActive = true)

#### Loop

The main loop has a default frequency of round about 1 Hz (1000ms **`delay`**) and blinks the status led with 0,5 Hz (when enabled). With the BME680 sensor, the loop has to wait for the sensor date. A single measurement takes round about 3 seconds, which leads into a frequency of 0,3 Hz. The **`delay`** will be ignored. 

In each loop, the data of all (active) sensors are measured and possible MQTT messages are parsed to obtain config changes. This includes the setup for disabled sensors. Optionally, the data can be send as a debug log to the serial interface of the device.

Every n-th loop, defined by the **`Interval`**, the data is transmitted to ioBroker and the optional ePaper display is updated.



## Appendix

#### BME680 / BSEC

The Bosch BSEC library uses precompiled libraries. You need to make some changes to the ESP8266 plattform.txt file to allow and include precompiled libraries:

1. find the plattform.txt file for your hardware package. The default path on a Windows PC should be for example: 

   ```
   C:\Users\<username>\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.7.4
   ```

   

2. Add the line "compiler.libraries.ldflags=" (block starts at line #82):

   ```c++
   # These can be overridden in platform.local.txt
   compiler.c.extra_flags=
   compiler.c.elf.extra_flags=
   compiler.S.extra_flags=
   compiler.cpp.extra_flags=
   compiler.ar.extra_flags=
   compiler.objcopy.eep.extra_flags=
   compiler.elf2hex.extra_flags=
   #### added for BSEC
   compiler.libraries.ldflags=
   ```

   

3. Change the line "Combine gc-sections, archives, and objects" (starts at line #113) and add "{compiler.libraries.ldflags}" directive at the suggested position:

   ```
   ## Combine gc-sections, archives, and objects
   # recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" {build.exception_flags} -Wl,-Map "-Wl,{build.path}/{build.project_name}.map" {compiler.c.elf.flags} {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" -Wl,--start-group {object_files} "{archive_file_path}" {compiler.c.elf.libs} -Wl,--end-group  "-L{build.path}"
   #### changed for BSEC
   recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" {build.exception_flags} -Wl,-Map "-Wl,{build.path}/{build.project_name}.map" {compiler.c.elf.flags} {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" -Wl,--start-group {object_files} "{archive_file_path}" {compiler.c.elf.libs} {compiler.libraries.ldflags} -Wl,--end-group  "-L{build.path}"
   
   ```

   