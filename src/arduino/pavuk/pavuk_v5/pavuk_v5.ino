#include <Servo.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU6050.h>
SoftwareSerial Bluetooth(2, 4);
MPU6050 mpu;
int cas = 70;
int inp = 0;
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int e = 0;
int f = 0;
int g = 0;
int h = 0;
int Time = 500;
int gyroT = 300;
char buffet[1];

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
  
  ZLK.attach(8);
  ZL.attach(9);
  PLK.attach(10);
  PL.attach(11);
  ZPK.attach(3);
  ZP.attach(5);
  PPK.attach(6);
  PP.attach(7);
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
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
    ZLK.write(90);
    PLK.write(90);
    ZPK.write(90);
    PPK.write(90);
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

void antispam(){
  int index = 0;
  while (Bluetooth.available() > 0){
    buffet[index] = Bluetooth.read();
    index++;
    if (index > 0){
      index = 0;
    }
  }
}

void battle(){
  if (inp == '5'){
    Xattack();
    kalibrace();
    }
  if (inp == '6'){
    if (a == 1){
      b = 1;
      kalibrace();
      }
    if (a == 0){
      cube();
      a = 1;
      }
    if (b == 1){
      a = 0;
      b = 0;
    }
  }
  if (inp == '7'){
    safe();
    kalibrace();
  }
}

void assist(){
  if (inp == '6'){
    if (e == 0){
      gyroT = 0;
      f = 1;
      }
    if (e == 1){
      e = 0;
      }
    if (f == 1){
      gyroT = 300;
      e = 1;
      f = 0;
      }
  }
  if (inp == '7'){
    if (g == 0){
      h = 1;
      }
    if (g == 1){
      g = 0;
      }
    if (h == 1){
      g = 1;
      h = 0;
      }
  }
}

void loop() {

  if (Bluetooth.available()){
      
      antispam();
      inp = buffet[0];
      if (inp == '0'){
        if (c == 1){
          d = 1;
          digitalWrite(12, LOW);
        }
        if (c == 0){
          c = 1;
          digitalWrite(12, HIGH);
        }
        if (d == 1){
          c = 0;
          d = 0;
        }
      }
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
      if (inp == '9'){
        kalibrace();
        a = 0;
      }
      if (c == 1){
        battle();
      }
      if (c == 0){
        assist();
      }
      delay(gyroT);
    }
    Vector normAccel = mpu.readNormalizeAccel();
    int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
    int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;

    if (e == 1){
      if (roll > 34 and roll < 56 or roll > -56 and roll < -34){
        cube();
        }
      if (pitch > 34 and pitch < 56 or pitch > -55 and pitch < -34){
        cube();
        }
      }
    if (g == 1){
      if (roll > 169 or roll < -169){
        delay(Time);
        safe();
        kalibrace();
        delay(Time);
        }
    }
}
