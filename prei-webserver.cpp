#include "prei-webserver.h"

PREiWebServer::PREiWebServer(PREiCore* lbc) {
  _core = lbc;

  _web_server.on("/", HTTP_GET, std::bind(&PREiWebServer::handleRoot, this));
  _web_server.onNotFound(std::bind(&PREiWebServer::handleNotFound, this));

  _web_server.begin();
}

const char* PREiWebServer::getArgument(const char * name) {
  return _web_server.arg(name).c_str();
}

void PREiWebServer::sendJSON(String message, uint16_t code, bool raw) {
  String success = (code/100) == 2 ? "true" : "false";

  if (!raw) {
    message = "\"" + message + "\"";
  }

  send(code, "application/json", "{\"success\":" + success + ",\"message\":" + message + "}");
}

void PREiWebServer::addGet(const char * route, PREiWebServer::THandlerFunction cb) {
  _web_server.on(route, HTTP_GET, cb);
}

void PREiWebServer::addPost(const char * route, PREiWebServer::THandlerFunction cb) {
  _web_server.on(route, HTTP_POST, cb);
}

void PREiWebServer::addPut(const char * route, PREiWebServer::THandlerFunction cb) {
  _web_server.on(route, HTTP_PUT, cb);
}

void PREiWebServer::addDelete(const char * route, PREiWebServer::THandlerFunction cb) {
  _web_server.on(route, HTTP_DELETE, cb);
}

void PREiWebServer::addOptions(const char * route, PREiWebServer::THandlerFunction cb) {
  _web_server.on(route, HTTP_OPTIONS, cb);
}

void PREiWebServer::addSensor(String name, int *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(*val));
  });
}

void PREiWebServer::addSensor(String name, float *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(*val));
  });
}

void PREiWebServer::addSensor(String name, String *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(*val));
  });
}

void PREiWebServer::addSensor(String name, int (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(cb()));
  });
}

void PREiWebServer::addSensor(String name, float (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(cb()));
  });
}

void PREiWebServer::addSensor(String name, String (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    sendJSON(String(cb()));
  });
}

void PREiWebServer::handleClient() {
  _web_server.handleClient();
}

void PREiWebServer::handleNotFound() {
  sendJSON("The requested page could not be found", 404);
}

void PREiWebServer::handleRoot() {
  SPIFFS.begin();
  File f = SPIFFS.open("/index.html", "r");

  if(!f) {
    sendJSON("Welcome to the root page. No portal configured.", 200);    
  } else {
    _web_server.streamFile(f, "text/html");
  }
  
  SPIFFS.end();
}

void PREiWebServer::send(uint8_t code, const char *content_type, const String &content) {
  _web_server.sendHeader("Access-Control-Max-Age", "10000");
  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  _web_server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  _web_server.send(code, content_type, content);
}
