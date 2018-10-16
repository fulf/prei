#include "prei-api.h"

PREiApi::PREiApi(PREiCore* core, PREiWebServer* web_server) {
  _web_server = web_server;
  _core = core;

  setPinHandlers();
  setWiFiHandlers();
  setChipHandlers();
}

void PREiApi::setPinHandlers() {
  PREiPin* pins = _core->getPins();
  uint8_t totalPins = _core->getPinCount();
  for(uint8_t i = 0; i < totalPins; ++i) {
    _web_server->addGet(("/pin/" + pins[i].name).c_str(), std::bind(&PREiApi::handlePinGet, this, i));
    _web_server->addPost(("/pin/" + pins[i].name).c_str(), std::bind(&PREiApi::handlePinPost, this, i));
    _web_server->addPut(("/pin/" + pins[i].name).c_str(), std::bind(&PREiApi::handlePinPut, this, i));
    _web_server->addDelete(("/pin/" + pins[i].name).c_str(), std::bind(&PREiApi::handlePinDelete, this, i));
    _web_server->addOptions(("/pin/" + pins[i].name).c_str(), std::bind(&PREiApi::handlePinOptions, this, i));
  }
}

void PREiApi::setWiFiHandlers() {
  _web_server->addGet("/wifi", std::bind(&PREiApi::handleWiFiGet, this));
  _web_server->addPost("/wifi", std::bind(&PREiApi::handleWiFiPost, this));
  _web_server->addDelete("/wifi", std::bind(&PREiApi::handleWiFiDelete, this));
}

void PREiApi::setChipHandlers() {
  _web_server->addGet("/chip", std::bind(&PREiApi::handleInfo, this));
  _web_server->addPost("/chip", std::bind(&PREiApi::handleUpdate, this));
  _web_server->addDelete("/chip", std::bind(&PREiApi::handleRestart, this));
}

void PREiApi::handleWiFiGet() {
  String response = "[";
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
    temp.replace("{e}", _core->getWiFiEncryptionType(i));

    response += temp + (i<n-1 ? "," : "");
  }
  response += "]";

  _web_server->sendJSON(response, 200, true);
}

void PREiApi::handleWiFiPost() {
  String ssid = _web_server->getArgument("ssid");
  String pass = _web_server->getArgument("pass");

  uint16_t respCode = 200;
  String respMessage;

  if(ssid.length() > 31 || ssid.length() == 0) {
    respCode = 400;
    respMessage = "Invalid SSID provided.";
  } else if((pass.length() > 0 && pass.length() < 8) || pass.length() > 63) {
    respCode = 400;
    respMessage = "Invalid password provided.";
  }

  if(respCode == 200) {
    _core->connectWiFi(ssid.c_str(), pass.c_str());

    if(_core->getWiFiStatus() != WL_CONNECTED) {
      respCode = 403;
      respMessage = "Invalid credentials provided.";
    } else {
      respCode = 200;
      respMessage = "Connected succesfully to provided WiFi.";
    }
  }

  _web_server->sendJSON(respMessage, respCode);
}

void PREiApi::handleWiFiDelete() {
  WiFi.disconnect();
  _web_server->sendJSON("Disconnected succesfully!");
}

void PREiApi::handlePinGet(uint8_t pin) {
  String response = "{\
    \"name\": \"{nm}\",\
    \"pin\": {pn},\
    \"digital\": {dg},\
    \"mode\": \"{md}\",\
    \"value\": {vl}\
  }";

  PREiPin p = _core->getPin(pin);

  response.replace("{nm}", p.name);
  response.replace("{pn}", String(p.pin));
  response.replace("{dg}", p.digital ? "true" : "false");
  response.replace("{md}", p.mode);
  response.replace("{vl}", String(p.value));

  _web_server->sendJSON(response, 200, true);
}

void PREiApi::handlePinPost(uint8_t pin) {
  PREiPin p = _core->getPin(pin);
  uint16_t output = atoi(_web_server->getArgument("output"));

  _core->setPinOutput(p, output);
  _web_server->sendJSON("POST pin " + String(p.pin));
}

void PREiApi::handlePinPut(uint8_t pin) {
  PREiPin p = _core->getPin(pin);

  _core->setPinMode(p, _web_server->getArgument("mode"));
  _web_server->sendJSON("PUT pin" + String(p.pin));
}

void PREiApi::handlePinDelete(uint8_t pin) {
  PREiPin p = _core->getPin(pin);

  _core->setPinOutput(p, 0);
  _web_server->sendJSON("DELETE pin " + String(p.pin));
}

void PREiApi::handlePinOptions(uint8_t pin) {
  _web_server->send(200);
}

void PREiApi::handleInfo() {
  String response = "{\"data\":\
    {\
      \"attributes\":{\
        \"chip_id\":{ci},\
        \"flash_chip_id\":{fci},\
        \"hostname\":\"{hn}\",\
        \"mdns\":\"DEBUGGING\",\
        \"ap_ssid\":\"DEBUGGING\",\
        \"ap_ip\":\"{aip}\",\
        \"sta_ssid\":{ssi},\
        \"sta_ip\":{sip},\
        \"flash_chip_size\":{fcs},\
        \"flash_chip_real_size\":{fcrs},\
        \"boot_version\":\"{bv}\",\
        \"core_version\":\"{cv}\",\
        \"sdk_version\":\"{sv}\",\
        \"firmware_version\":\"DEBUGGING\",\
        \"firmware_size\":{ss},\
        \"firmware_md5\":\"{smd5}\",\
        \"boot_timestamp\":0,\
        \"unix\":0\
      }\
    }\
  }";

  response.replace("{ci}", String(ESP.getChipId()));
  response.replace("{fci}", String(ESP.getFlashChipId()));
  response.replace("{hn}", WiFi.hostname());
  // response.replace("{mdns}", "http://" + String(_esp_hostname) + ".local/");
  // response.replace("{asi}", _esp_hostname);
  response.replace("{aip}", WiFi.softAPIP().toString());
  response.replace("{ssi}", WiFi.status() == WL_CONNECTED ? '"' + WiFi.SSID() + '"' : "null");
  response.replace("{sip}", WiFi.status() == WL_CONNECTED ? '"' + WiFi.localIP().toString() + '"' : "null");
  response.replace("{fcs}", String(ESP.getFlashChipSize()));
  response.replace("{fcrs}", String(ESP.getFlashChipRealSize()));
  response.replace("{bv}", String(ESP.getBootVersion()));
  response.replace("{cv}", String(ESP.getCoreVersion()));
  response.replace("{sv}", String(ESP.getSdkVersion()));
  // response.replace("{fv}", VERSION);
  response.replace("{ss}", String(ESP.getSketchSize()));
  response.replace("{smd5}", ESP.getSketchMD5());
  // response.replace("{ut}", _boot_timestamp != 0 ? String(_boot_timestamp) : "null");
  // response.replace("{ux}", unix != 0 ? String(unix) : "null");

  _web_server->sendJSON(response, 200, true);
}

void PREiApi::handleUpdate() {
  HTTPUpdateResult ret = ESPhttpUpdate.update(_web_server->getArgument("firmware"), "1.0.0");
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          _web_server->sendJSON("Update failed.", 500);
          break;
      case HTTP_UPDATE_NO_UPDATES:
          _web_server->sendJSON("Update not necessary.", 304);
          break;
      case HTTP_UPDATE_OK:
          _web_server->sendJSON("Update started.", 200);
          ESP.restart();
          break;
  }
}

void PREiApi::handleRestart() {
  _web_server->sendJSON("Restart initiated succesfully!");
  ESP.restart();
}