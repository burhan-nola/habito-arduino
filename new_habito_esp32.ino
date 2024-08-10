#include "Wire.h"
#include "PN532_I2C.h"
#include "PN532.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);


int led1 = D9; //red
int led2 = D10; //green
int led3 = D8; //blue
int led4 = D7; //yellow

String redCard = "BECE816E";
String greenCard = "6EF18B6E";
String yellowCard = "BEE0806E";
String blueCard = "4E637F6E";

String userRFID = "";

const int BUTTON_PIN = D3;
const int SHORT_PRESS_TIME = 1000; 

// Variables will change:
int lastState = LOW;
int currentState;
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

int buton = D3;

// Ganti dengan nama dan kata sandi Wi-Fi Anda
const char* ssid = "Bukan WiFi Gratis";
const char* password = "langsungconnect";
String idDevice = "habito_002";

// Ganti dengan URL API yang ingin Anda akses
const char* url = "https://habito-api.vercel.app";

void setup(void)
{

  Serial.begin(115200);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(led4,OUTPUT);
  pinMode(buton,INPUT_PULLUP);
  
  Serial.println("NFC/RFID Reader");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
  Serial.print("Didn't find PN53x board");
  while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  //  Non-blocking procedure
  nfc.setPassiveActivationRetries(0x01);
 
  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");
  // Memulai koneksi ke Wi-Fi
  WiFi.begin(ssid, password);

  // Menunggu hingga terhubung ke Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    digitalWrite(led1,HIGH);
    digitalWrite(led2,HIGH);
    digitalWrite(led3,HIGH);
    digitalWrite(led4,HIGH);
  }

  // Menampilkan informasi jika terhubung
  Serial.println("");
  Serial.println("Terhubung ke Wi-Fi");
  
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    int i=0;
    while(WiFi.status() != WL_CONNECTED){ 
      Serial.print(".");
      digitalWrite(led4,LOW);
      digitalWrite(led1,HIGH);
      delay(200);
      digitalWrite(led1,LOW);
      digitalWrite(led2,HIGH);
      delay(200);
      digitalWrite(led2,LOW);
      digitalWrite(led3,HIGH);
      delay(200);
      digitalWrite(led3,LOW);
      digitalWrite(led4,HIGH);
      delay(200);  
    } 
  Serial.println("");
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(2000);
}

void sendData(String light, int led) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String address ="/light?id=";
    String fullUrl;
    fullUrl = url;
    fullUrl += address;
    fullUrl += idDevice;
    fullUrl += "&light=";
    fullUrl += light;
    Serial.print("Requesting: ");
    Serial.println(fullUrl); 
    http.begin(fullUrl);  //Specify request destination
    int httpCode = http.GET();//Send the request
    String payload;  
    if (httpCode > 0) { //Check the returning code    
        payload = http.getString();   //Get the request response payload
        payload.trim();
        if( payload.length() > 0 ){
           Serial.println(payload + "\n");
        }
        digitalWrite(led,HIGH);
        delay(2000);
        digitalWrite(led,LOW);
    } else{
      payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
      digitalWrite(led,HIGH);
      delay(100);
      digitalWrite(led,LOW);
      delay(100);
      digitalWrite(led,HIGH);
      delay(100);
      digitalWrite(led,LOW);
    }
    
    http.end();   //Close connection
    }else{
      Serial.print("Not connected to wifi ");Serial.println(ssid);
      WiFi.begin(ssid, password);
    }
}

void online() {
  char ip[16]; // Menyimpan alamat IP sebagai string
  WiFi.localIP().toString().toCharArray(ip, 16); // Mendapatkan alamat IP dan mengonversinya menjadi string
  
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = idDevice;
  jsonDoc["ip"] = ip;
  jsonDoc["ssid"] = ssid;

  char jsonString[200]; // Buffer untuk menyimpan string JSON
  serializeJson(jsonDoc, jsonString, sizeof(jsonString)); // Serialisasi JSON

  HTTPClient https;
  String endpoint = "/keep-online";

  String fullUrl = url + endpoint;

  Serial.print("Requesting: ");
  Serial.println(fullUrl);

  if (https.begin(fullUrl)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(jsonString);
    Serial.print("HTTP Response Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, https.getString());
        String deviceStatus = doc["message"];// Ambil status perangkat dari respons JSON
        Serial.println(deviceStatus);
        digitalWrite(led1,HIGH);
        digitalWrite(led2,HIGH);
        digitalWrite(led3,HIGH);
        digitalWrite(led4,HIGH);
        delay(200);
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
        digitalWrite(led3,LOW);
        digitalWrite(led4,LOW);
      }
    } else {
      Serial.println(httpCode);
    }
    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
}

