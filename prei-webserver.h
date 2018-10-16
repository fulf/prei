#ifndef PREI_WEBSERVER_H
#define PREI_WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "prei-core.h"
#include <FS.h>
#include <map>

class PREiWebServer
{
  private:
    ESP8266WebServer _web_server;
    PREiCore *_core;
    void handleRoot(),
      handleNotFound(),
      setPreflightHeaders();

  public:
    PREiWebServer(PREiCore*);

    typedef std::function<void(void)> THandlerFunction;

    const char* getArgument(const char*);

    void send(uint8_t code, const char *content_type=NULL, const String &content=String("")),
      sendJSON(String, uint16_t = 200, bool = false),
      handleClient(),
      addGet(const char*, THandlerFunction),
      addPost(const char*, THandlerFunction),
      addPut(const char*, THandlerFunction),
      addDelete(const char*, THandlerFunction),
      addOptions(const char*, THandlerFunction),
      addSensor(String name, int *val),
      addSensor(String name, float *val),
      addSensor(String name, String *val),
      addSensor(String name, int (*cb)()),
      addSensor(String name, float (*cb)()),
      addSensor(String name, String (*cb)());
};

#endif
