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

ESP8266WebServer PREi::getServer() {
  return _web_server;
}

DNSServer PREi::getDNS() {
  return _dns_server;
}
void PREi::init() {
  Serial.begin(115200);
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

  _web_server.onNotFound(std::bind(&PREi::handleNotFound, this));

  _web_server.begin();
}

void PREi::sendJSON(int code, String message, bool raw=false) {
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
  HTTPUpdateResult ret = ESPhttpUpdate.update(_web_server.arg("firmware"), VERSION);
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
