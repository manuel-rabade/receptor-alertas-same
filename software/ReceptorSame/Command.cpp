#include "Command.h"

// tabla de comandos ordenada alfabeticamente
struct cmd table[] = {
  "GET_FREQUENCY", CMD_GET_FREQUENCY,
  "GET_QUALITY", CMD_GET_QUALITY,
  "LOAD_DEFAULTS", CMD_LOAD_DEFAULTS,
  "SET_CHANNEL", CMD_SET_CHANNEL,
  "SET_MUTE", CMD_SET_MUTE,
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
