#include "prei-poller.h"
#include <stdio.h>


PREiPoller::PREiPoller(const char* name, const char* url, PREiActor* actor) {
  _call_sign = name;
  _url = url;
  _actor = actor;
}

void PREiPoller::poll() {
  _http_client.begin(_url);
  int httpCode = _http_client.GET();

  if(httpCode == HTTP_CODE_OK) {
    String payload = _http_client.getString();

    if (payload != "") {
      int firstPipe = payload.indexOf('|'),
        lastPipe = payload.lastIndexOf('|');

      String entity = payload.substring(0, firstPipe),
        pin = payload.substring(firstPipe + 1 , lastPipe),
        action = payload.substring(lastPipe + 1);

      if (!strcmp(entity.c_str(), _call_sign)) {
          _actor->act(pin, action);
      }
    }
  }
  _http_client.end();
  yield();
}
