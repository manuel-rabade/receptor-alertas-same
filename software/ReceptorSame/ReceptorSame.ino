#include "Si4707.h"
#include "MockRadio.h"
#include "IO.h"
#include "Command.h"
#include "Config.h"

// configuración
#define CONFIG_VERSION 0x12 // versión memoria
#define SAME_TIMEOUT 6000 // tiempo de espera maximo para recibir un mensaje same completo (segundos)

// maquina de estados same
#define SAME_EOM_DET 0
#define SAME_PRE_DET 1
#define SAME_HDR_DET 2
#define SAME_HDR_RDY 3

// constantes globales
const unsigned long sameChannels[] = { // canales same (1 a 7) -> frecuencias radio (hertz)
  162400, 162425, 162450, 162475, 162500, 162525, 162550 };
const byte radioVolumes[] = { // volumen configuración (0 a 10) -> volumen radio (0 a 63)
  0, 6, 13, 19, 25, 32, 38, 44, 50, 57, 63 };

// radio
#if MOCKRADIO == 0 // 0 = si4707, > 0 caso de prueba con radio simulado
Si4707 radio;
#else
MockRadio radio; // radio simulado
#endif

// objetos globales
IO io;
Command cmd;
Config config;

// variables globales
boolean asqPrevStatus = 0; // estado asq
byte samePrevState = 0; // estado same
byte sameHeadersCount = 0; // conteo de cabeceras
boolean sameTimerEnabled = false;  // timer mensaje same
unsigned long sameTimer = 0;
boolean sameAlert = false; // estado alerta
boolean testAlert = false; // estado prueba de alerta
boolean relayState = false; // status relay
boolean audioState = false; // status audio

// pruebas same
boolean sameRwtEnabled = false;  // timer prueba semanal
unsigned long sameRwtTimer = 0;
boolean sameRmtEnabled = false;  // timer prueba mensual
unsigned long sameRmtTimer = 0;

void setup() {
  // inicio leds
  io.ledsBegin();

  // inicia puerto serial
  Serial.begin(9600);
  Serial.println(F("STARTUP"));

  // verificacion de memoria
  Serial.print(F("CONFIG_VERSION,"));
  Serial.println(config.getVersion());
  if (config.getVersion() != CONFIG_VERSION) {
    configDefaults();
    configSave();
  }

  // inicio si4707
  if (radio.begin()) {
    Serial.println(F("SI4707_OK"));
  } else {
    Serial.print(F("SI4707_ERROR"));
    io.ledsError();
    while (1) {
      io.ledsRefresh();
      delay(50);
    }
  }

  // aplicar configuacion
  updateSettings();
  io.ledsWait();
  Serial.println(F("RUN"));
}

void loop() {
  // monitor tono 1050 khz
  boolean asqStatus = radio.getASQ();
  if (asqPrevStatus != asqStatus) {
    if (asqStatus) {
      Serial.println("ASQ_ON");
      audioOn();
      sameReset();
    } else {
      Serial.println("ASQ_OFF");
    }
    asqPrevStatus = asqStatus;
  }

  // recepción mensajes same
  byte sameState = radio.getSAMEState();
  if (samePrevState != sameState) {
    switch (sameState) {
    case SAME_EOM_DET:
      // fin del mensaje
      Serial.println("SAME_EOM_DET");
      sameEnd();
      break;
    case SAME_PRE_DET:
      // preámbulo detectado
      sameRefreshTimer();
      Serial.println("SAME_PRE_DET");
      break;
    case SAME_HDR_DET:
      // cabecera detectada
      sameRefreshTimer();
      Serial.println("SAME_HDR_DET");
      break;
    case SAME_HDR_RDY:
      // cabecera lista
      sameRefreshTimer();
      sameHeadersCount++;
      Serial.print("SAME_HDR_RDY,");
      Serial.println(sameHeadersCount);
      // ¿se recibieron 3 cabeceras?
      if (sameHeadersCount == 3) {
        sameMessage();
      }
      break;
    }
    samePrevState = sameState;
  }

  // timeout mensaje same
  if (sameTimerEnabled && millis() - sameTimer > SAME_TIMEOUT) {
    Serial.println("SAME_TIMEOUT");
    // ¿se recibieron al menos 2 cabeceras?
    if (sameHeadersCount > 1) {
      sameMessage();
    } else {
      sameReset();
    }
  }

  // timeout prueba semanal
  if (sameRwtEnabled && millis() - sameRwtTimer > config.getRwtPeriodMillis()) {
    sameRwtEnabled = false;
    sameRwtTimer = 0;
    Serial.println("SAME_RWT_TIMEOUT");
    io.ledsWait();
  }

  // timeout prueba mensual
  if (sameRmtEnabled && millis() - sameRmtTimer > config.getRmtPeriodMillis()) {
    sameRmtEnabled = false;
    sameRmtTimer = 0;
    Serial.println("SAME_RMT_TIMEOUT");
    io.ledsWait();
  }

  // procesar comandos seriales
  if (cmd.isReady()) {
    commands();
    cmd.clearBuffer();
  }

  // boton de usuario
  if (io.isButtonTriggered()) {
    Serial.println("BUTTON_TRIGGERED");
    if (sameAlert) {
      audioOff();
      relayOff();
      Serial.println("SAME_ALERT_OFF");
      sameAlert = false;
      if (sameRwtEnabled || sameRmtEnabled) {
        io.ledsWaitRT();
      } else {
        io.ledsWait();
      }
    } else {
      if (testAlert) {
        Serial.println("TEST_ALERT_OFF");
        testAlert = false;
        audioOff();
        relayOff();
        if (sameRwtEnabled || sameRmtEnabled) {
          io.ledsWaitRT();
        } else {
          io.ledsWait();
        }
      } else {
        Serial.println("TEST_ALERT_ON");
        testAlert = true;
        io.ledsAlert();
        audioOn();
        relayOn();
      }
    }
  }

  // refrescar leds
  io.ledsRefresh();
  delay(50);
}

