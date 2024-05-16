#include "Wire.h"
#include "PN532_I2C.h"
#include "PN532.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "Bukan WiFi Gratis";
const char* password = "langsungconnect";
String idDevice = "habito_001";


String url = "https://api.habito.id";

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

int led1 = D6; //red
int led2 = D7; //yellow
int led3 = D5; //green
int led4 = D8; //blue

//int led1 = D5; //red
//int led2 = D6; //yellow
//int led3 = D7; //green
//int led4 = D8; //blue

String userRFID = "";

const int BUTTON_PIN = D3;
const int SHORT_PRESS_TIME = 1000; 

// Variables will change:
int lastState = LOW;
int currentState;
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

int buton = D3;


void setup() {
  Serial.begin(9600);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(led4,OUTPUT);
  pinMode(buton,INPUT);
  
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Non-blocking procedure
  nfc.setPassiveActivationRetries(0x01);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");
//  connectWiFi();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    int i=0;
    while(WiFi.status() != WL_CONNECTED){ 
      Serial.print(".");
      digitalWrite(led3,LOW);
      digitalWrite(led1,HIGH);
      delay(200);
      digitalWrite(led1,LOW);
      digitalWrite(led2,HIGH);
      delay(200);
      digitalWrite(led2,LOW);
      digitalWrite(led4,HIGH);
      delay(200);
      digitalWrite(led4,LOW);
      digitalWrite(led3,HIGH);
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
void online() {
  char ip[16]; // Menyimpan alamat IP sebagai string
  WiFi.localIP().toString().toCharArray(ip, 16); // Mendapatkan alamat IP dan mengonversinya menjadi string
  
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = idDevice;
  jsonDoc["ip"] = ip;
  jsonDoc["ssid"] = ssid;

  char jsonString[200]; // Buffer untuk menyimpan string JSON
  serializeJson(jsonDoc, jsonString, sizeof(jsonString)); // Serialisasi JSON

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String endpoint = "/keep-online";

  String fullUrl = url + endpoint;

  Serial.print("Requesting: ");
  Serial.println(fullUrl);

  if (https.begin(client, fullUrl)) {
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

void sendData(String light, int led) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
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
    http.begin(client,fullUrl);  //Specify request destination
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

void lightStatus() {
  
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = idDevice;

  char jsonString[200]; // Buffer untuk menyimpan string JSON
  serializeJson(jsonDoc, jsonString, sizeof(jsonString)); // Serialisasi JSON

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String endpoint = "/light-status";

  String fullUrl = url + endpoint;

  Serial.print("Requesting: ");
  Serial.println(fullUrl);

  if (https.begin(client, fullUrl)) {
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
          digitalWrite(led3,HIGH);
        }
        if(blueStatus == "true"){
          digitalWrite(led4,HIGH);
        }
        if(yellowStatus == "true"){
          digitalWrite(led2,HIGH);
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

void loop() {
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
    
      uint8_t success;
      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
      uint8_t uidLength;
    
      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
      if (success) {
        // Kartu terdeteksi
        Serial.println("Kartu Terdeteksi");
    
        // Dapatkan UID kartu
        String cardUID = "";
        for (uint8_t i = 0; i < uidLength; i++) {
          cardUID += String(uid[i], HEX);
        }
    
        Serial.print("UID Kartu: ");
        Serial.println(cardUID);

        digitalWrite(led1,HIGH);
        delay(100);
        digitalWrite(led1,LOW);
        digitalWrite(led2,HIGH);
        delay(100);
        digitalWrite(led2,LOW);
        digitalWrite(led4,HIGH);
        delay(100);
        digitalWrite(led4,LOW);
        digitalWrite(led3,HIGH);
        delay(100); 
        digitalWrite(led3,LOW);
      
        if(cardUID == "bece816e"){
          String light = "red";
          sendData(light, led1);
        }
        if(cardUID == "6ef18b6e"){
          String light = "green";
          sendData(light, led3);
        }
        if(cardUID == "4e637f6e"){
          String light = "blue";
          sendData(light, led4);
        }
        if(cardUID == "bee0806e"){
          String light = "yellow";
          sendData(light, led2);
        }
    
        delay(500); // Delay sebelum membaca kartu berikutnya
  }
}
