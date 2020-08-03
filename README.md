# ioBroker-IoT-DEV

#### ioBroker IoT DEV Environment (based on NodeMCU)

Working with IoT sensors and actors in ioBroker often needs a lot of development and testing. Every improvement needs a staging environment as well to keep the productive data untouched. I’ve started to setup a more or less universal dev environment for IoT devices like NodeMCU or Arduino. It enables a convenient and easy way to switch between dev and prod without uploading a new image to the device. 

## Prerequisites

* You need a running Arduino IDE and at least basic knowledge how it works. 
* You also need a running ioBroker instance and a little more than basic knowledge on that stuff.
* You need a REST API in your ioBroker setup
* You need to secure the REST API (for example, expose the required data only and hide any confidential and secret stuff)
* You need a userdata section in your ioBroker objects
>* IoT-Sensors: configuration data for every individual sensor (linked with the sensor ID) sample: 0_userdata.0.IoT-Sensors.01.json
>* IoT-Dev: I/O data section for developement (URL’s polled from the configuration data) sample: 0_userdata.0.IoT-Dev.Dev01.json
>* IoT: I/O data section for production (URL’s polled from the configuration data) sample: 0_userdata.0.IoT.Weather.json
* You should know how to work with IoT devices and electronics

## How it works – the basics

* The main loop has a delay of round about 1 second
>*  optional execution for (near) real time tasks every 50ms
>* 1 Hz blink interval for LED (status indicator)
>* push/pull data every x seconds (interval)
>* automatic reset when ioBroker not reachable (delayed by 30 seconds and indicated by a flashing LED)
* The sensor connects periodically to the REST API and pulls some data. This enables you to use the deep sleep mode when your device is running on battery (not covered in this image)
>* DevMode (bool): Device is in dev mode
>* Interval (int): Poll interval
>* baseURL_DATA_GET/SET(string): based on DevMode and configured URL’s in ioBroker. For example:
>>* http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Dev.Weather.
>>* http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT.Weather.
>>* http://192.168.1.240:8087/set/0_userdata.0.IoT.Weather.
* The sensor also posts some information to ioBroker:
>* SensorIP (string): current IP (interesting with DHCP)
>* Reset (bool): updates timestamp in ioBroker every time the sensor starts up

## Generic sensor config
This is the minimum part of the sketch you need to change before uploading:

    //-------------------------------------------------------
    // generic sensor config begin
    
    const char* ssid     = "yyyyyy";
    const char* password = "xxxxxx";
    
    String SensorID = "01";
    String baseURL_SENSOR_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Sensors." + SensorID + ".";
    String baseURL_SENSOR_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Sensors." + SensorID + ".";
    
    //-------------------------------------------------------
    // generic sensor config end

## Usage
Quite simple:
* create required the data structure in ioBroker for dev/prod
* set the generic configuration in your sketch
* start with DevMode = true and a low interval
* add your own code execution into the 1 second loop (// your code here) or the 50ms loop (//execute realtime tasks here). 
* switch between dev and prod by simply changing the DevMode object

## History

Initial release of version 2.0: 2020-08-03
