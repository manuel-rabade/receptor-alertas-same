#include "si4707_definitions.h"
#include "Si4707.h"
#include <Wire.h>

Si4707::Si4707() { }

boolean Si4707::begin()
{
  // First, initSi4707() must be called. The function returns the value
  //  of the Part Number reported by the Si4707, which can be checked
  //  to verify communication. It should always be 7.
  byte partNumber = initSi4707();
  if (partNumber == 7) {
    return true;
  }

  return false;
}

// initSi4707 performs the following functions, in sequence:
//  * Initialize all pins connected to Si4707 (SEN, SCLK, SDIO, and RST)
//  * Starts up the Wire class - used for two-wire communication
//  * Applies a reset signal to the Si4707
//  * Sends the Power Up command to the Si4707
//  * Sends the GET REV command to verify communication, returns the final
//    two digits of the Part Number (should be 7)
byte Si4707::initSi4707()
{
  // Set initial pin value: RST (Active-low reset)
  pinMode(SI4707_RESET, OUTPUT);  // Reset
  digitalWrite(SI4707_RESET, LOW);  // Keep the SI4707 in reset

  // Set initial pin values: SDIO and SCLK (serial data and clock lines)
  // Wire.begin() will take care of this
  Wire.begin();
  delay(1);  // Short delay before we take reset up

  // Raise RST, SCLK must not rise within 300ns before RST rises
  digitalWrite(SI4707_RESET, HIGH);
  delay(1);  // Give Si4707 a little time to reset

  // First, send the POWER UP command to turn on the Si4707.
  // The Si4707 must be powered up before sending any further commands.
  powerUp();

  // See Si4707_system_functions.ino for info on command_Get_Rev
  return command_Get_Rev(1);
}

// This function set up the Si4707 interrupts
void Si4707::setupInterrupts()
{
  // Configures the sources for the GPO2/IRQ interrupt pin, sets:
  // Sets SAME Interrupt Enable 0x04
  // ASQ Interrupt Enable 0x02
  setProperty(PROPERTY_GPO_IEN, 0x0006);

  // Configures interrupt related to the 1050 kHz alert tone, sets:
  // Interrupt Source Enable: Alert OFF 0x02
  // Interrupt Source Enable: Alert ON 0x01
  setProperty(PROPERTY_WB_ASQ_INTERRUPT_SOURCE, 0x0003);

  // Configures the SAME interrupt sources, sets:
  // Enable EOMDET as the source of SAME Interrupt 0x08
  // Enable SOMDET as the source of SAME Interrupt 0x04
  // Enable PREDET as the source of SAME Interrupt 0x02
  // Enable HDRRDY as the source of SAME Interrupt 0x01
  setProperty(PROPERTY_WB_SAME_INTERRUPT_SOURCE, 0x000F);
}

// Ths function sets the Si4707 to the freq it receives.
// freq should represent the frequency desired in kHz.
// e.g. 162400 will tune the radio to 162.400 MHz.
byte Si4707::setWBFrequency(unsigned long freq)
{
  // Keep tuned frequency between valid limits (162.4 - 162.55 MHz)
  long freqKhz = constrain(freq, 162400, 162550);;

  // Serial.print("Tuning to: ");
  // Serial.print((float) freqKhz / 1000.0, 4);
  // Serial.println(" MHz");

  // See si4707_system_functions.ino for info on command_Tune_Freq
  return command_Tune_Freq((freqKhz * 10)/25);
}

// This function does incremental tunes on the Si4707. Send a
// signed value representing how many increments you'd like to go
// (up is positive, down is negative). Each increment is 2.5kHz.
void Si4707::tuneWBFrequency(signed char increment)
{
  unsigned int freq = getWBFrequency();
  freq += increment;
  freq = constrain(freq, 64960, 65020);

  // Serial.print("Tuning to: ");
  // Serial.print((float) freq * 0.0025, 4);
  // Serial.println(" MHz");

  // See si4707_system_functions.ino for info on command_Tune_Freq
  command_Tune_Freq(freq);
}

// Returns the two-byte Si4707 representation of the frequency.
unsigned Si4707::getWBFrequency()
{
  // See si4707_system_functions.ino for info on command_Tune_Status
  return command_Tune_Status(0, 2);
}

// Returns true if the channel is still valid
boolean Si4707::getRSQ()
{
  // See si4707_system_functions.ino for info on command_RSQ_Status
  return command_RSQ_Status(2) & 0x1;
}

// Returns the recieved signal strength reported by Si4707
byte Si4707::getRSSI()
{
  // See si4707_system_functions.ino for info on command_RSQ_Status
  return command_RSQ_Status(4);
}

