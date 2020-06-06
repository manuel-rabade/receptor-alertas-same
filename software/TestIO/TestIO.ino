#include "IO.h"

IO io = IO();
int x = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");
}

void loop() {
  x++;

  /* if (Serial.available() > 0) { */
  /*   io.input(Serial.read()); */
  /* } */

  if (io.isButtonTriggered()) {
    Serial.println("BUTTON_TRIGGERED");
  }

  if (x == 100) {
    Serial.println("LED_1_ON,LED_2_OFF,RELAY_ON");
    io.ledOn(0);
    io.ledOff(1);
    io.relayOn();
  } else if (x == 200) {
    Serial.println("LED_1_OFF,LED_2_ON,RELAY_OFF");
    io.ledOff(0);
    io.ledOn(1);
    io.relayOff();
  } else if (x == 400) {
    Serial.println("LED_1_SLOW,LED_2_FAST,RELAY_OFF");
    io.ledSlow(0);
    io.ledFast(1);
  } else if (x == 800) {
    Serial.println("LED_1_FAST,LED_2_SLOW,RELAY_OFF");
    io.ledFast(0);
    io.ledSlow(1);
    x = 0;
  }

  io.ledRefresh();
  delay(50);
}