// ---------------------------------------------------------------------------
// same

// procesar mensaje same
void sameMessage() {
  byte size = radio.getSAMESize();
  // ¿mensaje vacío?
  if (size < 1) {
    Serial.println("SAME_EMPTY");
    return;
  }

  // adquisición mensaje
  byte msg[size];
  radio.getSAMEMessage(size, msg);
  sameReset();

  // reportamos mensaje
  Serial.print("SAME_MESSAGE,");
  for (byte i = 0; i < size; i++) {
    if (msg[i] > 31 && msg[i] < 127) {
      Serial.write(msg[i]);
    } else {
      Serial.print(".");
    }
  }
  Serial.println();

  // evento del mensaje
  char event[3] = { msg[5], msg[6], msg[7] };

  // areas del mensaje
  char areas[31][6];
  byte areasCount = 0;
  while (areasCount < 31) {
    byte offset = 8 + areasCount * 7;
    if (strncmp(msg + offset, "-", 1) == 0) {
      strncpy(areas[areasCount], msg + offset + 1, 6);
      areasCount++;
    } else {
      break;
    }
  }

  // filtramos codigo de area
  if (config.countAreaCodes() > 0){
    boolean areaFlag = false;
    for (byte i; i < areasCount; i++) {
      if (config.findAreaCodeWildcard(areas[i])) {
        areaFlag = true;
        break;
      }
    }
    if (!areaFlag) {
      Serial.println("SAME_IGNORE_AREA");
      return;
    }
  }

  // mensaje de prueba
  if (strncmp(event, "RWT", 3) == 0) {
    Serial.println("SAME_RWT");
    if (config.getRwtPeriod() > 0) {
      sameRwtEnabled = true;
      sameRwtTimer = millis();
      io.ledsWaitRT();
    }
    return;
  } else if (strncmp(event, "RMT", 3) == 0) {
    Serial.println("SAME_RMT");
    if (config.getRmtPeriod() > 0) {
      sameRmtEnabled = true;
      sameRmtTimer = millis();
      io.ledsWaitRT();
    }
    return;
  }

  // filtramos codigo de evento
  if (config.countEventCodes() > 0 && !config.findEventCode(event)) {
    Serial.println("SAME_IGNORE_EVENT");
    return;
  }

  // disparamos alerta
  Serial.println("SAME_ALERT_ON");
  sameAlert = true;
  io.ledsAlert();
  if (config.getRelay() > 1) {
    relayOn();
  }
  audioOn();
}

// fin mensaje same
void sameEnd() {
  if (sameAlert) {
    audioOff();
    if (config.getRelay() == 2) {
      relayOff();
    }
    if (config.getRelay() < 3) {
      Serial.println("SAME_ALERT_OFF");
      sameAlert = false;
      if (sameRwtEnabled || sameRmtEnabled) {
        io.ledsWaitRT();
      } else {
        io.ledsWait();
      }
    }
  }
  // ¿se recibieron 2 cabeceras?
  if (sameHeadersCount == 2) {
    sameMessage();
  }
  sameReset();
}

