#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

// configuracion comandos
#define BUFFER_SIZE 128 // tamaño máximo para buffers
#define COMMAND_SEP "," // separador comando

// llaves de comandos
#define CMD_GET_FREQUENCY 3
#define CMD_GET_QUALITY 2
#define CMD_LOAD_DEFAULTS 5
#define CMD_SET_CHANNEL 1
#define CMD_SET_MUTE 4

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
