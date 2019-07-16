#include <Servo.h>
#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(8, 9);
int cas = 70;
int inp = 0;
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int Time = 500;

Servo ZLK;
Servo ZL;
Servo PLK;
Servo PL;
Servo ZPK;
Servo ZP;
Servo PPK;
Servo PP;

void setup() {
  Bluetooth.begin(9600);
  ZLK.attach(2);
  ZL.attach(3);
  PLK.attach(10);
  PL.attach(11);
  ZPK.attach(4);
  ZP.attach(5);
  PPK.attach(6);
  PP.attach(7);
}

void posunDL(){
  ZLK.write(90);
  delay(cas);
  ZL.write(95);
  delay(cas);
  ZLK.write(0);
  delay(cas);

  PLK.write(90);
  delay(cas);
  PL.write(0);
  delay(cas);
  PLK.write(180);
  delay(cas);
  kalibraceD();
}

void posunDR(){
  ZPK.write(90);
  delay(cas);
  ZP.write(105);
  delay(cas);
  ZPK.write(180);
  delay(cas);

  PPK.write(90);
  delay(cas);
  PP.write(180);
  delay(cas);
  PPK.write(0);
  delay(cas);
  kalibraceD();
}

void posunZL(){
  PLK.write(90);
  delay(cas);
  PL.write(70);
  delay(cas);
  PLK.write(180);
  delay(cas);
  
  ZLK.write(90);
  delay(cas);
  ZL.write(180);
  delay(cas);
  ZLK.write(0);
  delay(cas);
  kalibraceZ();
}

void posunZR(){
  PPK.write(90);
  delay(cas);
  PP.write(100);
  delay(cas);
  PPK.write(0);
  delay(cas);
  
  ZPK.write(90);
  delay(cas);
  ZP.write(0);
  delay(cas);
  ZPK.write(180);
  delay(cas);
  kalibraceZ();
}

void Xattack(){
  PP.write(180);
  PL.write(0);
  ZP.write(90);
  ZL.write(90);
  delay(cas);
  PPK.write(90);
  PLK.write(90);
  delay(cas);
  ZP.write(0);
  ZL.write(180);
  delay(cas * 3);
  PPK.write(180);
  PLK.write(0);
  delay(cas * 2);
}

void cube(){
  if (a == 1){
    b = 1;
    kalibrace();
  }
  if (a == 0){
    ZLK.write(90);
    PLK.write(90);
    ZPK.write(90);
    PPK.write(90);
    a = 1;
  }
  if (b == 1){
    a = 0;
    b = 0;
  }
}

void safe(){
  PL.write(0);
  PP.write(180);
  delay(Time);
  PLK.write(0);
  PPK.write(180);
  delay(Time);
  ZL.write(50);
  ZP.write(130);
  delay(Time);
  PLK.write(180);
  PPK.write(0);
  delay(Time);
  ZLK.write(180);
  ZPK.write(0);
  delay(Time);
  ZL.write(180);
  ZP.write(0);
  delay(Time);
}

void kalibrace(){
  ZLK.write(0);
  ZL.write(135);
  PLK.write(180);
  PL.write(45);
  ZPK.write(180);
  ZP.write(45);
  PPK.write(0);
  PP.write(135);
  }

void kalibraceD(){
  ZL.write(150);
  PL.write(50);
  ZP.write(30);
  PP.write(130);
}

void kalibraceZ(){
  ZL.write(130);
  PL.write(30);
  ZP.write(50);
  PP.write(130);
}

void dopredu(){
  kalibraceD();
  delay(cas);
  posunDL();
  delay(cas);
  posunDR();
  delay(cas);
  
}

void dozadu(){
  kalibraceZ();
  delay(cas);
  posunZL();
  delay(cas);
  posunZR();
  delay(cas);
}

void pravo(){
  kalibraceZ();
  delay(cas);
  posunZR();
  kalibraceD();
  delay(cas);
  posunDL();
}

void lavo(){
  kalibraceZ();
  delay(cas);
  posunZL();
  kalibraceD();
  delay(cas);
  posunDR();
}


void loop() {
  //kalibrace();
  if (Bluetooth.available()){
      inp = Bluetooth.read();
      if (inp == '1'){
        dopredu();
      }
      if (inp == '2'){
        dozadu();
      }
      if (inp == '3'){
        pravo();
      }
      if (inp == '4'){
        lavo();
      }
      if (inp == '5'){
        Xattack();
        kalibrace();
      }
      if (inp == '6'){
        cube();
      }
      if (inp == '7'){
        safe();
        kalibrace();
      }
      if (inp == '9'){
        kalibrace();
      }
    }
}
