# PREi - Pseudo-REST ESP Interface
Offers an easy to set up REST server to control the ESP8266 module. Includes a DNS capturing WiFi AP configuration portal.

## Contents
- [Dependencies](#dependencies)
- [Installation](#installing)
  - [via Arduino](#installing-via-arduino)
  - [via PlatformIO](#installing-via-platformio)
- [Usage](#usage)
  - [Configuration portal](#configuration-portal)
  - [Pin control](#pin-control)
  - [Hardware control](#hardware-control)
    - [ESP module](#esp-module)
    - [Wireless](#wireless)

## Dependencies
PREi is built on top of the [ESP8266 Arduino Core](https://github.com/esp8266/Arduino), so that's an obvious dependency. Refer to [this link](https://github.com/esp8266/Arduino#installing-with-boards-manager) for instructions on how to install it.

Another dependency (which might be removed in future versions) is on [PREi NTP](https://github.com/fulf/prei-ntp), used for displaying the real time on the configuration portal.

## Installation
One of the best things about PREi is its ease of installation and compatibility with both Arduino and PlatformIO!

### Installing via Arduino
- Clone or [download](https://github.com/fulf/prei/archive/master.zip) this project as well as [PREi NTP](https://github.com/fulf/prei-ntp/archive/master.zip)
- Open the Arduino IDE, go to _Sketch_ -> _Include Library_ -> _Add .ZIP Library..._
- Select the PREi NTP library first, then repeat for the PREi library as well
- Upon a successful installation, an example project can be found by navigating to _File_ -> _Examples_ -> _PREi_ -> _myApp_

### Installing via PlatformIO
- Go to the _Libraries_ tab on the PlatformIO Home page.
- Search and find the PREi library and click the Install button
- Choose to install the library only for the current project (recommended) or globally

## Usage
As you can see from the example project included, all you have to do is include the library's header, instantiate a PREi object and call its `run` method every loop.

```cpp
#include <PREi.h> // Include the library

PREi *r; // Define a new PREi variable

void setup() {
  r = new PREi("ESP8266", "abcdef12"); // Instantiate it with an SSID and password (optionally)
}

void loop() {
  r->run(); // Call its run method every loop
}
```

Both constructor parameters are optional, if the password is not included, the WiFi AP will be open and if the SSID is not included either, the SSID will be chosen as follows :
```cpp
"ESP_" + String(ESP.getChipId())
```

Note that the SSID also dictates the mDNS hostname for the device.

### Configuration portal
Once connected to the ESP's wireless network, the configuration portal will pop up as a hotspot login page. It can also be accessed from the root page of the ESP.

The portal allows for a simple way to connect the ESP to a local wireless network, displaying scanned WiFis sorted by strength. It also shows some other useful information about the module.

<center>
<img src="http://i.imgur.com/SSRVYwB.png" alt="Configuration Portal">
</center>


### Pin control
Pins can be controlled via pseudo-REST requests* to the _pin/`pin-name`_ endpoints. The pin name can be any from _`D0` to `D8`_ for the digital pins, or _`A0`_ for the analog input.

A **GET** request returns a JSON with the pin's state that looks like this:
```javascript
{
  "data": {
    "name": "D4",
    "pin": 2,
    "digital": true,
    "mode": "input", // output / pwm
    "value": 1 // 0 - 1023 (0V - 3,3V) for PWM mode
  }
}
```

A **PUT** request with the `mode` parameter set to any of `output`, `input` or `pwm` changes the pin into that mode.

A **POST** request will put the pin in a _HIGH_ power state (3,3V). The request can also contain the `val` parameter set to any value from 0 to 1023 to change that pin's PWM value, if the pin is in the PWM mode.

A **DELETE** request will put the pin in a _LOW_ power state (0V).

### Hardware control
The PREi interface also exposes some minimal hardware control to the pseudo-REST API.

#### ESP module
The ESP module has it's URI set as `/esp`.

A **GET** request will return a huge JSON containing all the information presented on the configuration portal as well.

A **POST** request containing a URL for the `firmware` parameter will prompt the ESP to query that URL for a new firmware update. In case the query comes back with a 200 response, the ESP will start a OTA update.

A **DELETE** request will attempt restart the ESP module. Due to [a known issue with the ESP8266](https://github.com/resin-io-projects/esp8266/issues/5) a power-cycle after the initial serial flash is required to ensure the software reset will work properly.
#### Wireless
The ESP's WiFi has it's URI set as `/wifi`.

A **GET** request will start a WiFi scan and return a JSON containing a list of nearby WiFis with their SSID, RSSI and encryption type.

A **POST** request will make the ESP attempt to connect to the wireless specified by the `ssid` parameter, optionally using the password specified in the `pass` parameter.

A **DELETE** request will make the ESP disconnect from the wireless network, if connected to any.
