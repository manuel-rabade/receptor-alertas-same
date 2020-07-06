#ifndef MOCKRADIO_H
#define MOCKRADIO_H

#include <Arduino.h>

class MockRadio {
private:
  byte _case;
  byte _step;
  unsigned long _updated;
  boolean _next(unsigned int);

public:
  MockRadio();
  void test(byte);
  boolean begin();
  byte getASQ();
  byte getSAMEState();
  byte getSAMESize();
  void getSAMEMessage(byte size, byte message[]);
  void clearSAMEBuffer();
  void setMuteVolume(boolean mute);
  void setVolume(int vol);
  byte setWBFrequency(unsigned long freq); // ver byte
  unsigned int getWBFrequency();
  byte getRSSI();
  byte getSNR();
};

#endif

// Local Variables:
// mode: c++
// End:
