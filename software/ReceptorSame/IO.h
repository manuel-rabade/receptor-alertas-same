#ifndef IO_H
#define IO_H

#include <Arduino.h>

// configuracion botón
#define BUTTON_PIN 10 // pin
#define BUTTON_TIME 5000 // tiempo que se debe presionar para dispararlo (milisegundos)
#define BUTTON_DEBOUNCE_TIME 0.3 // tiempo para evitar rebote (segundos)
#define BUTTON_SAMPLE_FREQUENCY	10 // frecuencia de actualización (hertz)
#define BUTTON_MAXIMUM (BUTTON_DEBOUNCE_TIME * BUTTON_SAMPLE_FREQUENCY)

// fsm boton
#define BUTTON_OFF 0
#define BUTTON_ON  1

// pin leds
#define LEDS_PCB 6,7 // 1 y 2 en pcb
#define LEDS_AUX 8,9 // 1 y 2 auxiliares

// tiempos de parpadeo
#define BLINK_SLOW_TIME 1000 // lento  (milisegundos)
#define BLINK_FAST_TIME 100 // rápido (milisegundos)

// modos leds
#define LED_OFF 0
#define LED_ON 1
#define LED_SLOW 2
#define LED_FAST 3

// pin relay
#define RELAY_PIN 3

// pin audio player
#define AUDIO_PLAY_PIN A0
#define AUDIO_PLAY_TIME 100

class IO {

 private:
  // pins de entrada/salida
  byte _ledsPcbPin[2] = {LEDS_PCB};
  byte _ledsAuxPin[2] = {LEDS_AUX};

  // control boton
  boolean _buttonPrevState = LOW;
  boolean _buttonTriggered = false;
  int _buttonIntegrator = 0;
  unsigned long _buttonLastUpdate = 0;

  // control leds
  byte _ledsMode[2] = {LED_OFF, LED_OFF};
  boolean _ledsState[2] = {LOW, LOW};
  unsigned long _ledsLastUpdate[2];

  // actualizacion leds
  void _ledUpdate(byte, boolean);

  // reproductor audio
  boolean _audioPlayOn = false;
  boolean _audioPlayState = LOW;
  unsigned long _audioPlayLastUpdate = 0;

 public:
  IO();
  void refresh();

  // boton
  boolean isButtonTriggered();

  // led
  void ledOn(byte);
  void ledOff(byte);
  void ledSlow(byte);
  void ledFast(byte);

  // leds
  void ledsBegin();
  void ledsError();
  void ledsWait();
  void ledsWaitRT();
  void ledsAlert();

  // relay
  void relayOn();
  void relayOff();

  // audio player
  void audioPlayOn();
  void audioPlayOff();
};

#endif

// Local Variables:
// mode: c++
// End:
