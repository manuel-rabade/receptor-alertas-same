#include "Command.h"

// TEST
// TEST,ARG
// TEST,171 -> AB
// TEST,57584 -> F0F0

Command cmd = Command();

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (cmd.isReady()) {
    Serial.println("isReady");

    char* type = cmd.getType();
    Serial.print("getType: ");
    Serial.print(type);
    Serial.print(" (");
    Serial.print(strlen(type));
    Serial.println(")");
    free(type);

    if (cmd.isArg()) {
      Serial.println("isArg");

      char* arg = cmd.getArg();
      Serial.print("getArg: ");
      Serial.print(arg);
      Serial.print(" (");
      Serial.print(strlen(arg));
      Serial.println(")");
      free(arg);

      unsigned int argInt = cmd.getArgUInt();
      Serial.print("getArgUInt: 0x");
      Serial.println(argInt, HEX);

      byte argByte = cmd.getArgByte();
      Serial.print("getArgByte: 0x");
      Serial.println(argByte, HEX);
    }

    cmd.clearBuffer();
    Serial.println("---");
  }

  delay(50);
}

// Local Variables:
// mode: c++
// End:
