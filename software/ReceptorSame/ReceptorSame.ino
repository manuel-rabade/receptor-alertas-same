#include "Si4707.h"
#include "IO.h"
#include "Command.h"
#include "Config.h"

// configuración
#define CONFIG_VERSION 0x07 // versión memoria
#define SAME_TIMEOUT 6000 // tiempo de espera maximo para recibir un mensaje same (segundos)

// maquina de estados same
#define SAME_EOM_DET 0
#define SAME_PRE_DET 1
#define SAME_HDR_DET 2
#define SAME_HDR_RDY 3

// constantes globales
const unsigned long sameChannels[] = { // canales same (1 a 7) -> frecuencias radio (hertz)
  162400, 162425, 162450,
  162475, 162500, 162525,
  162550
};
const byte radioVolumes[] = { // volumen configuración (0 a 10) -> volumen radio (0 a 63)
  0, 6, 13, 19, 25, 32,
  38, 44, 50, 57, 63
};

// objetos globales
Si4707 radio;
IO io;
Command cmd;
Config config;

// variables globales
boolean asqPrevStatus = 0; // estado asq
byte samePrevState = 0; // estado same
byte sameHeadersCount = 0; // conteo de cabeceras
unsigned long sameTimer = 0; // timeout mensaje same

// obsoleto
// #define SAME_TEST_TIMEOUT 11400000 // 3 horas y 10 minutos
// #define ALERT_TIMEOUT 60000 // 1 minuto
// unsigned long same_test_timer;
// boolean alert_state;
// unsigned long alert_timer;

void setup() {
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
    // TODO: io.ledStartup
    io.ledSlow(0);
    io.ledOff(1);
  } else {
    Serial.print(F("SI4707_ERROR"));
    // TODO: io.ledSysError
    io.ledFast(0);
    io.ledFast(1);
    while (1) {
      io.ledRefresh();
      delay(50);
    }
  }

  // aplicar configuacion
  updateSettings();
  Serial.println(F("RUN"));
}

void loop() {
  // monitoreo tono 1050 khz
  boolean asqStatus = radio.getASQ();
  if (asqPrevStatus != asqStatus) {
    if (asqStatus) {
      Serial.println("ASQ_ON");
      // TODO
    } else {
      Serial.println("ASQ_OFF");
      // TODO
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
      // ¿se recibieron cabeceras?
      if (sameHeadersCount > 0) {
        sameMessage();
      } else {
        sameReset();
      }
      break;
    case SAME_PRE_DET:
      // preámbulo detectado
      sameTimer = millis();
      Serial.println("SAME_PRE_DET");
      break;
    case SAME_HDR_DET:
      // cabecera detectada
      sameTimer = millis();
      Serial.println("SAME_HDR_DET");
      break;
    case SAME_HDR_RDY:
      // cabecera lista
      sameTimer = millis();
      sameHeadersCount++;
      Serial.print("SAME_HDR_RDY,");
      Serial.println(sameHeadersCount);
      break;
    }
    // ¿se recibieron tres cabeceras?
    if (sameHeadersCount == 3) {
      sameMessage();
    }
    samePrevState = sameState;
    return;
  }

  // timeout mensaje same
  if (sameTimer && millis() - sameTimer > SAME_TIMEOUT) {
    Serial.println("SAME_TIMEOUT");
    // ¿se recibieron cabeceras?
    if (sameHeadersCount > 0) {
      sameMessage();
    } else {
      sameReset();
    }
  }

  // timeout prueba same
  // if (same_test_timer && millis() - same_test_timer > SAME_TEST_TIMEOUT) {
  //   same_test_timer = 0;
  //   io.ledOff(1);
  //   Serial.println("SAME_TEST_TIMEOUT");
  // }

  // timeout alerta
  // if (alert_state && millis() - alert_timer > ALERT_TIMEOUT) {
  //   alert_timer = 0;
  //   alertOff();
  //   Serial.println("ALERT_TIMEOUT");
  // }

  // procesar comandos seriales
  if (cmd.isReady()) {
    commands();
    cmd.clearBuffer();
  }

  // boton de usuario
  if (io.isButtonTriggered()) {
    Serial.println("BUTTON_TEST");
    alertOn();
  }

  // refrescar leeds
  io.ledRefresh();
  delay(50);
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
// mensajes same

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

  // TODO: mensajes, alertas, etc
  // prueba periódica
  if (msg[5] == 'R' &&
      msg[6] == 'W' &&
      msg[7] == 'T') {
    //same_test_timer = millis();
    io.ledOn(1);
    Serial.println("SAME_RWT");
  }
}

// reinicio buffer, timer y contador same
void sameReset() {
  radio.clearSAMEBuffer();
  sameTimer = 0;
  sameHeadersCount = 0;
}

// alerta
void alertOn() {
  //alert_state = 1;
  //alert_timer = millis();
  io.ledFast(0);
  io.relayOn();
  Serial.println("ALERT_ON");
}

void alertOff() {
  //alert_state = 0;
  io.ledSlow(0);
  io.relayOff();
  Serial.println("ALERT_OFF");
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
  case CMD_AUDIO:
    if (cmd.isArg()) {
      byte audio = cmd.getArgByte();
      if (config.setAudio(audio)) {
        // TODO
        break;
      }
    }
    Serial.println(F("AUDIO_ERROR"));
    break;
  case CMD_RELAY:
    if (cmd.isArg()) {
      byte relay = cmd.getArgByte();
      if (config.setRelay(relay)) {
        // TODO
        break;
      }
    }
    Serial.println(F("RELAY_ERROR"));
    break;
  case CMD_RMT_PERIOD:
    if (cmd.isArg()) {
      unsigned int period = cmd.getArgUInt();
      config.setRmtPeriod(period);
      // TODO
    } else {
      Serial.println(F("RMT_PERIOD_ERROR"));
    }
    break;
  case CMD_RWT_PERIOD:
    if (cmd.isArg()) {
      unsigned int period = cmd.getArgUInt();
      config.setRwtPeriod(period);
      // TODO
    } else {
      Serial.println(F("RWT_PERIOD_ERROR"));
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
  config.setAudio(2);
  config.setRelay(3);
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
  Serial.print(F("CONFIG_AUDIO,"));
  Serial.println(config.getAudio());
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
