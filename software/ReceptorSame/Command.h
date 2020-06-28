#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

// configuracion comandos
#define BUFFER_SIZE 128 // tamaño máximo para buffers
#define COMMAND_SEP "," // separador comando

// llaves de comandos
#define CMD_AREA_ADD 7
#define CMD_AREA_DEL 8
#define CMD_AREA_LIST 9
#define CMD_AUDIO_CONF 10
#define CMD_CHANNEL 1
#define CMD_DEFAULTS 5
#define CMD_DUMP 19
#define CMD_EVENT_ADD 11
#define CMD_EVENT_DEL 12
#define CMD_EVENT_LIST 13
#define CMD_FREQUENCY 3
#define CMD_MUTE 4
#define CMD_QUALITY 2
#define CMD_RELAY_CONF 14
#define CMD_RELOAD 18
#define CMD_RMT_DURATION 16
#define CMD_RWT_DURATION 15
#define CMD_SAVE 6
#define CMD_VOLUME 17

struct cmd {
  char *str;
  byte n;
};

class Command {

 private:
  char _inputBuffer[BUFFER_SIZE];
  byte _inputSize = 0;

  static int _compare(const void*, const void*);

public:
  Command();

  boolean isReady();
  char* getType();
  byte getCmd();
  boolean isArg();
  char* getArg();
  unsigned int getArgUInt();
  byte getArgByte();
  void clearBuffer();
};

#endif

// Local Variables:
// mode: c++
// End:
