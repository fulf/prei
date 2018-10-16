#include "prei-actor.h"

PREiActor::PREiActor(PREiCore* core) {
  _core = core;
}

void PREiActor::act(String pin, String action) {
  PREiPin p = _core->getPin(pin);

  _core->setPinOutput(p, action == "on" ? 1 : 0);
}
