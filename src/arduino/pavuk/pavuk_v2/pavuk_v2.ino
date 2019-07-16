#include <Servo.h>
int cas = 200;


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

void posunL(){
  ZLK.write(90);
  delay(cas);
  ZL.write(140);
  delay(cas);
  ZLK.write(0);
  delay(cas);
  
  PLK.write(90);
  delay(cas);
  PL.write(0);
  delay(cas);
  PLK.write(180);
  delay(cas);
  
  PPK.write(90);
  delay(cas);
  PP.write(90);
  delay(cas);
  PPK.write(0);
  delay(cas);
  
  ZPK.write(90);
  delay(cas);
  ZP.write(60);
  delay(cas);
  ZPK.write(180);
  delay(cas);
  
  PPK.write(90);
  delay(cas);
  PP.write(180);
  delay(cas);
  PPK.write(0);
  }

void kalibrace(){
  ZLK.write(0);
  ZL.write(170);
  PLK.write(180);
  PL.write(30);
  ZPK.write(180);
  ZP.write(10);
  PPK.write(0);
  PP.write(150);}

void loop() {
  kalibrace();
  delay(1000);
  posunL();
  delay(cas);

}
