#include "MockRadio.h"

MockRadio::MockRadio() {
  _case = MOCKRADIO;
  _step = 0;
  _state = 0;
  _updated = millis();
  _paused = millis();
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
  if (_fsm(2, 0, 10000, 0) || _fsm(3, 0, 10000, 0) || _fsm(4, 0, 10000, 0))  {
    return 0;  // retardo
  }
  if (_fsm(2, 1, 500, 0) || _fsm(3, 1, 500, 0) || _fsm(4, 1, 500, 0))  {
    return 1;  // preambulo
  }
  if (_fsm(2, 2, 1000, 0) || _fsm(3, 2, 1000, 0) || _fsm(4, 2, 1000, 0)) {
    return 2; // inicio cabecera
  }
  if (_fsm(2, 3, 1000, 0) || _fsm(3, 3, 1000, 0) || _fsm(4, 3, 1000, 0)) {
    return 3; // cabecera 2 lista
  }
  if (_fsm(2, 4, 500, 0) || _fsm(3, 4, 500, 0) || _fsm(4, 4, 500, 0))  {
    return 1; // preambulo
  }
  if (_fsm(2, 5, 1000, 0) || _fsm(3, 5, 1000, 0) || _fsm(4, 5, 1000, 0)) {
    return 2; // inicio cabecera
  }
  if (_fsm(2, 6, 1000, 0) || _fsm(3, 6, 1000, 0) || _fsm(4, 6, 1000, 0)) {
    return 3; // cabecera 2 lista
  }
  if (_fsm(2, 7, 500, 0) || _fsm(3, 7, 500, 0) || _fsm(4, 7, 500, 0))  {
    return 1; // preambulo
  }
  if (_fsm(2, 8, 1000, 0) || _fsm(3, 8, 1000, 0) || _fsm(4, 8, 1000, 0)) {
    return 2; // inicio cabecera
  }
  if (_fsm(2, 9, 1000, 0) || _fsm(3, 9, 1000, 0) || _fsm(4, 9, 30000, 0)) {
    return 3; // cabecera 3 lista
  }
  // mensaje y/o tono asq
  if (_fsm(2, 10, 1000, 0) || _fsm(3, 10, 1000, 0) || _fsm(4, 10, 1000, 0))  {
    return 0; // fin de mensaje
  }
  if (_fsm(2, 11, 500, 0) || _fsm(3, 11, 500, 0) || _fsm(4, 11, 500, 0))  {
    return 1; // preambulo
  }
  if (_fsm(2, 12, 1000, 0) || _fsm(3, 12, 1000, 0) || _fsm(4, 12, 1000, 0))  {
    return 0; // fin de mensaje
  }
  if (_fsm(2, 13, 500, 0) || _fsm(3, 13, 500, 0) || _fsm(4, 13, 500, 0))  {
    return 1; // preambulo
  }
  if (_fsm(2, 14, 1000, 0) || _fsm(3, 14, 1000, 0) || _fsm(4, 14, 1000, 0))  {
    return 0; // fin de mensaje
  }
  if (_fsm(2, 15, 500, 0) || _fsm(3, 15, 500, 0) || _fsm(4, 15, 500, 0))  {
    return 1; // preambulo
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

boolean MockRadio::_fsm(byte _c, byte _s, unsigned int delay, unsigned int pause) {
  if (_case == _c && _step == _s && _state == 0) {
    if (millis() - _updated > delay) {
      if (pause > 0) {
        _paused = millis();
        _state = 1;
      } else {
        _updated = millis();
        _step++;
      }
      return false;
    }
    return true;
  }
  if (_case == _c && _step == _s && _state == 1) {
    if (millis() - _paused > pause) {
      _updated = millis();
      _step++;
      _state = 0;
    }
    return false;
  }
  return false;
}