// Returns the signal-to-noise ratio reported by the Si4707
byte Si4707::getSNR()
{
  // See si4707_system_functions.ino for info on command_RSQ_Status
  return command_RSQ_Status(5);
}

// Returns the frequency offset reported by the Si4707
signed char Si4707::getFreqOffset()
{
  // See si4707_system_functions.ino for info on command_RSQ_Status
  return (signed char) command_RSQ_Status(7);
}

// Clear SAME interrupt and return the source
// 0 = Header Ready
// 1 = Preamble detected
// 2 = Start of message detected
// 3 = End of message detected
byte Si4707::getSAMEInt()
{
  byte buffer[14]; // Response buffer
  command_SAME_Status(1, 0, buffer);  // clear SAME interruption
  return buffer[1]; // Return first byte
}

// Returns the SAME state
// 0 = End of message
// 1 = Preamble detected
// 2 = Receiving SAME header message
// 3 = SAME header message complete
byte Si4707::getSAMEState()
{
  byte buffer[14]; // Response buffer
  command_SAME_Status(0, 0, buffer);
  return buffer[2]; // Return second byte
}

// Returns the SAME message size
byte Si4707::getSAMESize()
{
  byte buffer[14]; // Response buffer
  command_SAME_Status(0, 0, buffer);
  return buffer[3]; // Return third byte
}

// Returns the SAME message
void Si4707::getSAMEMessage(byte size, byte *message)
{
  byte returnAddress = 0; // Address counter
  byte buffer[14]; // Response buffer
  // Retrive full address
  for (int a = 0; a < size / 8; a++)
    {
      command_SAME_Status(0, returnAddress, buffer);
      for (int b = 0; b < 8; b++)
        {
          message[returnAddress] = buffer[b + 6];
          returnAddress++;
        }
    }
  // Retrive partitial address
  if (size % 8 > 0) {
    command_SAME_Status(0, returnAddress, buffer);
    for (int c = 0; c < size % 8; c++)
      {
        message[returnAddress] = buffer[c + 6];
        returnAddress++;
      }
  }
}

// Clear SAME message buffer
void Si4707::clearSAMEBuffer()
{
  byte buffer[14];
  command_SAME_Status(2, 0, buffer);
}

// Returns true if the 1050 kHz alert tone is present in weather band radio
byte Si4707::getASQ()
{
  return command_ASQ_Status(0, 2) & 0x1;
}

// Clear ASQ interrupt and return the source
byte Si4707::getASQInt()
{
  return command_ASQ_Status(1, 1);
}

// Depending on the value of the mute boolean, this function will either
//  mute (mute=1) or un-mute (mute=0) the Si4707's audio output.
void Si4707::setMuteVolume(boolean mute)
{
  // Mute left (bit 1) and right (bit 0) channels
  setProperty(PROPERTY_RX_HARD_MUTE, (mute<<1) | (mute<<0));
}

// This functionn interacts with the RX_VOLUME property of the Si4707.
//  Send a volume value (vol) between 0 (mute) and 63 (max volume).
void Si4707::setVolume(int vol)
{
  vol = constrain(vol, 0, 63); // vol should be between 0 and 63
  setProperty(PROPERTY_RX_VOLUME, vol);
}

// Set SNR threshold for valid channels
void Si4707::setSNR(unsigned int snr)
{
  snr = constrain(snr, 0, 127); // snr should be between 0 and 127
  setProperty(PROPERTY_WB_VALID_SNR_THRESHOLD, snr);
}

// Set RSSI threshold for valid channels
void Si4707::setRSSI(unsigned int rssi)
{
  rssi = constrain(rssi, 0, 127); // rssi should be between 0 and 127
  setProperty(PROPERTY_WB_VALID_RSSI_THRESHOLD, rssi);
}

// Get interrupt register
byte Si4707::getIntStatus()
{
  return command_Get_Int_Status();
}

/* Si4707_system_functions contains the nitty gritty system
   functions used to interface with the Si4707. High-level stuff,
   like constructing command strings, and parsing response strings.
   Also functions like sending commands, and waiting for the
   clear-to-send bit to be set.

   There are prettier wrappers for most of these functions in
   the example sketch.

   At the bottom of this sketch are functions used to interface
   with the Wire library - i2cWriteBytes and i2cReadBytes. */

//////////////////////////////
// High level functions //////
//////////////////////////////

/* powerup(): Sends the POWER_UP command to initiate the boot
   process, moving the Si4707 from powerdown to powerup mode.

   This performs critical operations in configuring the Si4707
   crystal oscillator, and turning on WB receive. */
