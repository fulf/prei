#include "prei.h"

PREi::PREi(uint8_t board) {
  Serial.begin(115200);

  _core = new PREiCore(board);
  _web_server = new PREiWebServer(_core);

  _api = new PREiApi(_core, _web_server);

  _dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  _dns_server.start(53, "*", WiFi.softAPIP());

  _poller = NULL;
}

void PREi::run() {
  _dns_server.processNextRequest();
  _web_server->handleClient();

  if (_poller) {
    _poller->poll();
  }
  yield();
}

void PREi::setWiFi(const char* ssid, const char* password = "") {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
}

void PREi::attachPoller(PREiPoller* _poller) {
  _poller = _poller;
}