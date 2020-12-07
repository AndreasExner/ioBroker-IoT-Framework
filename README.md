# ioBroker-IoT-Framework

**ioBroker IoT Framework (based on NodeMCU / ESP8266)**

Working with IoT sensors and actors in ioBroker often needs a lot of development and testing. Every improvement needs a staging environment as well to keep the productive data untouched. I’ve started to setup a more or less universal dev environment for IoT devices like NodeMCU or Arduino. It enables a convenient and easy way to switch between dev and prod without uploading a new image to the device. 

With the **3rd generation** of the Framework I stared to include modules for hardware sensors like the BME280 to make them reusable in further projects.

With the **5th generation** of the Framework was completely restructured because the long code became more and more unhandy. All generic and hardware specific sensors functions are moved the extension file AEX_iobroker_IoT_Framework.ino. 

**Important:**

Be sure to have this file in the same folder like the primary .ino file. Opening the primary .ino file in Arduino editor should loaded  the extension automatically.

## History

**5.2.0: sensor update**

- added SPS30
- added Wind speed and direction sensor (including software serial)
- minor bugfixes and improvements

**5.0.0: initial V5 release**



## Supported sensors

- BME280 (based on the [Adafruit_BME280_Library version 2.1.2 ](https://github.com/adafruit/Adafruit_BME280_Library))
- BME680 (based on the [BoschSensortec Arduino library for BSEC 1.6.1480](https://github.com/BoschSensortec/BSEC-Arduino-library))
- SCD30 (based on the [SparkFun SCD30 Arduino library release 9](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library))
- SPS30 (based on the [Sensirion embedded SPS library 3.1.0](https://github.com/Sensirion/embedded-sps))
- WindSensor
  - Software Serial ([based on the EspSoftwareSerial library 6.10.0](https://github.com/plerup/espsoftwareserial))

**Important:**

The Bosch BSEC library uses precompiled libraries. See the appendix for some changes to the ESP8266 plattform.txt file.



## Tested environment

- Software
  - Arduino IDE 1.8.13 (Windows)
  - ESP8266 hardware package 2.7.4
  - Adafruit_BME280_Library version 2.1.2
  - BSEC-Arduino-library 1.6.1480
  - SparkFun_SCD30_Arduino_Library release 9
  - Sensirion embedded SPS library 3.1.0
  - ESPSoftwareSerial library 6.10.0
- Hardware
  - NodeMCU Lolin V3 (ESP8266MOD 12-F)
  - NodeMCU D1 Mini (ESP8266MOD 12-F)
  - GY-BME280
  - CJMCU-680 BME680
  - Sensirion SCD30
  - Sensirion SPS30
  - 5 V MAX485 / RS485 Modul TTL to RS-485 MCU



## Prerequisites

* You need a running Arduino IDE and at least basic knowledge how it works. 
* You also need a running ioBroker instance and a little more than basic knowledge on that stuff.
* You need a REST API in your ioBroker setup
* You need to **secure** the REST API (for example, expose the required data only and hide any confidential and secret stuff)
* You need a userdata section in your ioBroker objects
* You should know how to work with IoT devices and electronics



## Setup

- Create a folder in your Arduino library folder
- Copy the primary sketch (e.g. DEV_5.0.ino) and the extension file (AEX_iobroker_IoT_Framework.ino) into the folder
- Open the primary sketch (e.g. DEV_5.0.ino) 
- Optional: install required libraries into your Arduino IDE
- Create (import) the datapoints in iobroker
  - 0_userdata.0.IoT-Devices.DEV.json (generic device configuration and monitoring)
  - 0_userdata.0.IoT-Dev.DEV.json (specific device configuration and data)
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

**Do not enable these function until required.** Additional code is required as well. You can find examples in my other projects.



#### Generic device section

```c++
// device settings - change settings to match your requirements

const char* ssid     = "<ssid>"; // Wifi SSID
const char* password = "<password>"; //Wifi password

String SensorID = "DEV"; //predefinded sensor ID, DEV by default to prevent overwriting productive data

int interval = 10;  // waiting time for the first masurement and fallback on error reading interval from iobroker

bool DevMode = true; //enable DEV mode on boot (do not change)
bool debug = true; //debug to serial monitor
bool led = true; //enable external status LED on boot
bool sensor_active = false; // dectivate sensor(s) on boot (do not change)
```

- Enter proper Wifi information
- The **`SensorID`** is part of the URL's and important for the the iobroker communications
- **`Interval`** defines the delay between two data transmissions / measurements. This value is used initially after boot. The interval dynamically updates from iobroker
- The **`DevMode`** switch prevents the device from sending data into your productive datapoints. It is enabled by default and can be overwritten dynamically from iobroker
- **`debug`** enables a more detailed serial output
- **`led`** enables the onboard led (status)
- The **`sensor_active`** switch disables the loading of hardware specific code on startup. This is very useful to test a sketch on the bread board without the connected hardware. It is disabled by default and gets dynamically enabled from the iobrocker during boot, as long as nothing else is configured.



#### Base URL's

```c++
/*
 * build base URL's
 * Change IP/FQND and path to match your environment
 */

String baseURL_DEVICE_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Devices." + SensorID + ".";
String baseURL_DEVICE_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Devices." + SensorID + ".";
```

The base url's, one for read and one to write data, are pointing to your iobroker datapoints in the devices section. The SensorID will be added to complete the path. 



## iobroker datapoints

#### Devices section

The default path for the devices root folder is: **`0_userdata.0.IoT-Devices`**. When the path is changed, it has to be changed in the sketch as well.

**It is mandatory to setup the following datapoints prior the first device boot:**

- **`DevMode`** If true, the baseURL's for the IoT-Dev section are used to prevent overwriting your production data. Also see generic device section.

- **`LED`** Controls the status LED. Also see generic device section.

- **`SensorActive`** Controls the hardware sensors. Also see generic device section.

- **`SensorName`** Easy to understand name for the sensor. Not used in the code.

- **`baseURL_GET_DEV`** points to the IoT-Dev datapoints in iobroker (e.g. 0_userdata.0.IoT-Dev.DEV)

  - ```
    http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Dev.DEV.
    ```

- **`baseURL_SET_DEV`** points to the IoT-Dev datapoints in iobroker (e.g. 0_userdata.0.IoT-Dev.DEV)

  - ```
    http://192.168.1.240:8087/set/0_userdata.0.IoT-Dev.DEV.
    ```

- **`baseURL_GET_PROD`** points to the IoT datapoints in iobroker (e.g. 0_userdata.0.IoT.Weather)

  - ```
    http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT.Weather.
    ```

- **`baseURL_SET_PROD`** points to the IoT datapoints in iobroker (e.g. 0_userdata.0.IoT.Weather)

  - ```
    http://192.168.1.240:8087/set/0_userdata.0.IoT.Weather.
    ```



#### Specific device configuration and data

Depending on the **`DevMode`**, the device reads config and writes data into different datapoint sections:

- Development root folder: **`0_userdata.0.IoT-Dev`**
- Production root folder: **`0_userdata.0.IoT`**

It is recommended to keep the datapoints in both sections identical to avoid errors when changing the **`DevMode`**. The values can be different. In the basic template only one datapoint is mandatory:

- **`Interval`** Defines the delay between two data transmissions / measurements. Also see generic device section.



## How it works

#### Boot phase / setup

- Connect Wifi
- Get Wifi State
- Get the dynamic configuration from iobroker (generic devices section)
- Build specific device URL's (based on the dynamic configuration)
- Send devices IP address
- Send device ID
- Send information about the last restart / reset
- Optional: run's setup for active sensors



#### Loop

The main loop has a frequency of 1 Hz and blinks the status led with 0,5 Hz (when enabled). Every n-th tick, defined by the **`Interval`**,  the following sequence will proceed:

- Loop (1Hz)
  - **`01`**
  - Interval reached
    - Get Wifi State (try reconnect first, then reset if not connected)
    - Send devices IP address
    - Get the dynamic configuration from iobroker (generic devices section)
    - Build specific device URL's (based on the dynamic configuration)
    - **`02`**
    - Get the new interval (specific device section)
  - Interval not reached
    - **`03`**
  - LED Blink 
    - **`04`**



#### Thread handling

Depending on the requirements and the used sensors, the loop can be look completely different (e.g BME680). However, the recommended positions for additional treads are:

- **`01`** Get data from sensor / serial output
- **`02`** Send date to iobroker, run sensor setup (if required)
- **`03`** Optional, serial output counter
- **`04`** Can be used for "real time" task that must run more frequently then in the 1 Hz loop (not implemented in this example)



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

   