void Si4707::powerUp(void)
{
  /* Power up (0x01) - initiate boot process.
     2 Arguments:
       ARG1: (CTSIEN)(GPO2OEN)(PATCH)(XOSCEN)(FUNC[3:0])
       ARG2: (OPMODE[7:0])
     Response: none (if FUNC = 3)*/
  cmd[0] = COMMAND_POWER_UP;  // Command 0x01: Power Up
  cmd[1] = 0x53;  // GP02 output enabled, crystal osc enabled, WB receive mode
  cmd[2] = 0x05;  // Use analog audio outputs
  writeCommand(3, cmd, 0, rsp);

  delay(POWER_UP_TIME_MS);
}

/* command_Tune_Freq(uint16_t frequency) sends the WB_TUNE_FREQ command
  with freqeuncy as the arguments. After tuning, the WB_TUNE_STATUS
  command is sent to verify whether or not tuning was succesful.

  frequency should be a 16-bit value equal to the freq. you want to
  tune to divided by 2.5kHz.
  For example, for 162.4MHz send 64960 (162400000 / 2500 = 64960)
  For 162.55 MHz send 65020 (162550000 / 2500 = 65020).

  Return RESP1 byte of WB_TUNE_STATUS response (valid and afcrl bits) */
byte Si4707::command_Tune_Freq(unsigned int frequency)
{
  cmd[0] = COMMAND_WB_TUNE_FREQ;
  cmd[1] = 0;
  cmd[2] = (uint8_t)(frequency >> 8);
  cmd[3] = (uint8_t)(frequency & 0x00FF);
  writeCommand(4, cmd, 1, rsp);

  byte status = 0;
  int i = 40; // wait 2 sec to tune
  while (--i && !(status&0x01)) {
    status = command_Get_Int_Status();
    delay(50);
  }

  return (command_Tune_Status(1, 1) >> 8);
}

/* WB_TUNE_STATUS (0x52) - Returns current frequency and RSSI/SNR
  at the moment of tune.
   Arguments (1 byte):
     (1) ARG1 - bit 0: INTACK (if set, clears seek/tune complete int)
   Response (6 bytes):
     (1) Status
     (2) bit 1: AFC rail indicator
         bit 0: Valid channel indicator
     (3) READFREQ(H) - Read frequency high byte
     (4) READFREQ(L) - Read frequency low byte
     (5) RSSI - Receive signal strength at frequency
     (6) SNR - Signal-to-noise ratio at frequency */
unsigned int Si4707::command_Tune_Status(byte intAck, byte returnIndex)
{
  cmd[0] = COMMAND_WB_TUNE_STATUS;
  cmd[1] = (intAck & 0x01);
  writeCommand(2, cmd, 6, rsp);

  return ((rsp[returnIndex] << 8) | rsp[returnIndex + 1]);
}

/* command_Get_Rev() sends the GET_REV command
   Response (9 bytes):
     (0) Status (Should have CTS bit (7) set)
     (1) PN[7:0] - Final two digits of part number
     (2) FWMAJOR[7:0] - Firmware major revision
     (3) FWMINOR[7:0] - Firmware minor revision
     (4) PATCH(H) - Patch ID high byte
     (5) PATCH(L) - Patch ID low byte
     (6) CMPMAJOR[7:0] - Component major revision
     (7) CMPMINOR[7:0] - Component minor revision
     (8) CHIPREV[7:0] - Chip revision

   Returns the response byte requested */
byte Si4707::command_Get_Rev(byte returnIndex)
{
  cmd[0] = COMMAND_GET_REV;
  writeCommand(1, cmd, 9, rsp);

  return rsp[returnIndex];
}

/* command_SAME_Status() sends the WB_SAME_STATUS command and
  returns the requested data address

  Arguments (2 bytes):
    (0): ()()()()()()(CLRBUF)(INTACK)
    (1): READADDR[7:0]
   Response (13 bytes):
    (0): Status byte
    (1): ()()()()(EMODET)(SOMDET)(PREDET)(HDRRDY)
    (2): STATE[7:0]
    (3): MSGLEN[7:0]
    (4): Confidence of data bytes 7-4
    (5): Confidence of data bytes 3-0
    (6-13): DATA0, DATA1, ..., DATA7 */
void Si4707::command_SAME_Status(byte setArgument, byte returnAddress, byte * returnData)
{
  cmd[0] = COMMAND_WB_SAME_STATUS;
  cmd[1] = setArgument;
  cmd[2] = returnAddress;
  writeCommand(3, cmd, 14, returnData);
}

