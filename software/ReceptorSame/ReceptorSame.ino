#include "Si4707.h"
#include "IO.h"
#include "Command.h"
#include "Config.h"

// parametors same
#define SAME_CHANNELS 162400,162425,162450,162475,162500,162525,162550
#define SAME_TIMEOUT 6000 // tiempo de espera maximo para recibir un mensaje same (segundos)

// maquina de estados same
#define SAME_EOM_DET 0
#define SAME_PRE_DET 1
#define SAME_HDR_DET 2
#define SAME_HDR_RDY 3

// objetos globales
Si4707 radio;
IO io;
Command cmd;
Config config;

// constantes y variables globales
const unsigned long same_channels[] = { SAME_CHANNELS }; // canales same
boolean asq_prev_status = 0; // estado asq
byte same_prev_state = 0; // estado same
byte same_headers_count = 0; // conteo de cabeceras
unsigned long same_timer = 0; // timeout mensaje same

// obsoleto
// #define SAME_TEST_TIMEOUT 11400000 // 3 horas y 10 minutos
// #define ALERT_TIMEOUT 60000 // 1 minuto
// unsigned long same_test_timer;
// boolean alert_state;
// unsigned long alert_timer;

void setup() {
  // inicia puerto serial
  Serial.begin(9600);
  Serial.println("SETUP");

  // verificacion de memoria
  Serial.print("MEM_VERSION,");
  Serial.println(config.getVersion());
  if (config.getVersion() != 0x02) {
    Serial.println("LOAD_DEFAULTS");
    config.setVersion(0x03);
    config.setChannel(7);
    config.setMute(true);
    config.setVolume(63);
    config.setAudio(2);
    config.setRelay(3);
    config.setRwtDuration(0);
    config.setRmtDuration(0);
    config.emptyAreaCodes();
    config.emptyEventCodes();
    Serial.println("MEM_SAVE");
    config.save();
  }

  // inicio si4707
  if (radio.begin()) {
    Serial.println("SI4707_OK");
    // TODO: io.ledStartup
    io.ledSlow(0);
    io.ledOff(1);
  } else {
    Serial.print("SI4707_ERROR");
    // TODO: io.ledSysError
    io.ledFast(0);
    io.ledFast(1);
    while (1) {
      io.ledRefresh();
      delay(50);
    }
  }

  // ajuste de mute y volumen
  radio.setMuteVolume(config.getMute());
  Serial.print("SET_MUTE,");
  Serial.println(config.getMute());
  config.setVolume(config.getVolume());
  Serial.print("SET_VOLUME,");
  Serial.println(config.getVolume());

  // sintonizacion canal
  if (radio.setWBFrequency(same_channels[config.getChannel() - 1])) {
    Serial.print("TUNE_OK,");
  } else {
    Serial.print("TUNE_ERROR,");
  }
  Serial.println(config.getChannel());
}

void loop() {
  // monitoreo tono 1050 khz
  boolean asq_status = radio.getASQ();
  if (asq_prev_status != asq_status) {
    if (asq_status) {
      Serial.println("ASQ_ON");
    } else {
      Serial.println("ASQ_OFF");
    }
    asq_prev_status = asq_status;
  }

  // recepción mensajes same
  byte same_state = radio.getSAMEState();
  if (same_prev_state != same_state) {
    switch (same_state) {
    case SAME_EOM_DET:
      // fin del mensaje
      Serial.println("SAME_EOM_DET");
      // ¿se recibieron cabeceras?
      if (same_headers_count > 0) {
        same_message();
      } else {
        same_reset();
      }
      break;
    case SAME_PRE_DET:
      // preámbulo detectado
      same_timer = millis();
      Serial.println("SAME_PRE_DET");
      break;
    case SAME_HDR_DET:
      // cabecera detectada
      same_timer = millis();
      Serial.println("SAME_HDR_DET");
      break;
    case SAME_HDR_RDY:
      // cabecera lista
      same_timer = millis();
      same_headers_count++;
      Serial.print("SAME_HDR_RDY,");
      Serial.println(same_headers_count);
      break;
    }
    // ¿se recibieron tres cabeceras?
    if (same_headers_count == 3) {
      same_message();
    }
    same_prev_state = same_state;
    return;
  }

  // timeout mensaje same
  if (same_timer && millis() - same_timer > SAME_TIMEOUT) {
    Serial.println("SAME_TIMEOUT");
    // ¿se recibieron cabeceras?
    if (same_headers_count > 0) {
      same_message();
    } else {
      same_reset();
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

  // ---------------------------------------------------------------------------

  if (cmd.isReady()) {
    char* type = cmd.getType();
  }

  if (io.isButtonTriggered()) {
    Serial.println("BUTTON_TEST");
    alertOn();
  }

  io.ledRefresh();

  delay(50);
}

// ---------------------------------------------------------------------------
// procesamos mensaje same
void same_message() {
  byte size = radio.getSAMESize();
  // ¿mensaje vacío?
  if (size < 1) {
    Serial.println("SAME_EMPTY");
    return;
  }

  // adquisición mensaje
  byte msg[size];
  radio.getSAMEMessage(size, msg);
  same_reset();

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
void same_reset() {
  radio.clearSAMEBuffer();
  same_timer = 0;
  same_headers_count = 0;
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

// Local Variables:
// mode: c++
// End:
