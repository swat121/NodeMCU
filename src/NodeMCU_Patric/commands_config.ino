//-----------------------------------------------------------------------------------------------------
void setCommands() {
  Serial.println();
  Serial.println("==================SET-COMMAND===============");
  Serial.println(WifiMode);
  if (WifiMode == "STA") {
    server.on("/api/v1/help", HTTP_GET, getHelp);
    server.on(UriBraces("/api/v1/switchers/relay/{}"), HTTP_PUT, relayHandle);
    server.on("/api/v1/power-module", HTTP_PUT, putPowerModule);
    server.on(UriBraces("/api/v1/temperature/{}"), HTTP_GET, getDataTemp);
    server.on("/api/v1/light", HTTP_GET, getLight);
    server.on("/api/v1/message", HTTP_PUT, putMessage);
    server.on("/api/v1/status", HTTP_GET, getStatus);
    server.on("/api/v1/configuration", HTTP_GET, getConfig);
  }
  if (WifiMode == "AP") {
    server.on("/", HTTP_GET, handleMainHtmlPage);
    server.on("/submit", HTTP_POST, handleFormSubmit);
  }
}

//-----------------------------------------------------------------------------------------------------

void getConfig() {
  String doc = createBoardConfigObject();
 
  //sendMessage(doc, 200);
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void getHelp() {
  StaticJsonDocument<bodySize> doc;;
  JsonObject help = doc.createNestedObject("help");
  JsonObject status = help.createNestedObject("status");

  status["method"] = "GET";
  status["endpoint"] = "/api/v1/status";

  JsonObject sensor = help.createNestedObject("sensor");

  JsonObject temp = sensor.createNestedObject("temp");

  temp["method"] = "GET";
  temp["endpoint"] = "/api/v1/temperature/{id}";

  JsonObject light = sensor.createNestedObject("light");

  light["method"] = "GET";
  light["endpoint"] = "/api/v1/light";


  JsonObject switcher = help.createNestedObject("switcher");

  JsonObject relay = switcher.createNestedObject("relay");

  relay["method"] = "PUT";
  relay["endpoint"] = "/api/v1/relays/{id}";

  JsonObject powerModule = switcher.createNestedObject("power-module");

  powerModule["method"] = "PUT";
  powerModule["endpoint"] = "/api/v1/powerModule";

  sendMessage(doc, 200);
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void getStatus() {
  StaticJsonDocument<bodySize> doc;
  doc["relay1"] = String(digitalRead(PIN_Relay1));
  doc["relay2"] = String(digitalRead(PIN_Relay2));
  doc["relay3"] = String(digitalRead(PIN_Relay3));
  doc["powerModule"] = String(digitalRead(PIN_Power_Module));
  sendMessage(doc, 200);
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void relayHandle() {
  int idParam = server.pathArg(0).toInt();
  switch (idParam) {
    case 1: relay1(); break;
    case 2: relay2(); break;
    case 3: relay3(); break;
    default: sendMessage("Error", "Relay module not found!", 404);
  }
}

void relay1() {
  Relay1 = !Relay1;
  digitalWrite(PIN_Relay1, Relay1);
  sendMessage("relay1", String(digitalRead(PIN_Relay1)));
  ledBlink(1, 100);
}

void relay2() {
  Relay2 = !Relay2;
  digitalWrite(PIN_Relay2, Relay2);
  delay(500);
  Relay2 = !Relay2;
  digitalWrite(PIN_Relay2, Relay2);
  sendMessage("relay2", "OPEN");
  ledBlink(1, 100);
}

void relay3() {
  Relay3 = !Relay3;
  digitalWrite(PIN_Relay3, Relay3);
  sendMessage("relay3", String(digitalRead(PIN_Relay3)));
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void putPowerModule() {
  PowerStatus = !PowerStatus;
  if (PowerStatus == true) {
    for (int i = 0; i < 255; i++) {
      analogWrite(PIN_Power_Module, i);
      delay(10);
    }
    digitalWrite(PIN_Power_Module, HIGH);
  }
  if (PowerStatus == false) {
    for (int i = 255; i > -1; i--) {
      analogWrite(PIN_Power_Module, i);
      delay(10);
    }
  }
  sendMessage("PowerStatus", String(digitalRead(PIN_Power_Module)));
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void getDataTemp() {
  int id = server.pathArg(0).toInt();
  float responce = getTemperatureByIndex(id);

  if (responce == -127.00) {
    sendMessage("temp", "Temperature device not found!", 404);
  } else {
    sendMessage("temp", String(responce));
  }
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void getLight() {
  sendMessage("backlight", "not yet");
  ledBlink(1, 100);
}

//-----------------------------------------------------------------------------------------------------

void putMessage() {
  sendMessage("message", "not yet");
  ledBlink(1, 100);
}
//-----------------------------------------------------------------------------------------------------

void sendMessage(String key, String value) {
  StaticJsonDocument<200> doc;
  String s;
  doc["name"] = data.name;
  doc[key] = value;
  serializeJson(doc, s);
  server.send(200, "application/json", s);
}

void sendMessage(String key, String value, int statusCode) {
  StaticJsonDocument<200> doc;
  String s;
  doc["name"] = data.name;
  doc[key] = value;
  serializeJson(doc, s);
  server.send(statusCode, "application/json", s);
}

void sendMessage(StaticJsonDocument<bodySize> doc, int statusCode) {
  String s;
  doc["name"] = data.name;
  serializeJson(doc, s);
  server.send(statusCode, "application/json", s);
}
//-----------------------------------------------------------------------------------------------------

float getTemperatureByIndex(int idParam) {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(idParam);
}
