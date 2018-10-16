#ifndef PREI_POLLER_H
#define PREI_POLLER_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "prei-actor.h"

class PREiPoller {
  private:
    HTTPClient _http_client;
    PREiActor* _actor;
    const char* _call_sign;
    const char* _url;
  public:
    PREiPoller(const char*, const char*, PREiActor*);
    void poll();
};

#endif
