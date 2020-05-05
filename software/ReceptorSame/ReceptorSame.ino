#include "Si4707.h"

// parametors same
#define SAME_TIMEOUT 6000 // 6 segundos
#define SAME_TEST_TIMEOUT 11400000 // 3 horas y 10 minutos

// interfaz de usuario
#define BUTTON_DEBOUNCE_DELAY 3000 // 3 segundos
#define ALERT_TIMEOUT 60000 // 1 minuto

// maquina de estados mensajes same
#define SAME_EOM_DET 0
#define SAME_PRE_DET 1
#define SAME_HDR_DET 2
#define SAME_HDR_RDY 3

// globales
Si4707 radio;

// monitoreo radio
boolean asq_prev_status;
byte same_prev_state, same_headers_count;
unsigned long same_timer, same_test_timer;

// interfaz de usuario
boolean button_prev_state;
unsigned long alert_timer, blink_timer, debounce_timer;

// temporal
unsigned long freq = 162550; // 162.550 MHz

void setup() {
  // iniciamos puerto serial
  Serial.begin(9600);
  Serial.println("SETUP");

  // iniciamos si4707
  if (radio.begin()) {
    Serial.println("SI4707_OK");
  } else {
    Serial.print("SI4707_ERROR");
    while(1) {
      // TODO: error en leds
    }
  }

  // sintonizamos canal
  if (radio.setWBFrequency(freq)) {
    Serial.println("TUNE_OK");
  } else {
    Serial.println("TUNE_ERROR");
  }
}

void loop() {
  // monitoreo tono 1050 khz
  boolean asq_status = radio.getASQ();
  if (asq_prev_status != asq_status) {
    if (asq_status) {
      same_reset();
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
  if (same_test_timer && millis() - same_test_timer > SAME_TEST_TIMEOUT) {
    same_test_timer = 0;
    Serial.println("SAME_TEST_TIMEOUT");
    // TODO: mostrar en led
  }

  // timeout alerta
  if (alert_timer && millis() - alert_timer > ALERT_TIMEOUT) {
    alert_timer = 0;
    Serial.println("ALERT_TIMEOUT");
    // TODO: apagar
  }

  // boton de prueba

  // configuracion serial

  // leds

  delay(100);
}

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
    same_test_timer = millis();
    Serial.println("SAME_RWT");
  }
}

// reinicio buffer, timer y contador same
void same_reset() {
  radio.clearSAMEBuffer();
  same_timer = 0;
  same_headers_count = 0;
}

// setup alerta
void alert() {
  Serial.println("ALERT");
}
