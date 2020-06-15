#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

#define BUFFER_SIZE 128 // tamaño máximo para buffers
#define COMMAND_SEP "," // separador comando

class Command {

 private:
  char _inputBuffer[BUFFER_SIZE];
  byte _inputSize = 0;

public:
  Command();

  boolean isReady();
  char* getType();
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
