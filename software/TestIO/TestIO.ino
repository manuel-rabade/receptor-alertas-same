#include <avr/wdt.h>

#define LED1 6
#define LED2 7
#define LED3 8
#define LED4 9

#define BTN 10
#define RELAY 3

int x = 0;
int y = 0;
void setup() {
  Serial.begin(9600);
  Serial.println("Setup");

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(BTN, INPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);

  digitalWrite(RELAY, HIGH);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);

  delay(1000);
}

void loop() {
  if (x < 1) {
    Serial.println("Loop");
    Serial.print("BTN: ");
    Serial.println(digitalRead(BTN));
    Serial.print("A6: ");
    Serial.println(analogRead(A6));
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, LOW);
    digitalWrite(A0, HIGH);
    digitalWrite(A1, LOW);
    digitalWrite(A2, HIGH);
    digitalWrite(A3, LOW);
    x++;
  } else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
    digitalWrite(A0, LOW);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
    digitalWrite(A3, HIGH);
    if (y < 1) {
      digitalWrite(RELAY, HIGH);
      y++;
    } else {
      digitalWrite(RELAY, LOW);
      y = 0;
    }
    x = 0;
  }
  delay(500);
}
