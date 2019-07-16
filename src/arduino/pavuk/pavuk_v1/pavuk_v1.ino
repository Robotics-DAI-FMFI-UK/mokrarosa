#include <Servo.h>

Servo ZLK;
Servo ZL;
Servo PLK;
Servo PL;
Servo ZPK;
Servo ZP;
Servo PPK;
Servo PP;

void setup() {
  ZLK.attach(1);
  ZL.attach(0);
  PLK.attach(2);
  PL.attach(3);
  ZPK.attach(4);
  ZP.attach(5);
  PPK.attach(6);
  PP.attach(7);
}

void posun(){
  PLK.write(90);
  delay(100);
  PL.write(0);
  delay(100);
  PLK.write(180);
  ZPK.write(90);
  delay(100);
  ZP.write(90);
  delay(100);
  ZPK.write(180);
  ZL.write(90);
  PPK.write(90);
  delay(100);
  PP.write(180);
  delay(100);
  PPK.write(0);
  }

void kalibrace(){
  ZLK.write(0);
  ZL.write(150);
  PLK.write(180);
  PL.write(30);
  ZPK.write(180);
  ZP.write(30);
  PPK.write(0);
  PP.write(150);}

void loop() {
  kalibrace();
  delay(1000);
  posun();
  delay(1000);
  
}
