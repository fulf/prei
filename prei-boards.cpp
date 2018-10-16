#include "prei-boards.h"

namespace WEMOSD1miniPro {
  int PinCount = 10;
  PREiPin Pins[] = {
    (PREiPin){"D0", D0, true, "input"},
    (PREiPin){"D1", D1, true, "input"},
    (PREiPin){"D2", D2, true, "input"},
    (PREiPin){"D3", D3, true, "input"},
    (PREiPin){"D4", D4, true, "input"},
    (PREiPin){"D5", D5, true, "input"},
    (PREiPin){"D6", D6, true, "input"},
    (PREiPin){"D7", D7, true, "input"},
    (PREiPin){"D8", D8, true, "input"},
    (PREiPin){"A0", A0, false, "input"}
  };
}
