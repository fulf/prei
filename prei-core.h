#ifndef PREI_CORE_H
#define PREI_CORE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "prei-boards.h"
#include <map>

class PREiCore
{
  private:
    uint8_t _pinCount;
    PREiPin* _pins;
    uint8_t findIndexByPin(PREiPin);
    uint16_t clamp(uint16_t, uint16_t, uint16_t);
  public:
    enum {
      WEMOSD1miniPro,
    };

    PREiCore(uint8_t);
    PREiPin* getPins();
    PREiPin getPin(uint8_t);
    PREiPin getPin(String);
    uint8_t getPinCount();
    void setPinMode(PREiPin, const char*);
    void setPinOutput(PREiPin, uint16_t);
    String getWiFiEncryptionType(uint8_t);
    uint8_t connectWiFi(const char*, const char* = "");
    uint8_t getWiFiStatus();
    void disconnectWiFi();
};

#endif
