#include "prei-core.h"

PREiCore::PREiCore(uint8_t board) {
  switch (board) {
    case PREiCore::WEMOSD1miniPro:
      _pinCount = WEMOSD1miniPro::PinCount;
      _pins = WEMOSD1miniPro::Pins;
      break;
    default:
      _pinCount = 0;
      _pins = {};
      break;
  }
}

PREiPin* PREiCore::getPins() {
  return _pins;
}

PREiPin PREiCore::getPin(uint8_t pin) {
  return _pins[pin];
}

PREiPin PREiCore::getPin(String name) {
  for (uint8_t i = 0; i < _pinCount; ++i) {
    if (_pins[i].name == name) return _pins[i];
  }
}

uint8_t PREiCore::getPinCount() {
  return _pinCount;
}

void PREiCore::setPinMode(PREiPin pin, const char* mode) {
  uint8_t pinIndex = findIndexByPin(pin);

  if (!strcmp(mode, "INPUT")) {
    pinMode(pin.pin, INPUT);
    _pins[pinIndex].mode = "input";
  } else if (!strcmp(mode, "OUTPUT")) {
    pinMode(pin.pin, OUTPUT);
    _pins[pinIndex].mode = "output";
  } else if (!strcmp(mode, "PWM")) {
    pinMode(pin.pin, OUTPUT);
    _pins[pinIndex].mode = "pwm";
  }
}

void PREiCore::setPinOutput(PREiPin pin, uint16_t output) {
  uint8_t pinIndex = findIndexByPin(pin);
  if (!strcmp(pin.mode, "output")) {
    uint16_t val = clamp(output, 0, 1);
    digitalWrite(pin.pin, val);
    _pins[pinIndex].value = val;
  } else if (!strcmp(pin.mode, "pwm")) {
    uint16_t val = clamp(output, 0, 1023);
    analogWrite(pin.pin, val);
    _pins[pinIndex].value = val;
  }
}

String PREiCore::getWiFiEncryptionType(uint8_t wifi_id) {
  switch(WiFi.encryptionType(wifi_id)) {
    case ENC_TYPE_NONE:
      return "none";
    case ENC_TYPE_WEP:
      return "wep";
    case ENC_TYPE_TKIP:
      return "wpa";
    case ENC_TYPE_CCMP:
      return "wpa2";
    case ENC_TYPE_AUTO:
      return "auto";
    default:
      return "unknown";
  }
}

uint8_t PREiCore::connectWiFi(const char* ssid, const char* password) {
  const uint8_t MAX_TRIES = 30;

  WiFi.begin(ssid, password);

  for(int t=0; t<MAX_TRIES && WiFi.status() != WL_CONNECTED; ++t) {
    delay(200);
  }

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Could not connect.");
      WiFi.disconnect();
  } else {
    Serial.println("Connected: " + WiFi.localIP().toString());
  }

  return WiFi.status();
}

uint8_t PREiCore::getWiFiStatus() {
  return WiFi.status();
}

void PREiCore::disconnectWiFi() {
  WiFi.disconnect();
}

uint8_t PREiCore::findIndexByPin(PREiPin pin) {
  for (uint8_t i = 0; i < _pinCount; ++i) {
    if (_pins[i].pin == pin.pin) return i;
  }
}

uint16_t PREiCore::clamp(uint16_t value, uint16_t min, uint16_t max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}
