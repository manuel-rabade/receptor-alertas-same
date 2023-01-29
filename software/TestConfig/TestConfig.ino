#include "Config.h"

Config config = Config();

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");

  Serial.print("MEM_BYTES,");
  Serial.println(config.bytes());

  Serial.print("MEM_VERSION,");
  Serial.println(config.getVersion());

  if (config.getVersion() != 0x12) {
    Serial.println("SET_DEFAULTS");
    config.setVersion(0x03);
    config.setChannel(7);
    config.setMute(false);
    config.setVolume(50);
    config.setRelay(3);
    config.setCDMX(true);
    config.setRwtPeriod(0);
    config.setRmtPeriod(0);
    config.emptyAreaCodes();
    testSetAreaCode("123456");
    dumpAreaCodes();
    config.emptyEventCodes();
    testSetEventCode("ABC");
    dumpEventCodes();
    Serial.println("MEM_SAVE");
    config.save();
  }

  Serial.println("MEM_RELOAD");
  config.reload();

  Serial.println("DUMP_START");
  Serial.print("MEM_VERSION,");
  Serial.println(config.getVersion());
  Serial.print("MEM_CHANNEL,");
  Serial.println(config.getChannel());
  Serial.print("MEM_MUTE,");
  Serial.println(config.getMute());
  Serial.print("MEM_VOLUME,");
  Serial.println(config.getVolume());
  Serial.print("MEM_RELAY,");
  Serial.println(config.getRelay());
  Serial.print("MEM_CDMX,");
  Serial.println(config.getCDMX());
  Serial.print("MEM_RWT_PERIOD,");
  Serial.println(config.getRwtPeriod());
  Serial.print("MEM_RMT_PERIOD,");
  Serial.println(config.getRmtPeriod());
  dumpAreaCodes();
  dumpEventCodes();
  Serial.println("DUMP_END");

  Serial.println("TEST_AREA_CODES_START");
  config.emptyAreaCodes();
  testSetAreaCode("555555"); // 5
  testSetAreaCode("333333"); // 3
  testSetAreaCode("222222"); // 2
  testSetAreaCode("444444"); // 4
  testSetAreaCode("000000"); // 1
  testSetAreaCode("567890"); // 6
  testSetAreaCode("333333"); // error
  dumpAreaCodes();
  testFindAreaCode("555555");
  testFindAreaCode("      ");
  testFindAreaCode("111111");
  testFindAreaCode("033333"); // comodin area code
  testFindAreaCode("900000"); // comodin area code
  testFindAreaCode("088888"); // comodin area code
  testClearAreaCode("444444");
  testClearAreaCode("      ");
  testClearAreaCode("666666");
  dumpAreaCodes();
  Serial.println("TEST_AREA_CODES_END");

  Serial.println("TEST_EVENT_CODES_START");
  config.emptyEventCodes();
  testSetEventCode("YZ-"); // 12
  testSetEventCode("MNO"); // 8
  testSetEventCode("STU"); // 10
  testSetEventCode("PQR"); // 9
  testSetEventCode("VWX"); // 11
  testSetEventCode("JKL"); // 7
  testSetEventCode("GHI"); // 6
  testSetEventCode("DEF"); // 5
  testSetEventCode("ABC"); // 4
  testSetEventCode("123"); // 1
  testSetEventCode("789"); // 3
  testSetEventCode("456"); // 2
  testSetEventCode("VWX"); // error
  dumpEventCodes();
  testFindEventCode("JKL");
  testFindEventCode("   ");
  testFindEventCode("ASD");
  testClearEventCode("ABC");
  testClearEventCode("   ");
  testClearEventCode("QWE");
  dumpEventCodes();
  Serial.println("TEST_EVENT_CODES_END");

  Serial.println("TEST_VOLUME_START");
  Serial.println("TEST_VOLUME,11,false");
  config.setVolume(11);
  config.setMute(false);
  dumpVolume();
  Serial.println("TEST_VOLUME,11,true");
  config.setMute(true);
  dumpVolume();
  Serial.println("TEST_VOLUME,44,true");
  config.setVolume(44);
  dumpVolume();
  Serial.println("TEST_VOLUME,22,false");
  config.setVolume(22);
  config.setMute(false);
  dumpVolume();
  Serial.println("TEST_VOLUME,22,true");
  config.setMute(true);
  dumpVolume();
  Serial.println("TEST_VOLUME_END");
}

void loop() {

}

void dumpVolume() {
  Serial.print("DUMP_VOLUME,");
  Serial.print(config.getVolume());
  Serial.print(",");
  Serial.println(config.getMute());
}

void dumpAreaCodes() {
  Serial.print("DUMP_AREA_CODES,");
  Serial.println(config.countAreaCodes());
  for (byte idx = 0; idx < config.countAreaCodes(); idx++) {
    Serial.print("DUMP_AREA_CODE,");
    Serial.print(idx);
    Serial.print(",");
    char* code = config.strAreaCode(idx);
    Serial.println(code);
    free(code);
  }
}

void testFindAreaCode(char code[3]) {
  Serial.print("FIND_AREA,");
  Serial.println(code);
  if (config.findAreaCodeWildcard(code)) {
    Serial.println("AREA_FOUND");
  } else {
    Serial.println("AREA_NOT_FOUND");
  }
}

void testSetAreaCode(char code[3]) {
  Serial.print("SET_AREA,");
  Serial.println(code);
  if (config.setAreaCode(code)) {
    Serial.println("AREA_SET");
  } else {
    Serial.println("AREA_NOT_SET");
  }
}

void testClearAreaCode(char code[3]) {
  Serial.print("CLEAR_AREA,");
  Serial.println(code);
  if (config.clearAreaCode(code)) {
    Serial.println("AREA_CLEAR");
  } else {
    Serial.println("AREA_NOT_CLEAR");
  }
}


void dumpEventCodes() {
  Serial.print("DUMP_EVENT_CODES,");
  Serial.println(config.countEventCodes());
  for (byte idx = 0; idx < config.countEventCodes(); idx++) {
    Serial.print("DUMP_EVENT_CODE,");
    Serial.print(idx);
    Serial.print(",");
    char* code = config.strEventCode(idx);
    Serial.println(code);
    free(code);
  }
}

void testFindEventCode(char code[3]) {
  Serial.print("FIND_EVENT,");
  Serial.println(code);
  if (config.findEventCode(code)) {
    Serial.println("EVENT_FOUND");
  } else {
    Serial.println("EVENT_NOT_FOUND");
  }
}

void testSetEventCode(char code[3]) {
  Serial.print("SET_EVENT,");
  Serial.println(code);
  if (config.setEventCode(code)) {
    Serial.println("EVENT_SET");
  } else {
    Serial.println("EVENT_NOT_SET");
  }
}

void testClearEventCode(char code[3]) {
  Serial.print("CLEAR_EVENT,");
  Serial.println(code);
  if (config.clearEventCode(code)) {
    Serial.println("EVENT_CLEAR");
  } else {
    Serial.println("EVENT_NOT_CLEAR");
  }
}

// Local Variables:
// mode: c++
// End:
