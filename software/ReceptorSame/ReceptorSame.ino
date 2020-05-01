#include "Si4707.h"

unsigned long tuneFreq = 162550; // 162.550 MHz

Si4707 radio;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup!");

  if (radio.begin()) {
    Serial.println("Successfully connected to Si4707");
  } else {
    Serial.print("Didn't connect to an Si4707");
    while(1);
  }

  if (radio.setWBFrequency(tuneFreq)) {
    Serial.println("Tune Success!");
  } else {
    Serial.println("Tune unsuccessful");
  }

}

void loop() {

  delay(100);
}

/* void radio() { */
/*   Serial.println("Radio!"); */

/*   byte intStatus = radio.getIntStatus(); */
/*   byte sameInt = radio.getSAMEInt(); */
/*   byte asqInt = radio.getASQInt(); */

/*   Serial.print("INT "); */
/*   Serial.print(intStatus, HEX); */
/*   Serial.println(); */

/*   Serial.print("SAME INT "); */
/*   Serial.print(sameInt, HEX); */
/*   Serial.println(); */

/*   Serial.print("ASQ INT "); */
/*   Serial.print(asqInt, HEX); */
/*   Serial.println(); */
/* } */
