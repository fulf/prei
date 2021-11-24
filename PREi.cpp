#include "PREi.h"

PREi::PREi(String hostname, String password) {
  _esp_hostname = hostname;
  _ap_password = password;
  PREi::init();
}

PREi::PREi(String hostname) {
  _esp_hostname = hostname;
  PREi::init();
}

PREi::PREi() {
  _esp_hostname = "ESP_" + String(ESP.getChipId());

  PREi::init();
}

void PREi::run() {
  _dns_server.processNextRequest();
  _web_server.handleClient();
  yield();
}

void PREi::addSensor(String name, int *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(*val));
  });
}

void PREi::addSensor(String name, float *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(*val));
  });
}

void PREi::addSensor(String name, String *val) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(*val));
  });
}

void PREi::addSensor(String name, int (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(cb()));
  });
}

void PREi::addSensor(String name, float (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(cb()));
  });
}

void PREi::addSensor(String name, String (*cb)()) {
  _web_server.on(("/sensor/" + name).c_str(), HTTP_GET, [=](){
    PREi::sendJSON(200, String(cb()));
  });
}

ESP8266WebServer* PREi::getServer() {
  return &_web_server;
}

DNSServer* PREi::getDNS() {
  return &_dns_server;
}

void PREi::init() {
  initPins();
  WiFi.setAutoConnect(true);
  ESPhttpUpdate.rebootOnUpdate(false);

  WiFi.mode(WIFI_AP_STA);
  if(_ap_password.length() > 7 && _ap_password.length() < 64) {
    WiFi.softAP(_esp_hostname.c_str(), _ap_password.c_str());
  } else {
    WiFi.softAP(_esp_hostname.c_str());
  }

  _dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  _dns_server.start(53, "*", WiFi.softAPIP());

  MDNS.begin(_esp_hostname.c_str());

  _web_server.on("/", HTTP_GET, std::bind(&PREi::handlePortal, this));

  _web_server.on("/esp", HTTP_GET, std::bind(&PREi::handleInfo, this));
  _web_server.on("/esp", HTTP_POST, std::bind(&PREi::handleUpdate, this));
  _web_server.on("/esp", HTTP_DELETE, std::bind(&PREi::handleRestart, this));

  _web_server.on("/wifi", HTTP_GET, std::bind(&PREi::handleScan, this));
  _web_server.on("/wifi", HTTP_POST, std::bind(&PREi::handleConnect, this));
  _web_server.on("/wifi", HTTP_DELETE, std::bind(&PREi::handleDisconnect, this));

  for(int i=0; i<10; ++i) {
    digitalWrite(pins[i].pin, HIGH);
    pinMode(pins[i].pin, INPUT);
    _web_server.on(("/pin/" + pins[i].name).c_str(), HTTP_GET, std::bind(&PREi::handlePinInfo, this));
    _web_server.on(("/pin/" + pins[i].name).c_str(), HTTP_POST, std::bind(&PREi::handlePinOn, this));
    _web_server.on(("/pin/" + pins[i].name).c_str(), HTTP_PUT, std::bind(&PREi::handlePinChange, this));
    _web_server.on(("/pin/" + pins[i].name).c_str(), HTTP_DELETE, std::bind(&PREi::handlePinOff, this));
    _web_server.on(("/pin/" + pins[i].name).c_str(), HTTP_OPTIONS, std::bind(&PREi::handlePreflight, this));
  }

  _web_server.onNotFound(std::bind(&PREi::handleNotFound, this));

  _web_server.begin();
}

void PREi::sendJSON(int code, String message, bool raw) {
  String success = (code/100) == 2 ? "true" : "false";
  _web_server.send(code, "application/json", raw ? message : "{\"success\":" + success+ ",\"message\":\"" + message+ "\"}");
}

