#ifndef RESP_H
#define RESP_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <FS.h>
#include <PREiNTP.h>

#define VERSION "1.0.0"

struct PREiPin {
    String name;
    uint8_t pin;
    bool digital;
    String mode;
    uint8_t pwm_val;
};

class PREi
{
  private:
    PREiNTP _prei_ntp;
    ESP8266WebServer _web_server;
    DNSServer _dns_server;
    String _esp_hostname,
      _ap_password,
      _version;
    PREiPin pins[10];
    unsigned long _boot_timestamp;
    uint8_t getPinFromUri(String uri);
    String generateLinksJSON(String path),
      generateInfoJSON(),
      generateScanJSON(),
      generatePinJSON(PREiPin pin);
    bool redirectToHost(bool permanent);
    void init(),
      initPins(),
      sendJSON(int code, String message, bool raw=false),
      // PREi config portal
      handlePortal(),
      // Route /esp GET/POST/DELETE
      handleInfo(),
      handleUpdate(),
      handleRestart(),
      // Route /wifi GET/POST/DELETE
      handleScan(),
      handleConnect(),
      handleDisconnect(),
      // Route /pin/{id} GET/POST/PUT/DELETE
      handlePinInfo(),
      handlePinOn(),
      handlePinChange(),
      handlePinOff(),
      // Other routes handler
      handleNotFound(),
      handlePreflight();

  public:
    PREi();
    PREi(String hostname);
    PREi(String hostname, String password);
    void run(),
      addSensor(String name, int *val),
      addSensor(String name, float *val),
      addSensor(String name, String *val),
      addSensor(String name, int (*cb)()),
      addSensor(String name, float (*cb)()),
      addSensor(String name, String (*cb)());
    ESP8266WebServer* getServer();
    DNSServer* getDNS();
};

#endif
