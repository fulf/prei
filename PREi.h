#ifndef PREI_H
#define PREI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include "prei-webserver.h"
#include "prei-poller.h"
#include "prei-api.h"
#include <map>

class PREi
{
  private:
    PREiCore* _core;
    PREiWebServer* _web_server;
    PREiPoller* _poller;
    PREiApi* _api;
    DNSServer _dns_server;
    std::map<const char*, PREiPin> _pins;
  public:
    enum {
      WEMOSD1miniPro,
    };
    PREi(uint8_t);
    void setWiFi(const char*, const char*);
    void attachPoller(PREiPoller*);
    void run();
};

#endif