void PREi::handlePortal() {
  if(PREi::redirectToHost(true)) {
      return;
  }

  SPIFFS.begin();
  File f = SPIFFS.open("/index.html", "r");
  if(!f) {
    PREi::sendJSON(404, "Could not find portal page.");
  }
  _web_server.streamFile(f, "text/html");
  SPIFFS.end();
}

void PREi::handleInfo() {
  PREi::sendJSON(200, generateInfoJSON(), true);
}

void PREi::handleUpdate() {
  WiFiClient client;
  HTTPUpdateResult ret = ESPhttpUpdate.update(client, _web_server.arg("firmware"), VERSION);
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          PREi::sendJSON(500, "Update failed.");
          break;
      case HTTP_UPDATE_NO_UPDATES:
          PREi::sendJSON(304, "Update not necessary.");
          break;
      case HTTP_UPDATE_OK:
          PREi::sendJSON(200, "Update started.");
          ESP.restart();
          break;
  }
}

void PREi::handleRestart() {
  PREi::sendJSON(200, "Restart initiated succesfully!");
  ESP.restart();
}

void PREi::handleScan() {
  _web_server.send(200, "application/json", generateScanJSON());
}

void PREi::handleConnect() {
  const int MAX_TRIES = 30;
  int status;
  String ssid = _web_server.arg("ssid");
  String pass = _web_server.arg("pass");

  int respCode = 200;
  String respMessage;

  if(ssid.length() > 31 || ssid.length() == 0) {
    respCode = 400;
    respMessage = "Invalid SSID provided.";
  } else if((pass.length() > 0 && pass.length() < 8) || pass.length() > 63) {
    respCode = 400;
    respMessage = "Invalid password provided.";
  }

  if(respCode == 200) {
    WiFi.begin(ssid.c_str(), pass.c_str());

    for(int t=0; t<MAX_TRIES && (status = WiFi.status()) != WL_CONNECTED; ++t) {
      delay(200);
    }

    if(status != WL_CONNECTED) {
      respCode = 403;
      respMessage = "Invalid credentials provided.";
    } else {
      respCode = 200;
      respMessage = "Connected succesfully to provided WiFi.";
    }
  }

  PREi::sendJSON(respCode, respMessage);

  if(respCode/100 != 2) {
    WiFi.disconnect();
  }
}

void PREi::handleDisconnect() {
  PREi::sendJSON(200, "Disconnected succesfully!");
  WiFi.disconnect();
}

void PREi::handlePinInfo() {
  uint8_t pin = getPinFromUri(_web_server.uri());

  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  _web_server.send(200, "application/json", generatePinJSON(pins[pin]));
}

void PREi::handlePinOn() {
  uint8_t pin = getPinFromUri(_web_server.uri());
  String val;

  _web_server.sendHeader("Access-Control-Allow-Origin", "*");
  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  if(!pins[pin].digital || pins[pin].mode == "input") {
    PREi::sendJSON(405, "Pin mode only supports GET requests.");
  } else if(pins[pin].mode == "output") {
    digitalWrite(pins[pin].pin, HIGH);
    PREi::sendJSON(200, "Pin voltage set to HIGH.");
  } else if((val = _web_server.arg("val")) != "") {
    analogWrite(pins[pin].pin, atoi(val.c_str()));
    PREi::sendJSON(200, "Pin voltage set to " + val + ".");
    pins[pin].pwm_val = atoi(val.c_str());
  } else {
    analogWrite(pins[pin].pin, 255);
    PREi::sendJSON(200, "Pin voltage set to 255.");
    pins[pin].pwm_val = 255;
  }
}

void PREi::handlePinChange() {
  uint8_t pin = getPinFromUri(_web_server.uri());

  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  if(pins[pin].digital) {
    String mode = _web_server.arg("mode");

    if(mode == "input") {
        pinMode(pins[pin].pin, INPUT);
    } else if(mode == "output") {
        pinMode(pins[pin].pin, OUTPUT);
    } else if(mode == "pwm") {
        pinMode(pins[pin].pin, OUTPUT);
    } else {
        PREi::sendJSON(400, "Invalid pin mode received.");
        return;
    };
    pins[pin].mode = mode;
    PREi::sendJSON(200, "Pin mode succesfully set to " + mode + ".");
  } else {
    PREi::sendJSON(405, "Pin mode only supports GET requests.");
  }
}

