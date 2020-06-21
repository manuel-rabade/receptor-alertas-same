#ifndef CONFIG_H
#define COFIG_H

#include <Arduino.h>

#define CODES_MAX 20

struct Memory {
  byte version;
  byte channel;
  byte volume;
  byte audio;
  byte relay;
  unsigned int rwtDuration;
  unsigned int rmtDuration;
  char areaCodes[CODES_MAX][6];
  char eventCodes[CODES_MAX][3];
  byte areaCodesCount;
  byte eventCodesCount;
};

class Config {

private:
  Memory _config;
  int binarySearch(char*, byte, int, int, byte, char*);
  boolean insertSorted(char*, byte, byte*, char*);
  boolean deleteSorted(char*, byte, byte*, char*);

public:
  Config();

  void setVersion(byte);
  byte getVersion();

  boolean setChannel(byte);
  byte getChannel();

  void setMute(boolean);
  boolean getMute();

  boolean setVolume(byte);
  byte getVolume();

  boolean setAudio(byte);
  byte getAudio();

  boolean setRelay(byte);
  byte getRelay();

  void setRwtDuration(unsigned int);
  unsigned int getRwtDuration();

  void setRmtDuration(unsigned int);
  unsigned int getRmtDuration();

  boolean setAreaCode(char*);
  boolean clearAreaCode(char*);
  boolean findAreaCode(char*);
  byte countAreaCodes();
  char* strAreaCode(byte);
  void emptyAreaCodes();

  boolean setEventCode(char*);
  boolean clearEventCode(char*);
  boolean findEventCode(char*);
  byte countEventCodes();
  char* strEventCode(byte);
  void emptyEventCodes();

  void save();
  void reload();
  int bytes();
};

#endif

// Local Variables:
// mode: c++
// End:
