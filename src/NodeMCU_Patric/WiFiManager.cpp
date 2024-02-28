#include "WString.h"
#include "WiFiManager.h"

IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

WiFiManager::WiFiManager(ESP8266WebServer& server)
  : server(server) {
}

void WiFiManager::wifiModeSTA(const String& ssid, const String& password) {
  Serial.println();
  Serial.println("======================Wifi-Mode-STA===========================");
  byte tries = 15;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (--tries && WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Non Connecting to WiFi..");
  } else {
    Serial.println("Connected to WiFi..");
  }
}

//=======================================================================================================================

void WiFiManager::wifiModeAP(const String& ssid, const String& password) {
  Serial.println();
  Serial.println("======================Wifi-Mode-AP===========================");

  Serial.print("Setting soft-AP configuration ... ");

  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.print(ssid);
  Serial.print(" | ");
  Serial.println(password);

  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");

  Serial.println(WiFi.softAPIP());
}

//=======================================================================================================================

void WiFiManager::changeWifiMode(const String& mode) {
  Serial.println("Change mode to " + mode);

  memoryService.startEEPROMSession(1024);
  memoryService.writeStatus(mode == "STA" ? 1 : 0);
  memoryService.endEEPROMSession();

  Serial.println("Reset..");
  ESP.restart();
}