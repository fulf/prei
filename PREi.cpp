#include "PREi.h"

PREi::PREi() {
  Serial.begin(115200);
  _esp_hostname = "ESP_" + String(ESP.getChipId());

  WiFi.setAutoConnect(false);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(_esp_hostname.c_str(), "zxyqweiop");

  _dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  _dns_server.start(53, "*", WiFi.softAPIP());

  MDNS.begin(_esp_hostname.c_str());

  _web_server.on("/info", [&](){
    _web_server.send(200, "application/json", generateInfoJSON());
  });

  _web_server.on("/scan", [&](){
    _web_server.send(200, "application/json", generateScanJSON());
  });

  _web_server.on("/connect", [&](){
    const int MAX_TRIES = 30;
    int status;

    String ssid = _web_server.arg("ssid");
    String pass = _web_server.arg("pass");
    int respCode = 200;
    String respMessage;

    if(ssid.length() > 31 || ssid.length() == 0) {
      respCode = 400;
      respMessage = "Invalid SSID provided.";
    } else if(pass.length() < 8 || pass.length() > 63) {
      respCode = 400;
      respMessage = "Invalid password provided.";
    }

    if(respCode == 200) {
      WiFi.begin(ssid.c_str(), pass.c_str());

      for(int t=0; t<MAX_TRIES && (status = WiFi.status()) != WL_CONNECTED; ++t) {
        delay(150);
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
  });

  _web_server.on("/disconnect", [&](){
    PREi::sendJSON(200, "Disconnected succesfully!");
    WiFi.disconnect();
  });

  _web_server.begin();
}

void PREi::run() {
  _dns_server.processNextRequest();
  _web_server.handleClient();
  yield();
}

void PREi::sendJSON(int code, String message) {
  String success = (code/100) == 2 ? "true" : "false";
  _web_server.send(code, "application/json", "{\"success\":" + success+ ",\"message\":\"" + message+ "\"}");
}

String PREi::generateInfoJSON() {
  unsigned long unix = _prei_ntp.getUnix();

  if(_boot_timestamp == 0 && unix != 0) {
    _boot_timestamp = unix - (millis()/1000);
  }

  String JSON = "";
  JSON += "{\"links\":";
  JSON += PREi::generateLinksJSON("/info");
  JSON += ",\"data\":";
  JSON += "{\
    \"attributes\":{\
      \"chip_id\":\"{ci}\",\
      \"flash_chip_id\":\"{fci}\",\
      \"hostname\":\"{hn}\",\
      \"mdns\":\"{mdns}\",\
      \"ap_ssid\":\"{asi}\",\
      \"ap_ip\":\"{aip}\",\
      \"sta_ssid\":\"{ssi}\",\
      \"sta_ip\":\"{sip}\",\
      \"flash_chip_size\":\"{fcs}\",\
      \"flash_chip_real_size\":\"{fcrs}\",\
      \"boot_version\":\"{bv}\",\
      \"core_version\":\"{cv}\",\
      \"sdk_version\":\"{sv}\",\
      \"firmware_version\":\"{fv}\",\
      \"firmware_size\":\"{ss}\",\
      \"firmware_md5\":\"{smd5}\",\
      \"boot_timestamp\":\"{ut}\",\
      \"unix\":\"{ux}\"\
    }\
  }";

  JSON.replace("{ci}", String(ESP.getChipId()));
  JSON.replace("{fci}", String(ESP.getFlashChipId()));
  JSON.replace("{hn}", WiFi.hostname());
  JSON.replace("{mdns}", "http://" + String(_esp_hostname) + ".local/");
  JSON.replace("{asi}", _esp_hostname);
  JSON.replace("{aip}", WiFi.softAPIP().toString());
  JSON.replace("{ssi}", WiFi.SSID());
  JSON.replace("{sip}", WiFi.localIP().toString());
  JSON.replace("{fcs}", String(ESP.getFlashChipSize()));
  JSON.replace("{fcrs}", String(ESP.getFlashChipRealSize()));
  JSON.replace("{bv}", String(ESP.getBootVersion()));
  JSON.replace("{cv}", String(ESP.getCoreVersion()));
  JSON.replace("{sv}", String(ESP.getSdkVersion()));
  JSON.replace("{fv}", VERSION);
  JSON.replace("{ss}", String(ESP.getSketchSize()));
  JSON.replace("{smd5}", ESP.getSketchMD5());
  JSON.replace("{ut}", String(_boot_timestamp));
  JSON.replace("{ux}", String(_prei_ntp.getUnix()));

  return JSON + "}";
}

String PREi::generateScanJSON() {
  String JSON = "";
  JSON += "{\"links\":";
  JSON += PREi::generateLinksJSON("/scan");
  JSON += ",\"data\":[";
  int n = WiFi.scanNetworks();
  for(int i=0; i<n; ++i) {
    String temp = "{\
      \"id\":\"{i}\",\
      \"attributes\":{\
        \"ssid\":\"{s}\",\
        \"rssi\":\"{r}\",\
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

String PREi::generateLinksJSON(String path) {
  String hostname = _web_server.hostHeader();
  String JSON = "{";
  if(path == "/info") {
    JSON += "\"self\":\"http://"+hostname+"/info\",";
    JSON += "\"scan\":\"http://"+hostname+"/scan\",";
    JSON += "\"connect\":\"http://"+hostname+"/connect\",";
    JSON += "\"disconnect\":\"http://"+hostname+"/disconnect\"";
  } else if(path == "/scan") {
    JSON += "\"self\":\"http://"+hostname+"/scan\",";
    JSON += "\"info\":\"http://"+hostname+"/info\",";
    JSON += "\"connect\":\"http://"+hostname+"/connect\",";
    JSON += "\"disconnect\":\"http://"+hostname+"/disconnect\"";
  };
  return JSON + "}";
}
