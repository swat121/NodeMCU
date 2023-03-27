#include <ESP8266WiFi.h>
#include "ESP8266WebServer.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <EEPROM.h>

//#include <ESPForm.h>

//------------------------------------------------------------------------
#define AP_SSID "Patric"
#define AP_PASS "12345678"
#define STR_ADDR 32

//------------------------------------------------------------------------
IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

//------------------------------------------------------------------------
unsigned long prevMillis = 0;
unsigned long serverTimeout = 2 * 60 * 1000;
//------------------------------------------------------------------------
boolean status = true;
String ssid;
String pass;
//------------------------------------------------------------------------
unsigned long timer;
boolean stat = true;
#define PIN_LED_Good 2    //D4
#define PIN_LED_Error 14  //D5
//------------------------------------------------------------------------
#define PIN_Relay1 12  //D6
boolean Relay1 = false;

#define PIN_Relay2 13  //D7
boolean Relay2 = false;

#define PIN_Relay3 15  //D8
boolean Relay3 = false;

#define PIN_Light 4  //D2
boolean Light = false;

//------------------------------------------------------------------------

boolean backlightStat = false;
boolean connectStat = false;

//------------------------------------------------------------------------
ESP8266WebServer server(80);
// const char* ssid = "Parents";
// const char* password = "Drim1932";
//------------------------------------------------------------------------

const char* serverName = "http://192.168.0.102:8080/bot/alarm";
String sensorReadings;

//------------------------------------------------------------------------
#define ONE_WIRE_BUS 0  //D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperature;
unsigned long timerTemp;
//------------------------------------------------------------------------

String WifiMode;

void setup() {

  Serial.begin(115200);
  delay(100);
  //------------------------------------------------------------------------
  sensors.begin();
  //------------------------------------------------------------------------

  pinMode(PIN_LED_Good, OUTPUT);
  pinMode(PIN_LED_Error, OUTPUT);

  pinMode(PIN_Relay1, OUTPUT);
  digitalWrite(PIN_Relay1, LOW);

  pinMode(PIN_Relay2, OUTPUT);
  digitalWrite(PIN_Relay2, LOW);

  pinMode(PIN_Relay3, OUTPUT);
  digitalWrite(PIN_Relay3, LOW);

  pinMode(PIN_Light, OUTPUT);
  analogWrite(PIN_Light, 0);

  //---------------------------------------------------------------------------------------------------
  readFromEEPROM();
  //---------------------------------------------------------------------------------------------------
  if (status) {
    wifiModeSTA(ssid, pass);
    //server.begin();  //Запускаем сервер
    //ESPForm.server().begin();
    Serial.println("Server listening");
    WifiMode = "STA";
  } else {
    wifiModeAP();
    server.begin();
    WifiMode = "AP";
  }
  //-----------------------------------------------------------------------------------------------------
}

//-----------------------------------LOOP--------------------------------------------------------------
void loop() {
  if (WifiMode == "STA") {
    //ESPForm.handleWebClient();
    checkConnect();
  }

  if (WifiMode == "AP") {
    // If a client existed
    // if (ESPForm.getClientCount() > 0) {
    //   if (millis() - prevMillis > 1000) {
    //     prevMillis = millis();
    //     //The event listener for text2 is not set because we don't want to listen to its value changes
    //     ESPForm.setElementContent("ssid", ssid);
    //     ESPForm.setElementContent("pass", pass);
    //   }
    // }
    server.handleClient();
  }
}

//-----------------------------------------------------------------------------------------------------

void checkConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    ledDisconnect();
    connectStat = true;
  } else {
    if (connectStat == true) {
      ledBlink(3, 100);
      connectStat = false;
      digitalWrite(PIN_LED_Error, LOW);
    }
  }
}

//-----------------------------------------------------------------------------------------------------

void ledBlink(int count, int microsecond) {
  for (int i = 0; i < count; i++) {
    digitalWrite(PIN_LED_Good, HIGH);
    delay(microsecond);
    digitalWrite(PIN_LED_Good, LOW);
    delay(microsecond);
  }
}

//-----------------------------------------------------------------------------------------------------

void ledDisconnect() {
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(PIN_LED_Error, stat);
    stat = !stat;
  }
}
