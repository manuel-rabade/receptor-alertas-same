#ifndef MOCKRADIO_H
#define MOCKRADIO_H

#include <Arduino.h>

// #define MOCKRADIO 0 // desactivamos pruebas de integracion

// #define MOCKRADIO 1 // falla si4707

// #define MOCKRADIO 2 // mensaje de prueba
// #define MOCKMSG "-CIV-RWT-000000+0300-832300-XDIF/005-....."
// #define MOCKMSG "-CIV-RMT-000000+0300-832300-XDIF/005-....."

// #define MOCKRADIO 3 // mensaje de alerta con voz sin tono asq
// #define MOCKMSG "-WXR-SPS-039173-039051-139069+0030-1591829-KCLE/NWS-"

// #define MOCKRADIO 4 // mensaje de alerta con tono asq
// #define MOCKMSG "-WXR-TOR-039173-039051-139069+0030-1591829-KCLE/NWS-"

//#define MOCKRADIO 5 // mensaje con solo dos cabeceras
//#define MOCKMSG "-CIV-RWT-000000+0300-832300-XDIF/005-....."

#define MOCKRADIO 6 // mensaje de alerta sismica
#define MOCKMSG "-CIV-EQW-000000+0005-832326-XDIF/005-....."

class MockRadio {
private:
  byte _case;
  byte _step;
  byte _count;
  unsigned long _updated;
  boolean _fsm(byte, unsigned int);

public:
  MockRadio();
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
