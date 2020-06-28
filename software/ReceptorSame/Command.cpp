#include "Command.h"

// tabla de comandos ordenada alfabeticamente
struct cmd table[] = {
  "AREA_ADD", CMD_AREA_ADD,
  "AREA_DEL", CMD_AREA_DEL,
  "AREA_LIST", CMD_AREA_LIST,
  "AUDIO_CONF", CMD_AUDIO_CONF,
  "EVENT_ADD", CMD_EVENT_ADD,
  "EVENT_DEL", CMD_EVENT_DEL,
  "EVENT_LIST", CMD_EVENT_LIST,
  "GET_FREQUENCY", CMD_GET_FREQUENCY,
  "GET_QUALITY", CMD_GET_QUALITY,
  "LOAD_DEFAULTS", CMD_LOAD_DEFAULTS,
  "RELAY_CONF", CMD_RELAY_CONF,
  "RELOAD", CMD_RELOAD,
  "RMT_TIMEOUT", CMD_RMT_TIMEOUT,
  "RWT_TIMEOUT", CMD_RWT_TIMEOUT,
  "SAVE", CMD_SAVE,
  "SET_CHANNEL", CMD_SET_CHANNEL,
  "SET_MUTE", CMD_SET_MUTE,
  "SET_VOLUME", CMD_SET_VOLUME,
};

Command::Command() {

}

boolean Command::isReady() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (_inputSize > 0) {
        return true;
      }
    } else {
      if (_inputSize < BUFFER_SIZE) {
        _inputBuffer[_inputSize] = c;
        _inputSize++;
      }
    }
  }

  return false;
}

char* Command::getType() {
  byte len = _inputSize;
  char *pos = _inputBuffer;

  char *sep = strstr(_inputBuffer, COMMAND_SEP);
  if (sep != NULL) {
    len = sep - pos;
  }

  char *res = malloc(len * sizeof(char));
  strncpy(res, pos, len);
  res[len] = '\0';
  return res;
}

byte Command::getCmd() {
  struct cmd *res;
  char* type = getType();

  struct cmd key = { type };
  res = bsearch(&key, table, sizeof(table)/sizeof(table[0]),
                sizeof table[0], _compare);
  free(type);

  if (res) {
    return res->n;
  } else {
    return 0;
  }
}

boolean Command::isArg() {
  if (strstr(_inputBuffer, COMMAND_SEP) == NULL) {
    return false;
  }

  return true;
}

char* Command::getArg() {
  char *sep = strstr(_inputBuffer, COMMAND_SEP);
  if (sep == NULL) {
    return NULL;
  }

  byte len = _inputSize - (byte)(sep - _inputBuffer) - 1;
  char *res = malloc(len * sizeof(char));
  strncpy(res, sep + 1, len);
  res[len] = '\0';
  return res;
}

unsigned int Command::getArgUInt() {
  char* arg = getArg();
  unsigned int res = strtoul(arg, NULL, 10);
  free(arg);
  return res;
}

byte Command::getArgByte() {
  return (byte) getArgUInt();
}

void Command::clearBuffer() {
  memset(_inputBuffer,0,BUFFER_SIZE);
  _inputSize = 0;
}

static int Command::_compare(const void *a, const void *b) {
     const struct cmd *cmd1 = a;
     const struct cmd *cmd2 = b;
     return strcmp(cmd1->str, cmd2->str);
}