void PREi::handlePinOff() {
  uint8_t pin = getPinFromUri(_web_server.uri());

  _web_server.sendHeader("Access-Control-Allow-Origin", "*");
  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  if(!pins[pin].digital || pins[pin].mode == "input") {
    PREi::sendJSON(405, "Pin mode only supports GET requests.");
  } else if(pins[pin].mode == "output") {
    digitalWrite(pins[pin].pin, LOW);
    PREi::sendJSON(200, "Pin voltage set to LOW.");
  } else {
    analogWrite(pins[pin].pin, 0);
    pins[pin].pwm_val = 0;
    PREi::sendJSON(200, "Pin voltage set to 0.");
  }
}

void PREi::handleNotFound() {
  if(PREi::redirectToHost(false)) {
    return;
  }

  String uri = _web_server.uri();
  String contentType = "text/plain";
  SPIFFS.begin();

  File f = SPIFFS.open(uri, "r");
  if(!f) {
    PREi::sendJSON(404, "Resource not found.");
    return;
  }

  if(uri.substring(uri.length() - 5) == ".html") {
    contentType = "text/html";
  } else if(uri.substring(uri.length() - 4) == ".css") {
    contentType = "text/css";
  } else if(uri.substring(uri.length() - 3) == ".js") {
    contentType = "text/javascript";
  }

  _web_server.streamFile(f, contentType);
  SPIFFS.end();
}

void PREi::handlePreflight() {
  _web_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");

  PREi::sendJSON(200, "Preflight headers sent.");
}

bool PREi::redirectToHost(bool permanent) {
  String localIP = _web_server.client().localIP().toString();

  if (_web_server.hostHeader() != localIP) {
    _web_server.sendHeader("Location", String("http://") + localIP, true);
    _web_server.send(permanent ? 308 : 302, "text/plain", "");
    _web_server.client().stop();
    return true;
  }

  return false;
}

String PREi::generateInfoJSON() {
  unsigned long unix = _prei_ntp.getUnix();

  if(_boot_timestamp == 0 && unix != 0) {
    _boot_timestamp = unix - (millis()/1000);
  }

  String JSON = "";
  JSON += "{\"data\":";
  JSON += "{\
    \"attributes\":{\
      \"chip_id\":{ci},\
      \"flash_chip_id\":{fci},\
      \"hostname\":\"{hn}\",\
      \"mdns\":\"{mdns}\",\
      \"ap_ssid\":\"{asi}\",\
      \"ap_ip\":\"{aip}\",\
      \"sta_ssid\":{ssi},\
      \"sta_ip\":{sip},\
      \"flash_chip_size\":{fcs},\
      \"flash_chip_real_size\":{fcrs},\
      \"boot_version\":\"{bv}\",\
      \"core_version\":\"{cv}\",\
      \"sdk_version\":\"{sv}\",\
      \"firmware_version\":\"{fv}\",\
      \"firmware_size\":{ss},\
      \"firmware_md5\":\"{smd5}\",\
      \"boot_timestamp\":{ut},\
      \"unix\":{ux}\
    }\
  }";

  JSON.replace("{ci}", String(ESP.getChipId()));
  JSON.replace("{fci}", String(ESP.getFlashChipId()));
  JSON.replace("{hn}", WiFi.hostname());
  JSON.replace("{mdns}", "http://" + String(_esp_hostname) + ".local/");
  JSON.replace("{asi}", _esp_hostname);
  JSON.replace("{aip}", WiFi.softAPIP().toString());
  JSON.replace("{ssi}", WiFi.status() == WL_CONNECTED ? '"' + WiFi.SSID() + '"' : "null");
  JSON.replace("{sip}", WiFi.status() == WL_CONNECTED ? '"' + WiFi.localIP().toString() + '"' : "null");
  JSON.replace("{fcs}", String(ESP.getFlashChipSize()));
  JSON.replace("{fcrs}", String(ESP.getFlashChipRealSize()));
  JSON.replace("{bv}", String(ESP.getBootVersion()));
  JSON.replace("{cv}", String(ESP.getCoreVersion()));
  JSON.replace("{sv}", String(ESP.getSdkVersion()));
  JSON.replace("{fv}", VERSION);
  JSON.replace("{ss}", String(ESP.getSketchSize()));
  JSON.replace("{smd5}", ESP.getSketchMD5());
  JSON.replace("{ut}", _boot_timestamp != 0 ? String(_boot_timestamp) : "null");
  JSON.replace("{ux}", unix != 0 ? String(unix) : "null");

  return JSON + "}";
}

