#include "Arduino.h"  // Required for byte type

#define SI4707_ADDRESS 0b0010001 // if SEN is pulled low
#define SI4707_RESET 5

class Si4707 {
private:
  // These arrays will be used by most command functions to construct
  // the command string and receive response bytes.
  byte rsp[15];
  byte cmd[8];

  void powerUp(void);
  byte command_Tune_Freq(unsigned int frequency);
  unsigned int command_Tune_Status(byte intAck, byte returnIndex);
  byte command_Get_Rev(byte returnIndex);
  void command_SAME_Status(byte setArgument, byte returnAddress, byte * returnData);
  byte command_RSQ_Status(byte returnIndex);
  byte command_ASQ_Status(byte setArgument, byte returnIndex);
  byte command_Get_Int_Status(void);
  void waitForCTS(void);
  void writeCommand(byte cmdSize, byte * command, byte replySize, byte * reply);
  unsigned int getProperty(unsigned int property);
  void setProperty(unsigned int propNumber, unsigned int propValue);
  void i2cReadBytes(byte number_bytes, byte * data_in);
  void i2cWriteBytes(uint8_t number_bytes, uint8_t *data_out);
  byte initSi4707();
  void setupInterrupts();

public:
  Si4707();
  boolean begin();
  byte setWBFrequency(unsigned long freq);
  void tuneWBFrequency(signed char increment);
  unsigned int getWBFrequency();
  boolean getRSQ();
  byte getRSSI();
  byte getSNR();
  signed char getFreqOffset();
  byte getSAMEState();
  byte getSAMESize();
  void getSAMEMessage(byte size, byte message[]);
  void clearSAMEBuffer();
  byte getASQ();
  void setMuteVolume(boolean mute);
  void setVolume(int vol);
  void setSNR(unsigned int snr);
  void setRSSI(unsigned int rssi);
  byte getIntStatus();
  byte getSAMEInt();
  byte getASQInt();
};
