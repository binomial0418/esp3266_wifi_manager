#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ap_ssid       ;
const char* ap_password   ;
const char* WIFI_FILE     = "/wifi.ini";

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    WIFI SSID    : <input type="text" name="ssid"><br>
    WIFI Password: <input type="text" name="password"><br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  String wifiIni;
  bool   goAPMode = false;
  String ssid = ""      ;
  String password = ""  ;
  int timeoutCount = 10;
  bool onOff = true;
  
  Serial.begin(115200);
 
  Serial.println(" ");
  wifiIni = readConfigFile(WIFI_FILE);
  ssid = getSsidPwdFromJson(wifiIni,"ssid");
  password = getSsidPwdFromJson(wifiIni,"pwd");
  //Serial.println(String(ap_ssid) + String(ap_password) );

  if (ssid.length() > 0 && password.length() > 0){  
    Serial.println("WIFI MODE ");
    Serial.print(F("Wait for WiFi... "));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    while (WiFi.status() != WL_CONNECTED && timeoutCount > 0){
      Serial.print(".");
      onOff = !onOff;
      digitalWrite(LED_BUILTIN, onOff ? LOW : HIGH);
      delay(1000);
      timeoutCount--;
    }
    goAPMode = timeoutCount == 0;
   
    if (goAPMode == true) {
      Serial.println("WiFi Failed!");
    }else{
      Serial.println();
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());    
    }    
  }
  if (goAPMode == true){
    auto chipID = ESP.getChipId();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("AP MODE ");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(String("ESP-") + String(chipID, HEX)); 
   // Send web page with input fields to client
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
   });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
   server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String ssid;
    String pwd;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      ssid = request->getParam(PARAM_INPUT_1)->value();
      pwd  = request->getParam(PARAM_INPUT_2)->value();
      writeWifiConfigFile(WIFI_FILE,ssid,pwd);
//      ssid = readConfigFile(WIFI_FILE);  
//      Serial.println("readfile:" + ssid);
    }
    //轉換網頁
//    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
//                                     + inputParam + ") with value: " + inputMessage +
//                                     "<br><a href=\"/\">Return to Home Page</a>");
    request->send(200, "text/html", "<a href=\"/\">Return to Home Page</a>");                                     
  });
  server.onNotFound(notFound);
  server.begin();
  }
}

void loop()
{
 
}


String readConfigFile(String typ){
  String s;
  SPIFFS.begin();
  File data = SPIFFS.open(typ, "r");   
  
  size_t size = data.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // Read and store file contents in buf
  data.readBytes(buf.get(), size-2);
  // Closing file
  data.close();
  //Serial.println(String(typ) + ":" + String(buf.get()));
  s = String(buf.get());
  return s;
}

void writeWifiConfigFile(String typ,String ssid,String pwd){
  String str;
  StaticJsonDocument<200> json_doc;
  char json_output[100];
  json_doc["ssid"] = ssid + "#";
  json_doc["pwd"]  = pwd + "#";
  serializeJson(json_doc, json_output);
 
  SPIFFS.begin();
  File f = SPIFFS.open(typ, "w");  
  if (!f) {
    Serial.println("Failed to open config file for writing");
  }
  f.println(json_output);
  f.close();
}


String getSsidPwdFromJson(String val,String field){
  StaticJsonDocument<200> json_doc;
  DeserializationError json_error;
  const char* ssid;
  const char* pwd;
  int i;
  
  json_error = deserializeJson(json_doc, val);
  if (!json_error) {
    String s    = String(json_doc[field]);
    i = s.indexOf('#');
    s = s.substring(0,i);
    //Serial.println(field + ":" + s);
    
    return s;
  }
}