// actualizar timer same
void sameRefreshTimer() {
  sameTimerEnabled = true;
  sameTimer = millis();
}

// reiniciar buffer, timer y contador same
void sameReset() {
  radio.clearSAMEBuffer();
  sameTimerEnabled = false;
  sameTimer = 0;
  sameHeadersCount = 0;
}

// ---------------------------------------------------------------------------
// disparo audio y relay

void audioOn() {
  if (!audioState) {
    Serial.println("AUDIO_ON");
    radio.setMuteVolume(false);
    audioState = true;
    if (config.getRelay() == 1) {
      relayOn();
    }
  }
}

void audioOff() {
  if (audioState) {
    Serial.println("AUDIO_OFF");
    radio.setMuteVolume(true);
    audioState = false;
    if (config.getRelay() == 1) {
      relayOff();
    }
  }
}

void relayOn() {
  if (!relayState) {
    Serial.println("RELAY_ON");
    io.relayOn();
    relayState = true;
  }
}

void relayOff() {
  if (relayState) {
    Serial.println("RELAY_OFF");
    io.relayOff();
    relayState = false;
  }
}

// ---------------------------------------------------------------------------
// aplicar configuracion

// todas las configuraciones
void updateSettings() {
  updateMute();
  updateVolume();
  updateChannel();
}

// ajuste de mute
void updateMute() {
  radio.setMuteVolume(config.getMute());
  Serial.print(F("MUTE,"));
  Serial.println(config.getMute());
}

// ajuste de volumen
void updateVolume() {
  radio.setVolume(radioVolumes[config.getVolume()]);
  Serial.print(F("VOLUME,"));
  Serial.println(config.getVolume());
}

// sintonizar canal
void updateChannel() {
  if (radio.setWBFrequency(sameChannels[config.getChannel() - 1])) {
    Serial.print(F("TUNE_OK,"));
  } else {
    Serial.print(F("TUNE_ERROR,"));
  }
  Serial.println(config.getChannel());
}

// ---------------------------------------------------------------------------
// configuración y comandos

