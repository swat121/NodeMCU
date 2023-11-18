#include "ESP8266WebServer.h"
#include <ESP8266HTTPClient.h>

// My classes
#include "WiFiManager.h"
#include "MemoryService.h"
#include "ConnectionService.h"
#include "ClientData.h"

#include <uri/UriBraces.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <EEPROM.h>

#include "GyverButton.h"

// Button variables
#define BTN_PIN 5
GButton switchModeButton(BTN_PIN);

//------------------------------------------------------------------------

ClientData data;

//------------------------------------------------------------------------
#define AP_SSID data.name
#define AP_PASS "12345678"

//------------------------------------------------------------------------


//------------------------------------------------------------------------

String pass;
String WifiMode;

const String WIFI_MODE_STA = "STA";
const String WIFI_MODE_AP = "AP";
//------------------------------------------------------------------------

unsigned long timer;
boolean stat = true;
#define PIN_LED_Good 2    //D4
#define PIN_LED_Error 14  //D5
//------------------------------------------------------------------------
int PIN_Relay1 = 12;  //D6
boolean Relay1 = false;

int PIN_Relay2 = 13;  //D7
boolean Relay2 = false;

int PIN_Relay3 = 15;  //D8
boolean Relay3 = false;

int PIN_Power_Module = 4;  //D2
boolean PowerStatus = false;

//------------------------------------------------------------------------

boolean flagForCheckConnect = false;
boolean flagIsConnectToServer = true;

//------------------------------------------------------------------------
ESP8266WebServer server(80);

WiFiManager wifiManager(server);
MemoryService memoryService;
ConnectionService connectionService;

//------------------------------------------------------------------------

//------------------------------------------------------------------------
int ONE_WIRE_BUS = 0;  //D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long timerTemp;
//------------------------------------------------------------------------


#define bodySize 1024


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
  pass = credentials.password;
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
  String boardData = createBoardDataJson();
  String clientData = createClientDataJson();

  WifiMode = WIFI_MODE_STA;
  wifiManager.wifiModeSTA(data.ssid, pass);
  new (&connectionService) ConnectionService(clientData, boardData);

  if (WiFi.status() == WL_CONNECTED) {
    String ip_parts[4];
    data.ip = WiFi.localIP().toString();
    data.mac = WiFi.macAddress();
    splitString(data.ip, ip_parts);
    connectionService.connectToServer(ip_parts, 700);
    ledBlink(3, 100);
  }
}

void handleAPConnection() {
  WifiMode = WIFI_MODE_AP;
  wifiManager.wifiModeAP(AP_SSID, AP_PASS);
}

//-----------------------------------LOOP--------------------------------------------------------------
void loop() {
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
    digitalWrite(PIN_LED_Error, stat);
    stat = !stat;
  }
}

String createBoardDataJson() {
  // Before change doc, you mus change doc size (check optimize doc size here https://arduinojson.org/v6/assistant/#/step1)
  StaticJsonDocument<512> doc;
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

  serializeJson(doc, payload);
  return payload;
}

String createClientDataJson() {
  // Before change doc, you mus change doc size (check optimize doc size here https://arduinojson.org/v6/assistant/#/step1)
  StaticJsonDocument<128> doc;

  doc["name"] = data.name;
  doc["ip"] = data.ip;
  doc["mac"] = data.mac;
  doc["ssid"] = data.ssid;

  String payload;
  serializeJson(doc, payload);
  return payload;
}