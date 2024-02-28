#include "ESP8266WebServer.h"

// My classes
#include "WiFiManager.h"
#include "MemoryService.h"
#include "ConnectionService.h"
#include "ClientData.h"
#include <ESP8266mDNS.h>

#include <uri/UriBraces.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "GyverButton.h"

// Button variables
#define BTN_PIN 5  // D0
GButton switchModeButton(BTN_PIN);

//------------------------------------------------------------------------

ClientData data;

//------------------------------------------------------------------------
#define AP_SSID data.name
#define AP_PASS data.apPass

//------------------------------------------------------------------------

String WifiMode;

#define WIFI_MODE_STA "STA"
#define WIFI_MODE_AP "AP"
//------------------------------------------------------------------------

unsigned long timer;
boolean disconnectLedStatus = true;
#define PIN_LED_Good 2    //D4
#define PIN_LED_Error 14  //D5
//------------------------------------------------------------------------
#define PIN_Relay1 12  //D6
boolean Relay1 = false;

#define PIN_Relay2 13  //D7
boolean Relay2 = false;

#define PIN_Relay3 15  //D8
boolean Relay3 = false;

#define PIN_Power_Module 4  //D2
boolean PowerStatus = false;

//------------------------------------------------------------------------

ESP8266WebServer server(80);

WiFiManager wifiManager(server);
MemoryService memoryService;
ConnectionService connectionService;

//------------------------------------------------------------------------
#define ONE_WIRE_BUS 0  //D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long timerTemp;
//------------------------------------------------------------------------


void setup() {

  Serial.begin(115200);
  delay(100);

  switchModeButton.setTimeout(3000);
  switchModeButton.setType(HIGH_PULL);

  switchModeButton.setDirection(NORM_OPEN);

  Serial.println();
  Serial.println("Branch: feature");

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

  pinMode(PIN_Power_Module, OUTPUT);
  analogWrite(PIN_Power_Module, 0);

  //---------------------------------------------------------------------------------------------------

  MemoryCredentials credentials = memoryService.readSsidAndPass();
  data.ssid = credentials.ssid;
  data.staPass = credentials.password;
  boolean wifiModeStatus = credentials.status;

  //---------------------------------------------------------------------------------------------------

  setupWifiMode(wifiModeStatus);
  //-----------------------------------------------------------------------------------------------------
}

void setupWifiMode(boolean& status) {
  if (status) {
    handleSTAConnection();
  } else {
    handleAPConnection();
  }

  setCommands();
  server.begin();
}

void handleSTAConnection() {
  WifiMode = WIFI_MODE_STA;
  String mdnsName = "esp-plug-" + data.name;
  wifiManager.wifiModeSTA(data.ssid, data.staPass);

  if (WiFi.status() == WL_CONNECTED) {
    data.ip = WiFi.localIP().toString();
    data.mac = WiFi.macAddress();

    String boardData = createBoardDataJson();
    String clientData = createClientDataJson();
    ConnectionService().connectToServer(mdnsName);

    ledBlink(3, 100);
  }
}

void handleAPConnection() {
  WifiMode = WIFI_MODE_AP;
  wifiManager.wifiModeAP(AP_SSID, AP_PASS);
}

//-----------------------------------LOOP--------------------------------------------------------------
void loop() {
  MDNS.update();
  switchModeButton.tick();
  if (switchModeButton.isHolded()) {
    Serial.println("Button is holding");

    digitalWrite(PIN_LED_Error, LOW);
    digitalWrite(PIN_LED_Good, LOW);
    ledBlink(3, 100);
    wifiManager.changeWifiMode(WifiMode == WIFI_MODE_STA ? WIFI_MODE_AP : WIFI_MODE_STA);
  }

  if (WifiMode == WIFI_MODE_STA && WiFi.status() != WL_CONNECTED) {
    ledDisconnect();
    return;
  }

  if (WifiMode == WIFI_MODE_STA) {
    digitalWrite(PIN_LED_Error, LOW);
  }

  server.handleClient();
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
    Serial.println("Wifi not connected");
    timer = millis();
    digitalWrite(PIN_LED_Error, disconnectLedStatus);
    disconnectLedStatus = !disconnectLedStatus;
  }
}

String createBoardDataJson() {
  // Before change doc, you mus change doc size (check optimize doc size here https://arduinojson.org/v6/assistant/#/step1)
  StaticJsonDocument<768> doc;
  String payload;
  doc["name"] = data.name;

  JsonObject setting = doc.createNestedObject("setting");


  // sensors
  JsonArray sensors = setting.createNestedArray("sensors");
  JsonObject temperature = sensors.createNestedObject();
  temperature["moduleName"] = "temperature";
  JsonArray temperatureData = temperature.createNestedArray("data");
  JsonObject tempData1 = temperatureData.createNestedObject();
  tempData1["moduleId"] = "1";
  tempData1["pin"] = ONE_WIRE_BUS;


  // switchers
  JsonArray switchers = setting.createNestedArray("switchers");
  JsonObject relay = switchers.createNestedObject();

  relay["moduleName"] = "relay";
  JsonArray relayData = relay.createNestedArray("data");

  JsonObject relayData1 = relayData.createNestedObject();
  relayData1["moduleId"] = "1";
  relayData1["pin"] = PIN_Relay1;

  JsonObject relayData2 = relayData.createNestedObject();
  relayData2["moduleId"] = "2";
  relayData2["pin"] = PIN_Relay2;

  JsonObject relayData3 = relayData.createNestedObject();
  relayData3["moduleId"] = "3";
  relayData3["pin"] = PIN_Relay3;

  JsonObject power_module = switchers.createNestedObject();
  power_module["moduleName"] = "power-module";
  JsonArray powerData = power_module.createNestedArray("data");

  JsonObject powerData1 = powerData.createNestedObject();
  powerData1["moduleId"] = "1";
  powerData1["pin"] = PIN_Power_Module;



  serializeJson(doc, payload);
  return payload;
}

String createClientDataJson() {
  // Before change doc, you mus change doc size (check optimize doc size here https://arduinojson.org/v6/assistant/#/step1)
  StaticJsonDocument<256> doc;

  doc["name"] = data.name;
  doc["ip"] = data.ip;
  doc["mac"] = data.mac;
  doc["ssid"] = data.ssid;

  String payload;
  serializeJson(doc, payload);

  Serial.print("Payload client: ");
  Serial.println(payload);
  return payload;
}