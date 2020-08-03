#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define LED D4

//-------------------------------------------------------
// generic sensor config

const char* ssid     = "yyyyyyyy";
const char* password = "xxxxxxxx";

String SensorID = "DEV";

String baseURL_SENSOR_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Sensors." + SensorID + ".";
String baseURL_SENSOR_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Sensors." + SensorID + ".";

String URL_DevMode = baseURL_SENSOR_GET + "DevMode";
String URL_IP = baseURL_SENSOR_SET + "SensorIP?value=";
String URL_RST = baseURL_SENSOR_SET + "Reset?value=";
String URL_SID, URL_INT;

bool DevMode = true;

int interval = 30; 
int counter = 0;

void(* HWReset) (void) = 0;

String baseURL_DATA_GET, baseURL_DATA_SET;

//-------------------------------------------------------
// specific sensor config



//####################################################################
// Setup

void setup() {  
  
  Serial.begin(115200);  
  delay(100);
  
  pinMode(LED, OUTPUT);

//---------------------------------------------------------------------
// WiFi Connect

  Serial.println('\n');
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  digitalWrite(LED, HIGH);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i > 20) {
      Serial.println("Connection failed!");  
      reboot_on_error();
    }
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  get_dynamic_config();
  build_urls();
  send_ip();
  send_sid();
  send_rst();
  get_interval();
}


//####################################################################
// Reboot on Error

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


//####################################################################
// get Interval from iobroker

void get_interval() {

  int intervall_current = interval;
  
  HTTPClient http;
  Serial.println("URL_INT = " + URL_INT);
  http.begin(URL_INT);
  http.GET();
  
  int interval_new = http.getString().toInt();

  if (intervall_current != interval_new) {
    interval = interval_new;
    Serial.println("New Interval = " + String(interval));  
  }
  
  http.end();
}

//####################################################################
// get dynamic sensor config from iobroker

void get_dynamic_config() {

  // ---------------------------------------------
  // get DevMode

  Serial.println("URL_DevMode = " + URL_DevMode);
  
  HTTPClient http;
  
  http.begin(URL_DevMode);
  int httpcode = http.GET();

  if (httpcode != 200) {
    Serial.println("Error getting DevMode -> switch to DevMode = true");
    DevMode = true;
  }
  else {

    String DevModeString = http.getString();

    Serial.println("DevMode = " + DevModeString);

    if(DevModeString == "false") {DevMode = false;}
    else {DevMode = true;}
  }

  http.end();

  // ---------------------------------------------
  // get BaseURLs

  if (DevMode == true) {
  
    http.begin(baseURL_SENSOR_GET + "baseURL_GET_DEV");
    httpcode = http.GET();
    if (httpcode == 200) { 
      baseURL_DATA_GET = http.getString();
    }
    else { Serial.println("Error getting baseURL_DATA_GET");}
      
    http.begin(baseURL_SENSOR_GET + "baseURL_SET_DEV");
    httpcode = http.GET();
    if (httpcode == 200) { 
      baseURL_DATA_SET = http.getString();
    }
    else { Serial.println("Error getting baseURL_DATA_SET");}
  }
  else {

    http.begin(baseURL_SENSOR_GET + "baseURL_GET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) { 
      baseURL_DATA_GET = http.getString();
    }
    else { Serial.println("Error getting baseURL_DATA_GET");}
      
    http.begin(baseURL_SENSOR_GET + "baseURL_SET_PROD");
    httpcode = http.GET();
    if (httpcode == 200) { 
      baseURL_DATA_SET = http.getString();
    }
    else { Serial.println("Error getting baseURL_DATA_SET");}
  }

  baseURL_DATA_GET.replace("\"", "");
  baseURL_DATA_SET.replace("\"", "");

  Serial.println("baseURL_DATA_GET = " + baseURL_DATA_GET);
  Serial.println("baseURL_DATA_SET = " + baseURL_DATA_SET);
  
}

//####################################################################
// Build URL's

void build_urls() {

  URL_INT = baseURL_DATA_GET + "Interval";
  URL_SID = baseURL_DATA_SET + "SensorID?value=" + SensorID;
 
}

//####################################################################
// Send local IP to ioBroker

void send_ip() {
  
  HTTPClient http;
  
  Serial.println(WiFi.localIP());

  String sendURL = URL_IP + WiFi.localIP().toString();

  http.begin(sendURL);
  int httpcode = http.GET();
  String payload = http.getString();
  Serial.println("HTTP Return = " + String(httpcode));
  Serial.println(payload);
  http.end();

  if (httpcode != 200) {
    Serial.println("Error sending data to iobroker!");
    reboot_on_error();    
  }
} 

//####################################################################
// Send Sensor ID to ioBroker

void send_sid() {

  Serial.println("Send SID = " + SensorID);
  
  HTTPClient http;
  
  http.begin(URL_SID);
  http.GET();
  http.getString();
  http.end();
} 


//####################################################################
// Send RST to ioBroker

void send_rst() {
  
  HTTPClient http;
  
  Serial.println("Send RST");
  String sendURL = URL_RST + "true";

  http.begin(sendURL);
  http.GET();
  http.getString();
  http.end();
}  


//####################################################################
// Loop
  
void loop() {

  int c = 0;

  if (counter >= interval) {
    get_dynamic_config();
    build_urls();
    send_ip();
    get_interval();
    
    // your code here
    
    counter = 0;
  }
  Serial.println(counter);
  ++counter;

   
//---------------------------------------------------------------------
// Blink LED

  while (c < 10) {
    delay(50);

//execute realtime tasks here

    ++c;
  }

  digitalWrite(LED, HIGH);        
  c = 0;

  while (c < 10) {
    delay(50);

//execute realtime tasks here

    ++c;
  }

  digitalWrite(LED, LOW);         
  c = 0;
}
