#ifndef IO_H
#define IO_H

#include <Arduino.h>

// configuracion botón
#define BUTTON_PIN 10 // pin botón
#define BUTTON_TIME 5000 // tiempo que se debe presionar el botón para dispararlo (milisegundos)
#define BUTTON_DEBOUNCE_TIME 0.3 // tiempo para evitar rebote (segundos)
#define BUTTON_SAMPLE_FREQUENCY	10 // frecuencia de actualización (hertz)
#define BUTTON_MAXIMUM (BUTTON_DEBOUNCE_TIME * BUTTON_SAMPLE_FREQUENCY) // máximo integrador
#define BUTTON_OFF 0 // fsm boton
#define BUTTON_ON  1

// configuracion leds
#define LEDS_PCB 6,7 // pins leds 1 y 2 en pcb
#define LEDS_AUX 8,9 // pins leds 1 y 2 auxiliares
#define BLINK_SLOW_TIME 1000 // tiempo de parapadeo lento  (milisegundos)
#define BLINK_FAST_TIME 100 // tiempo de parpadeo rápido (milisegundos)
#define LED_OFF 0 // modos leds
#define LED_ON 1
#define LED_SLOW 2
#define LED_FAST 3

// configuracion relay
#define RELAY_PIN 3 // pin relay

class IO {

 private:
  // pins de entrada/salida
  byte _ledsPcbPin[2] = {LEDS_PCB};
  byte _ledsAuxPin[2] = {LEDS_AUX};

  // control boton
  boolean _buttonPrevState = LOW;
  boolean _buttonTriggered = false;
  int _buttonIntegrator = 0;
  unsigned long _buttonLastUpdate;

  // control leds
  byte _ledsMode[2] = {LED_OFF, LED_OFF};
  boolean _ledsState[2] = {LOW, LOW};
  unsigned long _ledsLastUpdate[2];

  // actualizacion leds
  void _ledUpdate(byte, boolean);

 public:
  IO();

  // boton
  boolean isButtonTriggered();

  // leds
  void ledOn(byte);
  void ledOff(byte);
  void ledSlow(byte);
  void ledFast(byte);
  void ledRefresh();

  // relay
  void relayOn();
  void relayOff();
};

#endif
