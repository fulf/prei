#include <PREi.h>

PREi *r;

void setup() {
  r = new PREi("ESP8266", "abcdef12");
}

void loop() {
  r->run();
}
