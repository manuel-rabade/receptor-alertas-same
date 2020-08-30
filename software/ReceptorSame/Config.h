#ifndef CONFIG_H
#define COFIG_H

#include <Arduino.h>

#define CODES_MAX 20

// estructura de la configuración
struct Memory {
  byte version;
  byte channel;
  byte volume;
  byte relay;
  unsigned long rwtPeriod; // millis
  unsigned long rmtPeriod; // millis
  char areaCodes[CODES_MAX][6];
  char eventCodes[CODES_MAX][3];
  byte areaCodesCount;
  byte eventCodesCount;
};

class Config {

private:
  // instancia de la configuración
  Memory _config;

  // busqueda binaria de arreglos
  int _binarySearch(char*, byte, int, int, byte, char*);
  // inserción ordenada de arreglos
  boolean _insertSorted(char*, byte, byte*, char*);
  // borrado ordenado de arreglos
  boolean _deleteSorted(char*, byte, byte*, char*);

public:
  Config();

  // setters y getters de datos simples
  void setVersion(byte);
  byte getVersion();
  boolean setChannel(byte);
  byte getChannel();
  void setMute(boolean);
  boolean getMute();
  boolean setVolume(byte);
  byte getVolume();
  boolean setRelay(byte);
  byte getRelay();
  void setRwtPeriod(unsigned int);
  unsigned int getRwtPeriod();
  unsigned long getRwtPeriodMillis();
  void setRmtPeriod(unsigned int);
  unsigned int getRmtPeriod();
  unsigned long getRmtPeriodMillis();

  // operación de codigos de area
  boolean setAreaCode(char*);
  boolean clearAreaCode(char*);
  boolean findAreaCode(char*);
  boolean findAreaCodeWildcard(char*);
  byte countAreaCodes();
  char* strAreaCode(byte);
  void emptyAreaCodes();

  // operación de codigos de evento
  boolean setEventCode(char*);
  boolean clearEventCode(char*);
  boolean findEventCode(char*);
  byte countEventCodes();
  char* strEventCode(byte);
  void emptyEventCodes();

  // guardar o cargar en eeprom y tamaño de datos
  void save();
  void reload();
  int bytes();
};

#endif

// Local Variables:
// mode: c++
// End:
