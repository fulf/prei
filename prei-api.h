#ifndef PREI_API_H
#define PREI_API_H

#include <ESP8266httpUpdate.h>
#include "prei-webserver.h"
#include "prei-core.h"

class PREiApi {
  private:
    PREiWebServer *_web_server;
    PREiCore *_core;
    void setPinHandlers(),
    handlePinGet(uint8_t),
    handlePinPost(uint8_t),
    handlePinPut(uint8_t),
    handlePinDelete(uint8_t),
    handlePinOptions(uint8_t),

    setWiFiHandlers(),
    handleWiFiGet(),
    handleWiFiPost(),
    handleWiFiDelete(),

    setChipHandlers(),
    handleInfo(),
    handleUpdate(),
    handleRestart();
  public:
    PREiApi(PREiCore*, PREiWebServer*);
};

#endif