/* WB_RSQ_STATUS (0x53) - Returns status information about
  received signal quality - RSSI, SNR, freq offset.
   Argument (1 byte):
     (0) bit 0: Interrupt acknowledge
   Response (8 bytes):
     (0) Status (Should have CTS bit (7) set)
     (1) RESP1: ()()()()(SNRHINT)(SNRLINT)(RSSIHINT)(RSSILINT)
     (2) RESP2: ()()()()()()(AFCRL)(VALID)
     (3) Nothing
     (4) RSSI[7:0] - Received signal strength indicator
     (5) SNR[7:0] - Signal-to-noise metric
     (6) Nothing
     (7) FREQOFF[7:0] - signed frequency offset in kHz  */
byte Si4707::command_RSQ_Status(byte returnIndex)
{

  cmd[0] = COMMAND_WB_RSQ_STATUS;
  cmd[1] = 0;

  writeCommand(2, cmd, 8, rsp);

  return rsp[returnIndex];
}

/* WB_ASQ_STATUS (0x55) - Returns status information about
  the 1050 kHz alert tone in weather band radio
   Argument (1 byte):
     (0) bit 0: Interrupt acknowledge
   Response (3 bytes):
     (0) Status (Should have CTS bit (7) set)
     (1) RESP1: ()()()()()()(ALERTOFF_INT)(ALERTON_INT)
     (2) RESP2: ()()()()()()()(ALERT) */
byte Si4707::command_ASQ_Status(byte setArgument, byte returnIndex)
{

  cmd[0] = COMMAND_WB_ASQ_STATUS;
  cmd[1] = setArgument;

  writeCommand(2, cmd, 3, rsp);

  return rsp[returnIndex];
}

/* command_Get_Int_Status() sends the GET_INT_STATUS command and returns
  the byte from the commands response
*/
byte Si4707::command_Get_Int_Status(void)
{
  cmd[0] = COMMAND_GET_INT_STATUS;
  cmd[1] = 0;
  rsp[1] = 0;

  writeCommand(1, cmd, 1, rsp);

  return rsp[0];
}

// This function will wait until the CTS (clear-to-send) bit has
// been set (or will timeout after 500 ms)
void Si4707::waitForCTS(void)
{
  byte status = 0;
  int i = 1000;

  while (--i && !(status&0x80))
  {
    i2cReadBytes(1, &status);
    delayMicroseconds(500);
  }
}

// This function will write a command string with a given size
// And retrun a filled reply string with replySize bytes in it.
void Si4707::writeCommand(byte cmdSize, byte * command, byte replySize, byte * reply)
{
  waitForCTS();
  i2cWriteBytes(cmdSize, command);
  waitForCTS();
  if (replySize)
  {
    i2cReadBytes(replySize, reply);
  }

  for (int i=0; i<replySize; i++)
  {
    reply[i] += 1;
    reply[i] -= 1;
  }
}

// This function returns a two-byte property from the requested
// property address.
unsigned int Si4707::getProperty(unsigned int property)
{
  cmd[0] = COMMAND_GET_PROPERTY;
  cmd[1] = 0;
  cmd[2] = (byte)((property & 0xFF00) >> 8);
  cmd[3] = (byte)(property & 0x00FF);
  writeCommand(4, cmd, 4, rsp);

  return ((rsp[2] << 8) | rsp[3]);
}

// This function sets a property to the given two-byte property
//  value.
void Si4707::setProperty(unsigned int propNumber, unsigned int propValue)
{
  cmd[0] = COMMAND_SET_PROPERTY;
  cmd[1] = 0;
  cmd[2] = (uint8_t)(propNumber >> 8);
  cmd[3] = (uint8_t)(propNumber & 0x00FF);
  cmd[4] = (uint8_t)(propValue >> 8);
  cmd[5] = (uint8_t)(propValue & 0x00FF);

  writeCommand(6, cmd, 1, rsp);
}

//////////////////////////////
// Low level functions ///////
//////////////////////////////

// Read a specified number of bytes via the I2C bus.
// Will timeout if there is no response from the address.
void Si4707::i2cReadBytes(byte number_bytes, byte *data_in)
{
  int timeout = 100000;

  Wire.requestFrom((byte) SI4707_ADDRESS, number_bytes);
  while ((Wire.available() < number_bytes) && (--timeout));
  while((number_bytes--) && timeout)
  {
    *data_in++ = Wire.read();
  }
  Wire.endTransmission();
}

// Write a specified number of bytes to the Si4707.
void Si4707::i2cWriteBytes(uint8_t number_bytes, uint8_t *data_out)
{
  Wire.beginTransmission(SI4707_ADDRESS);
  while (number_bytes--)
  {
    Wire.write(*data_out++);
  }
  Wire.endTransmission();
}
