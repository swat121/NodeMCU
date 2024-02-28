#include "MemoryService.h"

MemoryService::MemoryService() {
}

void MemoryService::writeSsidAndPass(const String& ssid, const String& password) {
  int ssidLength = ssid.length();
  int passLength = password.length();

  Serial.println(F("Writing EEPROM ssid:"));
  writeStatus('1');

  for (int i = 1; i <= ssidLength; ++i) {
    EEPROM.write(i, ssid[i - 1]);
    Serial.print(F("Wrote: "));
    Serial.println(ssid[i - 1]);
  }

  int delimiterPosition = ssidLength + 1;
  EEPROM.write(delimiterPosition, '|');
  Serial.println(F("Wrote: |"));

  for (int i = 0; i < passLength; ++i) {
    EEPROM.write(delimiterPosition + 1 + i, password[i]);
    Serial.print(F("Wrote: "));
    Serial.println(password[i]);
  }

  int endPosition = delimiterPosition + 1 + passLength;
  EEPROM.write(endPosition, '&');
  Serial.print(F("Wrote: "));
  Serial.println('&');
}

//=======================================================================================================================

void MemoryService::writeStatus(uint8_t status) {

  EEPROM.write(0, status);

  Serial.print(F("Wrote status: "));
  Serial.println(status);
}

//=======================================================================================================================

void MemoryService::startEEPROMSession(int size) {
  Serial.println(F("====================== START-EEPROM-Session (Write) ==========================="));

  EEPROM.begin(size);
}

//=======================================================================================================================

void MemoryService::endEEPROMSession() {
  EEPROM.commit();
  EEPROM.end();
  Serial.println(F("====================== END-EEPROM-Session (Write) ==========================="));
}

MemoryCredentials MemoryService::readSsidAndPass() {
  Serial.println(F("======================Start-Read-From-EEPROM==========================="));
  MemoryCredentials credentials;

  EEPROM.begin(1024);

  boolean status = boolean(EEPROM.read(0));  // Чтение статуса
  Serial.print(F("Status from EEPROM: "));
  Serial.println(status);

  String ssid = "";
  String pass = "";
  char currentChar;
  int i = 1;  // Начинаем с 1, так как 0 байт занят статусом

  // Чтение SSID
  while ((currentChar = char(EEPROM.read(i))) != '|' && i <= 40) {
    ssid += currentChar;
    i++;
  }

  // Пропускаем разделитель и продолжаем чтение пароля
  i++;
  while ((currentChar = char(EEPROM.read(i))) != '&' && i <= 80) {
    pass += currentChar;
    i++;
  }

  EEPROM.end();

  Serial.print(F("SSID from EEPROM: "));
  Serial.println(ssid);
  Serial.print(F("Password from EEPROM: "));
  Serial.println(pass);

  credentials.ssid = ssid;
  credentials.password = pass;
  credentials.status = status;

  Serial.println(F("======================END-Read-To-EEPROM==========================="));

  return credentials;
}
