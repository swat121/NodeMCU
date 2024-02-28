#ifndef CONNECTION_SERVICE_H
#define CONNECTION_SERVICE_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include "ESP8266WebServer.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>

class ConnectionService {
public:
  ConnectionService();
  void connectToServer(const String& name);

private:
  void setUpSetting();
};

#endif