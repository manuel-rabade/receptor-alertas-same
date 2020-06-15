#ifndef CONFIG_H
#define COFIG_H

#include <Arduino.h>

#define UINT_SIZE

#define VERSION_ADDR 1
#define CHANNEL_ADDR 2
#define VOLUME_ADDR 3
#define AUDIO_ADDR 4
#define RELAY_ADDR 5
#define RWT_ADDR 6
#define RMT_ADDR 8

#define AREA_ADDR 10
#define AREA_SIZE 50

#define EVENT_ADDR 60
#define EVENT_SIZE 50

class Config {

public:
  Config();
  byte version;

  boolean setChannel(byte);
  byte getChannel();

  void setMute(boolean);
  boolean getMute();

  void setVolume(byte);
  byte getVolume();

  boolean setAudio(byte);
  byte getAudio();

  boolean setRelay(byte);
  byte getRelay();

  boolean setArea(char*);
  boolean clearArea(char*);
  boolean findArea(char*);
  char* listArea();

  boolean setEvent(char*);
  boolean clearEvent(char*);
  boolean findEvent(char*);
  char* listEvent();

  boolean setRwtDuration(unsigned int);
  unsigned int getRwtDuration();

  boolean setRmtDuration(unsigned int);
  unsigned int getRmtDuration();

  void dump();
};

#endif

// Local Variables:
// mode: c++
// End:
