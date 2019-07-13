#include <Servo.h>

Servo motor;
int a;

void setup() {
  motor.attach(4);
  Serial.begin(9600);
  a = 90;
}

void loop() {
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == '+') a++;
    if (c == '-') a--;
  }
  motor.write(a);
}

