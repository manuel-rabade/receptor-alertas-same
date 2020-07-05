#include "IO.h"

IO::IO() {
  // configuramos puertos
  for (byte i = 0; i < 2; i++) {
    pinMode(_ledsPcbPin[i], OUTPUT);
    pinMode(_ledsAuxPin[i], OUTPUT);
  }
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}

boolean IO::isButtonTriggered() {
  // actualizamos integrador
  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (_buttonIntegrator > 0) {
      _buttonIntegrator--;
    }
  } else if (_buttonIntegrator < BUTTON_MAXIMUM) {
    _buttonIntegrator++;
  }

  // actualizamos estado del botón de acuerdo al integrador
  boolean buttonState = _buttonPrevState;
  if (_buttonIntegrator == 0) {
    buttonState = LOW;
  } else if (_buttonIntegrator >= BUTTON_MAXIMUM) {
    _buttonIntegrator = BUTTON_MAXIMUM;
    buttonState = HIGH;
  }

  // monitoreamos cambio botón
  if (buttonState != _buttonPrevState) {
    if (buttonState == HIGH) {
      _buttonLastUpdate = millis();
    } else {
      _buttonTriggered = false;
    }
    _buttonPrevState = buttonState;
  }

  // monitoreamos estado y timer botón
  if (buttonState == HIGH && millis() - _buttonLastUpdate > BUTTON_TIME && !_buttonTriggered) {
    _buttonTriggered = true;
    return true;
  }

  return false;
}

void IO::ledOn(byte n) {
  _ledsMode[n] = LED_ON;
  _ledUpdate(n, HIGH);
}

void IO::ledOff(byte n) {
  _ledsMode[n] = LED_OFF;
  _ledUpdate(n, LOW);
}

void IO::ledSlow(byte n) {
  _ledsMode[n] = LED_SLOW;
  _ledUpdate(n, !_ledsState[n]);
}

void IO::ledFast(byte n) {
  _ledsMode[n] = LED_FAST;
  _ledUpdate(n, !_ledsState[n]);
}

void IO::ledsBegin() {
  ledOn(0);
  ledOn(1);
}

void IO::ledsError() {
  ledFast(0);
  ledOff(1);
}

void IO::ledsWait() {
  ledSlow(0);
  ledOff(1);
}

void IO::ledsWaitRT() {
  ledSlow(0);
  ledOn(1);
}

void IO::ledsAlert() {
  ledFast(0);
  ledFast(1);
}

void IO::ledsRefresh() {
  for (byte n = 0; n < 2; n++) {
    // tiempo de parpadeo
    unsigned long blink_time = 0;
    if (_ledsMode[n] == LED_SLOW) {
      blink_time = BLINK_SLOW_TIME;
    } else if (_ledsMode[n] == LED_FAST) {
      blink_time = BLINK_FAST_TIME;
    } else {
      continue; // avanzamos en el ciclo si no es lento o rápido
    }
    // actualizamos led
    if (millis() - _ledsLastUpdate[n] > blink_time) {
      _ledUpdate(n, !_ledsState[n]);
    }
  }
}

void IO::_ledUpdate(byte n, boolean s) {
  digitalWrite(_ledsPcbPin[n], s);
  digitalWrite(_ledsAuxPin[n], s);
  _ledsState[n] = s;
  _ledsLastUpdate[n] = millis();
}

void IO::relayOn() {
  digitalWrite(RELAY_PIN, HIGH);
}

void IO::relayOff() {
  digitalWrite(RELAY_PIN, LOW);
}
