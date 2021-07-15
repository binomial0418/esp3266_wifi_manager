#include <Arduino.h>
#include <FS.h>

#include <ESP8266WiFi.h>

const char* CONFIG_FILE = "/config.json";
const char* SSID_FILE = "/ssid.ini";
const char* PWD_FILE = "/pwd.ini";
String ssid = "123"     ;
String password = "456"  ;

void setup()
{
  Serial.begin(115200);
  writeConfigFile(SSID_FILE,ssid);
  writeConfigFile(PWD_FILE,password);
  ssid = readConfigFile(SSID_FILE);  
  password = readConfigFile(PWD_FILE);
  
}

void loop()
{
 
}


String readConfigFile(String typ){
  File data = SPIFFS.open(typ, "r");   
  
  size_t size = data.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // Read and store file contents in buf
  data.readBytes(buf.get(), size);
  // Closing file
  data.close();
  Serial.print(String(typ) + ":" + String(buf.get()));
  return String(buf.get());
}

void writeConfigFile(String typ,String val){
  Serial.println("Saving config file:" + typ);
//  Serial.println("SPIFFS format start");
//  SPIFFS.format();    // 格式化SPIFFS
//  Serial.println("SPIFFS format finish");
  if(SPIFFS.begin()){ // 啟動SPIFFS
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
  File f = SPIFFS.open(typ, "w");  
  
  if (!f) {
    Serial.println("Failed to open config file for writing");
  }
  f.println(val);
  f.close();
  Serial.println("\nConfig file was successfully saved");
}
