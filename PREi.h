#ifndef RESP_H
#define RESP_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
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
      _ap_hostname,
      _ap_password,
      _version;
    unsigned long _boot_timestamp;
    String generateLinksJSON(String path),
      generateInfoJSON(),
      generateScanJSON();
    void sendJSON(int code, String message);

  public:
    PREi();
    void run();
};

#endif