void lightStatus() {
  
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = idDevice;

  char jsonString[200]; // Buffer untuk menyimpan string JSON
  serializeJson(jsonDoc, jsonString, sizeof(jsonString)); // Serialisasi JSON


  HTTPClient https;
  String endpoint = "/light-status";

  String fullUrl = url + endpoint;

  Serial.print("Requesting: ");
  Serial.println(fullUrl);

  if (https.begin(fullUrl)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(jsonString);
    Serial.print("HTTP Response Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, https.getString());
        String redStatus = doc["red"]["status"];
        String greenStatus = doc["green"]["status"];
        String blueStatus = doc["blue"]["status"];
        String yellowStatus = doc["yellow"]["status"];
        if(redStatus == "true"){
          digitalWrite(led1,HIGH);
        }
        if(greenStatus == "true"){
          digitalWrite(led2,HIGH);
        }
        if(blueStatus == "true"){
          digitalWrite(led3,HIGH);
        }
        if(yellowStatus == "true"){
          digitalWrite(led4,HIGH);
        }
        delay(2000);
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
        digitalWrite(led3,LOW);
        digitalWrite(led4,LOW);
      }
    } else {
      Serial.println(httpCode);
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,HIGH);
      digitalWrite(led4,HIGH);
      delay(100);
      digitalWrite(led1,LOW);
      digitalWrite(led2,LOW);
      digitalWrite(led3,LOW);
      digitalWrite(led4,LOW);
      delay(100);
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,HIGH);
      digitalWrite(led4,HIGH);
      delay(100);
      digitalWrite(led1,LOW);
      digitalWrite(led2,LOW);
      digitalWrite(led3,LOW);
      digitalWrite(led4,LOW);
    }
    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
}

static unsigned long lastRFIDPollTime = 0;
const unsigned long RFIDPollInterval = 50000;

void loop()
{
  if(WiFi.status() != WL_CONNECTED){
    connectWiFi();
    online();
  }
  
  unsigned long currentMillis = millis();

  if (currentMillis - lastRFIDPollTime >= RFIDPollInterval) {
    lastRFIDPollTime = currentMillis;
    online();
  }

    
  currentState = digitalRead(BUTTON_PIN);
  if(lastState == HIGH && currentState == LOW)        // button is pressed
    pressedTime = millis();
  else if(lastState == LOW && currentState == HIGH) { // button is released
    releasedTime = millis();
    
    long pressDuration = releasedTime - pressedTime;
    if( pressDuration < SHORT_PRESS_TIME )
        {
          Serial.println("A short press is detected");
          digitalWrite(led1,HIGH);
          digitalWrite(led2,HIGH);
          digitalWrite(led3,HIGH);
          digitalWrite(led4,HIGH);
          delay(100);
          digitalWrite(led1,LOW);
          digitalWrite(led2,LOW);
          digitalWrite(led3,LOW);
          digitalWrite(led4,LOW);
          lightStatus();
        }
    if( pressDuration > SHORT_PRESS_TIME )
      {
        Serial.println("A Long press is detected");
//        WiFi.disconnect();
        digitalWrite(led1,HIGH);
        digitalWrite(led2,HIGH);
        digitalWrite(led3,HIGH);
        digitalWrite(led4,HIGH);
        delay(1000);
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
        digitalWrite(led3,LOW);
        digitalWrite(led4,LOW);
//        connectWiFi();
      }
  }
  lastState = currentState;
  
  readRFID();
}

void readRFID(void)
{
  boolean success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success)
  {
  for (uint8_t i = 0; i < uidLength; i++)
    {
    if (uid[i] <= 0xF) {
      userRFID += "0";
    }
    userRFID += String(uid[i] & 0xFF, HEX);
  }
  userRFID.toUpperCase();
  Serial.println(userRFID);

    digitalWrite(led1,HIGH);
    delay(100);
    digitalWrite(led1,LOW);
    digitalWrite(led2,HIGH);
    delay(100);
    digitalWrite(led2,LOW);
    digitalWrite(led3,HIGH);
    delay(100);
    digitalWrite(led3,LOW);
    digitalWrite(led4,HIGH);
    delay(100); 
    digitalWrite(led4,LOW);
    
  //HTTP Request
  if(userRFID == redCard){
    String light = "red";
    sendData(light, led1);
  }
  if(userRFID == greenCard){
    String light = "green";
    sendData(light, led2);
  }
  if(userRFID == blueCard){
    String light = "blue";
    sendData(light, led3);
  }
  if(userRFID == yellowCard){
    String light = "yellow";
    sendData(light, led4);
  }

  
  delay(400);
  userRFID = "";
  }
}