void commands() {
  switch (cmd.getCmd()) {
  case CMD_FREQUENCY:
    Serial.print(F("FREQUENCY,"));
    Serial.println((float) radio.getWBFrequency() * 0.0025, 4);
    break;
  case CMD_QUALITY:
    Serial.print(F("QUALITY,"));
    Serial.print(radio.getRSSI());
    Serial.print(F(","));
    Serial.println(radio.getSNR());
    break;
  case CMD_CHANNEL:
    if (cmd.isArg()) {
      byte channel = cmd.getArgByte();
      if (config.setChannel(channel)) {
        updateChannel();
        break;
      }
    }
    Serial.println(F("CHANNEL_ERROR"));
    break;
  case CMD_MUTE:
    if (cmd.isArg()) {
      byte mute = cmd.getArgByte();
      if (mute == 1) {
        config.setMute(true);
        updateMute();
        break;
      } else if (mute == 0) {
        config.setMute(false);
        updateMute();
        break;
      }
    }
    Serial.println(F("MUTE_ERROR"));
    break;
  case CMD_VOLUME:
    if (cmd.isArg()) {
      byte volume = cmd.getArgByte();
      if (config.setVolume(volume)) {
        updateVolume();
        break;
      }
    }
    Serial.println(F("VOLUME_ERROR"));
    break;
  case CMD_RELAY:
    if (cmd.isArg()) {
      byte relay = cmd.getArgByte();
      if (config.setRelay(relay)) {
        Serial.print(F("RELAY,"));
        Serial.println(relay);
        break;
      }
    }
    Serial.println(F("RELAY_ERROR"));
    break;
  case CMD_RWT_PERIOD:
    if (cmd.isArg()) {
      unsigned int period = cmd.getArgUInt();
      config.setRwtPeriod(period);
      Serial.print(F("RWT_PERIOD,"));
      Serial.println(period);
    } else {
      Serial.println(F("RWT_PERIOD_ERROR"));
    }
    break;
  case CMD_RMT_PERIOD:
    if (cmd.isArg()) {
      unsigned int period = cmd.getArgUInt();
      config.setRmtPeriod(period);
      Serial.print(F("RMT_PERIOD,"));
      Serial.println(period);
    } else {
      Serial.println(F("RMT_PERIOD_ERROR"));
    }
    break;
  case CMD_DEFAULTS:
    configDefaults();
    updateSettings();
    break;
  case CMD_RELOAD:
    Serial.println(F("MEM_RELOAD"));
    config.reload();
    updateSettings();
    break;
  case CMD_SAVE:
    configSave();
    break;
  case CMD_DUMP:
    configDump();
    break;
  case CMD_AREA_ADD:
    if (!cmd.isArg()) {
      Serial.println(F("AREA_ADD_ERROR"));
    } else {
      char* code = cmd.getArg();
      if (config.setAreaCode(code)) {
        Serial.print(F("AREA_ADD_OK,"));
        Serial.println(code);
      } else {
        Serial.println(F("AREA_ADD_ERROR"));
      }
      free(code);
    }
    break;
  case CMD_AREA_DEL:
    if (!cmd.isArg()) {
      Serial.println(F("AREA_DEL_ERROR"));
    } else {
      char* code = cmd.getArg();
      if (config.clearAreaCode(code)) {
        Serial.print(F("AREA_DEL_OK,"));
        Serial.println(code);
      } else {
        Serial.println(F("AREA_DEL_ERROR"));
      }
      free(code);
    }
    break;
  case CMD_EVENT_ADD:
    if (!cmd.isArg()) {
      Serial.println(F("EVENT_ADD_ERROR"));
    } else {
      char* code = cmd.getArg();
      if (config.setEventCode(code)) {
        Serial.print(F("EVENT_ADD_OK,"));
        Serial.println(code);
      } else {
        Serial.println(F("EVENT_ADD_ERROR"));
      }
      free(code);
    }
    break;
  case CMD_EVENT_DEL:
    if (!cmd.isArg()) {
      Serial.println(F("EVENT_DEL_ERROR"));
    } else {
      char* code = cmd.getArg();
      if (config.clearEventCode(code)) {
        Serial.print(F("EVENT_DEL_OK,"));
        Serial.println(code);
      } else {
        Serial.println(F("EVENT_DEL_ERROR"));
      }
      free(code);
    }
    break;
  default:
    Serial.println(F("UNKNOWN_COMMAND"));
  }
}

void configDefaults() {
  Serial.println(F("CONFIG_DEFAULTS"));
  config.setVersion(CONFIG_VERSION);
  config.setChannel(7);
  config.setMute(true);
  config.setVolume(10);
  config.setRelay(1);
  config.setSASMEX(true);
  config.setRwtPeriod(0);
  config.setRmtPeriod(0);
  config.emptyAreaCodes();
  config.emptyEventCodes();
}

void configSave() {
  Serial.println(F("MEM_SAVE"));
  config.save();
}

void configDump() {
  Serial.print(F("CONFIG_VERSION,"));
  Serial.println(config.getVersion());
  Serial.print(F("CONFIG_CHANNEL,"));
  Serial.println(config.getChannel());
  Serial.print(F("CONFIG_MUTE,"));
  Serial.println(config.getMute());
  Serial.print(F("CONFIG_VOLUME,"));
  Serial.println(config.getVolume());
  Serial.print(F("CONFIG_RELAY,"));
  Serial.println(config.getRelay());
  Serial.print(F("CONFIG_RWT_PERIOD,"));
  Serial.println(config.getRwtPeriod());
  Serial.print(F("CONFIG_RMT_PERIOD,"));
  Serial.println(config.getRmtPeriod());
  Serial.print(F("CONFIG_AREA_CODES,"));
  Serial.println(config.countAreaCodes());
  for (byte idx = 0; idx < config.countAreaCodes(); idx++) {
    Serial.print(F("CONFIG_AREA_CODE,"));
    Serial.print(idx);
    Serial.print(F(","));
    char* code = config.strAreaCode(idx);
    Serial.println(code);
    free(code);
  }
  Serial.print(F("CONFIG_EVENT_CODES,"));
  Serial.println(config.countEventCodes());
  for (byte idx = 0; idx < config.countEventCodes(); idx++) {
    Serial.print(F("CONFIG_EVENT_CODE,"));
    Serial.print(idx);
    Serial.print(F(","));
    char* code = config.strEventCode(idx);
    Serial.println(code);
    free(code);
  }
}

// Local Variables:
// mode: c++
// End:
