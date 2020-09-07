#include "MockRadio.h"

MockRadio::MockRadio() {
  _case = MOCKRADIO;
  _step = 0;
  _updated = millis();
}

boolean MockRadio::begin() {
  if (_case == 1) {
    return false;
  }
  return true;
}

byte MockRadio::setWBFrequency(unsigned long freq) {
  return 1;
}

byte MockRadio::getASQ() {
  if (_case == 4 && _step == 9 && millis() - _updated > 1500 && millis() - _updated < 10000) {
    return 1;
  } else {
    return 0;
  }
}

byte MockRadio::getSAMEState() {
  if (_case > 1) {
    if (_fsm(0, 10000))  {
      return 0;  // retardo
    }
    if (_fsm(1, 500))  {
      return 1;  // preambulo
    }
    if (_fsm(2, 1000)) {
      return 2; // inicio cabecera
    }
    if (_fsm(3, 1000)) {
      return 3; // cabecera 2 lista
    }
    if (_fsm(4,500)) {
      return 1; // preambulo
    }
    if (_fsm(5, 1000)) {
      return 2; // inicio cabecera
    }
    if (_fsm(6, 1000)) {
      return 3; // cabecera 2 lista
    }
    if (_fsm(7,500)) {
      return 1; // preambulo
    }
    if (_fsm(8, 1000)) {
      return 2; // inicio cabecera
    }
    if ((_case == 4 && _fsm(9, 30000)) || (_case != 4 && _fsm(9, 1000))) {
      return 3; // cabecera 3 lista
    }
    // mensaje y/o tono asq
    if (_fsm(10, 1000)) {
      return 0; // fin de mensaje
    }
    if (_fsm(11, 500)) {
      return 1; // preambulo
    }
    if (_fsm(12, 1000)) {
      return 0; // fin de mensaje
    }
    if (_fsm(13, 500)) {
      return 1; // preambulo
    }
    if (_fsm(14, 1000)) {
      return 0; // fin de mensaje
    }
    if (_fsm(15, 500)) {
      return 1; // preambulo
    }
    return 0;
  }
  return 0;
}

byte MockRadio::getSAMESize() {
  if (_case == 2) {
    return 42;
  } else if (_case == 3 || _case == 4) {
    return 52;
  } else {
    return 0;
  }
}

void MockRadio::getSAMEMessage(byte size, byte message[]) {
  if (_case == 2 || _case == 3 || _case == 4) {
    strncpy(message, MOCKMSG, size);
  }
}

void MockRadio::clearSAMEBuffer() { }

unsigned int MockRadio::getWBFrequency() {
  return 777;
}

byte MockRadio::getRSSI() {
  return 88;
}

byte MockRadio::getSNR() {
  return 9;
}

void MockRadio::setMuteVolume(boolean mute) { }

void MockRadio::setVolume(int vol) { }

boolean MockRadio::_fsm(byte _s, unsigned int delay) {
  if (_step == _s) {
    if (millis() - _updated > delay) {
      _updated = millis();
      _step++;
    }
    return true;
  }
  return false;
}
