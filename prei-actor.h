#ifndef PREI_ACTOR_H
#define PREI_ACTOR_H

#include "prei-core.h"

class PREiActor{
  private:
    PREiCore* _core;
  public:
    PREiActor(PREiCore*);
    void act(String, String);
};

#endif
