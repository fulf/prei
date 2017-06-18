#ifndef RESP_H
#define RESP_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <PREiNTP.h>

#define VERSION "1.0.0"

class PREi
{
  private:
    PREiNTP _prei_ntp;
    ESP8266WebServer _web_server;
    DNSServer _dns_server;
    String _esp_hostname,
      _ap_password,
      _version;
    unsigned long _boot_timestamp;
    String generateLinksJSON(String path),
      generateInfoJSON(),
      generateScanJSON();
    void init(),
      sendJSON(int code, String message, bool raw),
      // Route /esp GET/POST/DELETE
      handleInfo(),
      handleUpdate(),
      handleRestart(),
      // Route /wifi GET/POST/DELETE
      handleScan(),
      handleConnect(),
      handleDisconnect();

  public:
    PREi();
    PREi(String hostname);
    PREi(String hostname, String password);
    void run();
};

#endif
