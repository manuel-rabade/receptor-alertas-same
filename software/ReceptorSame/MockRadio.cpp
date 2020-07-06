#include "MockRadio.h"

MockRadio::MockRadio() { }

void MockRadio::test(byte id) {
  _case = id;
  _step = 0;
  _updated = millis();
}

boolean MockRadio::begin() {
  if (_case == 1) {
    return false;
  } else {
    return true;
  }
}

byte MockRadio::setWBFrequency(unsigned long freq) {
  return 1;
}

byte MockRadio::getASQ() {
  return 0;
}

byte MockRadio::getSAMEState() {
  if (_case == 2) {
    if (_step == 0 && _next(10000)) {
      return 1; // preambulo
    } else if (_step == 1 && _next(100)) {
      return 2; // inicio cabecera
    } else if (_step == 2 && _next(100)) {
      return 3; // cabecera lista
    } else if (_step == 3 && _next(100)) {
      return 1;
    } else if (_step == 4 && _next(100)) {
      return 2;
    } else if (_step == 5 && _next(100)) {
      return 3;
    } else if (_step == 6 && _next(100)) {
      return 1;
    } else if (_step == 7 && _next(100)) {
      return 2;
    } else if (_step == 8 && _next(100)) {
      return 3;
    } else if (_step == 11 && _next(100)) {
      return 0; // fin de mensaje
    } else if (_step == 12 && _next(100)) {
      return 1; // preamulo
    } else if (_step == 13 && _next(100)) {
      return 0;
    } else if (_step == 14 && _next(100)) {
      return 1;
    } else if (_step == 15 && _next(100)) {
      return 0;
    } else if (_step == 16 && _next(100)) {
      return 1;
    } else if (_step == 17 && _next(100)) {
      return 0;
    }
  } else {
    return 0;
  }
}

byte MockRadio::getSAMESize() {
  if (_case == 2) {
    if (_step == 9 && _next(0)) {
      return 42;
    }
  }
  return 0;
}

void MockRadio::getSAMEMessage(byte size, byte message[]) {
  if (_case == 2) {
    if (_step == 10 && _next(0)) {
      strncpy(message, "-CIV-RWT-000000+0300-832300-XDIF/005-.....", size);
    }
  }
}

void MockRadio::clearSAMEBuffer() { }

unsigned int MockRadio::getWBFrequency() {
  return 999;
}

byte MockRadio::getRSSI() {
  return 99;
}

byte MockRadio::getSNR() {
  return 9;
}

void MockRadio::setMuteVolume(boolean mute) { }
void MockRadio::setVolume(int vol) { }

boolean MockRadio::_next(unsigned int delay) {
  if (millis() - _updated > delay) {
    _step++;
    return true;
  }
  return false;
}
