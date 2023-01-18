#include "IO.h"

#define CONFIG 0x01
#define DELAY 5000
#define AUDIO_DELAY 60000

IO io = IO(CONFIG);
byte state = 1;
byte prevState = 0;
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");
}

void loop() {
  if (io.isButtonTriggered()) {
    Serial.println("BUTTON_TRIGGERED");
  }

  if (state != prevState) {
    switch (state) {
    case 1:
      Serial.println("LED_1_ON,LED_2_OFF,RELAY_ON");
      io.ledOn(0);
      io.ledOff(1);
      io.relayOn();
      break;
    case 2:
      Serial.println("LED_1_OFF,LED_2_ON,RELAY_OFF");
      io.ledOff(0);
      io.ledOn(1);
      io.relayOff();
      break;
    case 3:
      Serial.println("LED_1_SLOW,LED_2_FAST,RELAY_OFF");
      io.ledSlow(0);
      io.ledFast(1);
      break;
    case 4:
      Serial.println("LED_1_FAST,LED_2_SLOW,RELAY_OFF");
      io.ledFast(0);
      io.ledSlow(1);
      break;
    case 5:
      Serial.println("LEDS_BEGIN");
      io.ledsBegin();
      break;
    case 6:
      Serial.println("LEDS_ERROR");
      io.ledsError();
      break;
    case 7:
      Serial.println("LEDS_WAIT");
      io.ledsWait();
      break;
    case 8:
      Serial.println("LEDS_WAIT_RT");
      io.ledsWaitRT();
      break;
    case 9:
      Serial.println("LEDS_ALERT");
      io.ledsAlert();
      break;
    case 10:
      Serial.println("AUDIO_PLAY_ON");
      io.audioPlayOn();
      break;
    case 11:
      Serial.println("AUDIO_PLAY_OFF");
      io.audioPlayOff();
      break;
    }
    prevState = state;
    lastUpdate = millis();
  }

  unsigned long _delay = DELAY;
  if (state == 10) {
    _delay = AUDIO_DELAY;
  }
  if (millis() - lastUpdate > _delay) {
    if (state == 11) {
      state = 0;
    } else {
      state++;
    }
  }

  io.refresh();
  delay(50);
}

// Local Variables:
// mode: c++
// End:
