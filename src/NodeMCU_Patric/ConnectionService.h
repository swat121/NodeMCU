#ifndef CONNECTION_SERVICE_H
#define CONNECTION_SERVICE_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include "ESP8266WebServer.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

class ConnectionService {
public:
  ConnectionService();
  void runMDNS(const String& name);
  void publishMessage(const String& topic, const String& message);
  void startConnectMqtt();
  void mqttLoop();

private:
  void setUpSetting();
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
  void reconnectMqtt();
};

#endif