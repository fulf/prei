#ifndef PREI_BOARDS_H
#define PREI_BOARDS_H

#include <Arduino.h>
#include <map>

struct PREiPin{
  String name;
  uint8_t pin;
  bool digital;
  const char* mode;
  int value;
};

namespace WEMOSD1miniPro {
  extern int PinCount;
  extern PREiPin Pins[];
}

#endif
