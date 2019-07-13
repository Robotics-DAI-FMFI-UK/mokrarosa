#include <Servo.h>

Servo motor;

void setup() {
  motor.attach(4);
}

void loop() {
  motor.write(90);
  delay(1000);
  motor.write(30);
  delay(1000);
  motor.write(150);
  delay(1000);
}

