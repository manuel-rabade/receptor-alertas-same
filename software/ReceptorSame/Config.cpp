#include <EEPROM.h>
#include "Config.h"

Config::Config() {
  reload();
}

void Config::setVersion(byte version) {
  _config.version = version;
}

byte Config::getVersion() {
  return _config.version;
}

boolean Config::setChannel(byte channel) {
  if (channel < 1 || channel > 7) {
    return false;
  }
  _config.channel = channel;
  return true;
}

byte Config::getChannel() {
  return _config.channel;
}

void Config::setMute(boolean mute) {
  if (mute) {
    _config.volume |= 1 << 7;
  } else {
    _config.volume &= ~(1 << 7);
  }
}

boolean Config::getMute() {
  return (_config.volume & 1 << 7) > 0;
}

boolean Config::setVolume(byte volume) {
  if (volume > 63) {
    return false;
  }

  volume |= _config.volume & 1 << 7;
  _config.volume = volume;
  return true;
}

byte Config::getVolume() {
  byte volume = _config.volume;
  volume &= ~(1 << 7);
  return volume;
}

boolean Config::setAudio(byte audio) {
  if (audio > 2) {
    return false;
  }
  _config.audio = audio;
}

byte Config::getAudio() {
  return _config.audio;
}

boolean Config::setRelay(byte relay) {
  if (relay > 3) {
    return false;
  }
  _config.relay = relay;
}

byte Config::getRelay() {
  return _config.relay;
}

void Config::setRwtDuration(unsigned int seconds) {
  _config.rwtDuration = seconds;
}

unsigned int Config::getRwtDuration() {
  return _config.rwtDuration;
}

void Config::setRmtDuration(unsigned int seconds) {
  _config.rmtDuration = seconds;
}

unsigned int Config::getRmtDuration() {
  return _config.rmtDuration;
}

// -------------------------------------------------------------------------------------------------

boolean Config::setAreaCode(char* code) {
  if (findAreaCode(code)) {
    return false;
  }

  return _insertSorted(_config.areaCodes[0], 6, &_config.areaCodesCount, code);
}

boolean Config::clearAreaCode(char* code) {
  return _deleteSorted(_config.areaCodes[0], 6, &_config.areaCodesCount, code);
}

boolean Config::findAreaCode(char* code) {
  if (_binarySearch(_config.areaCodes[0], 6, 0, _config.areaCodesCount, _config.areaCodesCount, code) < 0) {
    return false;
  }

  return true;
}

char* Config::strAreaCode(byte idx) {
  char *ptr = malloc(7 * sizeof(char));
  strncpy(ptr, _config.areaCodes[idx], 6);
  ptr[6] = '\0';
  return ptr;
}

byte Config::countAreaCodes() {
  return _config.areaCodesCount;
}

void Config::emptyAreaCodes() {
  _config.areaCodesCount = 0;
}

// -------------------------------------------------------------------------------------------------

boolean Config::setEventCode(char* code) {
  if (findEventCode(code)) {
    return false;
  }

  return _insertSorted(_config.eventCodes[0], 3, &_config.eventCodesCount, code);
}

boolean Config::clearEventCode(char* code) {
  return _deleteSorted(_config.eventCodes[0], 3, &_config.eventCodesCount, code);
}

boolean Config::findEventCode(char* code) {
  if (_binarySearch(_config.eventCodes[0], 3, 0, _config.eventCodesCount, _config.eventCodesCount, code) < 0) {
    return false;
  }

  return true;
}

char* Config::strEventCode(byte idx) {
  char *ptr = malloc(4 * sizeof(char));
  strncpy(ptr, _config.eventCodes[idx], 3);
  ptr[3] = '\0';
  return ptr;
}

byte Config::countEventCodes() {
  return _config.eventCodesCount;
}

void Config::emptyEventCodes() {
  _config.eventCodesCount = 0;
}

// -------------------------------------------------------------------------------------------------

void Config::save() {
  EEPROM.put(0, _config);
}

void Config::reload() {
  EEPROM.get(0, _config);
}

int Config::bytes() {
  return sizeof(_config);
}

// -------------------------------------------------------------------------------------------------

int Config::_binarySearch(char* codes, byte len, int low, int high, byte size, char* key) {
  // Serial.print("search ");
  // Serial.print(key);
  // Serial.print(" -> ");
  // Serial.print(size);
  // Serial.print(" ");
  // Serial.print(low);
  // Serial.print(" ");
  // Serial.print(high);

  if (high < low || size < 1) {
    // Serial.println(" return -1");
    return -1;
  }

  byte half = (low + high) / 2;
  // Serial.print(" ");
  // Serial.print(half);

  char val[len];
  strncpy(val, &codes[half * len], len);

  int cmp = strncmp(val, key, len);
  // Serial.print(" cmp ");
  // Serial.println(cmp);
  if (cmp == 0) {
    return half;
  }

  if (cmp < 0) {
    return _binarySearch(codes, len, (half + 1), high, size, key);
  } else {
    return _binarySearch(codes, len, low, (half - 1), size, key);
  }
}

boolean Config::_insertSorted(char* codes, byte len, byte* size, char* key) {
  if (*size >= CODES_MAX) {
    return false;
  }
  // Serial.print("insert ");
  // Serial.println(key);

  int pos = - 1;
  for (pos = (*size) - 1; (pos >= 0 && (*size) > 0); pos--) {
    // Serial.print("pos ");
    // Serial.print(pos);
    char val[len];
    strncpy(val, &codes[pos * len], len);

    int cmp = strncmp(val, key, len);
    // Serial.print(" cmp ");
    // Serial.print(cmp);
    if (cmp >= 0) {
      strncpy(&codes[(pos + 1) * len], &codes[pos * len], len);
      // Serial.println(" move ->");
    } else if (cmp < 0) {
      // Serial.println(" break");
      break;
    }
  }

  // Serial.print("insert ");
  // Serial.print(pos);
  // Serial.print(" + 1 ");
  strncpy(&codes[(pos + 1) * len], key, len);
  (*size)++;
  // Serial.print(" size ");
  // Serial.println(*size);
  return true;
}

boolean Config::_deleteSorted(char* codes, byte len, byte* size, char* key) {
  int pos = _binarySearch(codes, len, 0, *size, *size, key);
  if (pos < 0) {
    return false;
  }

  for (int idx = pos; idx < (*size) - 1; idx++) {
    strncpy(&codes[idx * len], &codes[(idx + 1) * len], 3);
  }

  (*size)--;
  return true;
}