String PREi::generateScanJSON() {
  String JSON = "";
  JSON += "{\"data\":[";
  int n = WiFi.scanNetworks();
  for(int i=0; i<n; ++i) {
    String temp = "{\
      \"id\":{i},\
      \"attributes\":{\
        \"ssid\":\"{s}\",\
        \"rssi\":{r},\
        \"encryption\":\"{e}\"\
      }\
    }";

    temp.replace("{i}", String(i+1));
    temp.replace("{s}", WiFi.SSID(i));
    temp.replace("{r}", String(WiFi.RSSI(i)));

    String enc = "";
    switch(WiFi.encryptionType(i)) {
      case ENC_TYPE_NONE:
        enc += "none";
        break;
      case ENC_TYPE_WEP:
        enc += "wep";
        break;
      case ENC_TYPE_TKIP:
        enc += "wpa";
        break;
      case ENC_TYPE_CCMP:
        enc += "wpa2";
        break;
      case ENC_TYPE_AUTO:
        enc += "auto";
        break;
      default:
        enc += "unknown";
    }
    temp.replace("{e}", enc);

    JSON += temp + (i<n-1 ? "," : "");
  }

  return JSON + "]}";
}

uint8_t PREi::getPinFromUri(String uri) {
  uri.replace("/pin/", "");
  for(int i=0; i<10; ++i) {
    if(pins[i].name == uri) {
      return i;
    }
  }
}

String PREi::generatePinJSON(PREiPin pin) {
  String JSON = "";
  JSON += "{\"data\":";
  JSON += "{\
    \"name\": \"{nm}\",\
    \"pin\": {pn},\
    \"digital\": {dg},\
    \"mode\": \"{md}\",\
    \"value\": {vl}\
  }";

  JSON.replace("{nm}", pin.name);
  JSON.replace("{pn}", String(pin.pin));
  JSON.replace("{dg}", pin.digital ? "true" : "false");
  JSON.replace("{md}", pin.mode);
  JSON.replace("{vl}", String(pin.digital ? (pin.mode == "pwm" ? pin.pwm_val : digitalRead(pin.pin)) : analogRead(pin.pin)));

  return JSON + "}";
}

void PREi::initPins() {
  pins[0] = (PREiPin){"D0", D0, true, "input", 0};
  pins[1] = (PREiPin){"D1", D1, true, "input", 0};
  pins[2] = (PREiPin){"D2", D2, true, "input", 0};
  pins[3] = (PREiPin){"D3", D3, true, "input", 0};
  pins[4] = (PREiPin){"D4", D4, true, "input", 0};
  pins[5] = (PREiPin){"D5", D5, true, "input", 0};
  pins[6] = (PREiPin){"D6", D6, true, "input", 0};
  pins[7] = (PREiPin){"D7", D7, true, "input", 0};
  pins[8] = (PREiPin){"D8", D8, true, "input", 0};
  pins[9] = (PREiPin){"A0", A0, false, "input", 0};
}
