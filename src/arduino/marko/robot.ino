/*
   SOFTWARE FOR ROBOT CONTROL
   v4.1

   Marko Tomcik
   13th July 2019
*/

#include <Servo.h>
#include <SoftwareSerial.h>

#define MAX_VOLTAGE 8.4
#define MIN_VOLTAGE 6
#define LED 13

#define BATTERY 1
#define USB 0

SoftwareSerial Bluetooth(10, 11);

Servo ZLK, ZL, PLK, PL;
Servo ZPK, ZP, PPK, PP;

int readInAdvance[2];
int latency = 70;
float batteryLevel, batteryVoltage;
int source;

float findOutSource() {
  analogReference(INTERNAL); 
  float volt = analogRead(A7); 
  analogReference(DEFAULT);
  return volt * 0.006129032;  // volt * 1.1 * 57 / 10 / 1023
}

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);
  pinMode(LED, OUTPUT);
  
  ZLK.attach(6);
  ZL.attach(7);
  PLK.attach(9);
  PL.attach(8);
  ZPK.attach(2);
  ZP.attach(3);
  PPK.attach(5);
  PP.attach(4);

  if(findOutSource() > 5) {
    source = BATTERY;
    Serial.println("Source: Battery");
  }
  else {
    Serial.println("Source: USB");
  }
}

void moveFL() {
  ZLK.write(90);
  delay(latency);
  ZL.write(90);
  delay(latency);
  ZLK.write(0);
  delay(latency);

  PLK.write(90);
  delay(latency);
  PL.write(0);
  delay(latency);
  PLK.write(180);
  delay(latency);
  kalibrationF();
}

void moveFR() {
  ZPK.write(90);
  delay(latency);
  ZP.write(90);
  delay(latency);
  ZPK.write(180);
  delay(latency);

  PPK.write(90);
  delay(latency);
  PP.write(180);
  delay(latency);
  PPK.write(0);
  delay(latency);
  kalibrationF();
}

void moveBL() {
  PLK.write(90);
  delay(latency);
  PL.write(90);
  delay(latency);
  PLK.write(180);
  delay(latency);
  
  ZLK.write(90);
  delay(latency);
  ZL.write(180);
  delay(latency);
  ZLK.write(0);
  delay(latency);
  kalibrationB();
}

void moveBR() {
  PPK.write(90);
  delay(latency);
  PP.write(90);
  delay(latency);
  PPK.write(0);
  delay(latency);
  
  ZPK.write(90);
  delay(latency);
  ZP.write(0);
  delay(latency);
  ZPK.write(180);
  delay(latency);
  kalibrationB();
}

void forward() {
  kalibrationF();
  delay(latency);
  moveFL();
  delay(latency);
  moveFR();
  delay(latency);
}

void backward() {
  kalibrationB();
  delay(latency);
  moveBL();
  delay(latency);
  moveBR();
  delay(latency);
}

void left() {
  kalibrationB();
  delay(latency);
  moveBL();
  kalibrationF();
  delay(latency);
  moveFR();
}

void right() {
  kalibrationB();
  delay(latency);
  moveBR();
  kalibrationF();
  delay(latency);
  moveFL();
}

void kalibration() {
  ZLK.write(0);
  ZL.write(135);
  PLK.write(180);
  PL.write(45);
  ZPK.write(180);
  ZP.write(45);
  PPK.write(0);
  PP.write(135);
}

void kalibrationF() {
  ZL.write(150);
  PL.write(50);
  ZP.write(30);
  PP.write(130);
}

void kalibrationB() {
  ZL.write(130);
  PL.write(30);
  ZP.write(50);
  PP.write(130);
}

float measureBatteryVoltage() {
  analogReference(INTERNAL);
  float volt = analogRead(A2); 
  analogReference(DEFAULT);
  return volt * 0.01181640625;  // volt * 1.1 * 242 / 22 / 1023  (22 KOhm out of 220+22=242 KOhm)
}

void lowBattery() {
  Serial.println("LOW BATTERY! PLEASE RECHARGE!");
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  measureBatteryLevel();
}

void measureBatteryLevel() {
  static unsigned long lastMeasuring = 0;
  unsigned long tm = millis();

  if(tm - lastMeasuring > 500) {
    lastMeasuring = tm;
    batteryVoltage = measureBatteryVoltage();
    batteryLevel = (MAX_VOLTAGE - batteryVoltage) * 100 / (MAX_VOLTAGE - MIN_VOLTAGE);
    Serial.print("Battery level: ");
    Serial.print(batteryLevel);
    Serial.print(" (");
    Serial.print(batteryVoltage);
    Serial.println("V)");
    if (batteryVoltage < MIN_VOLTAGE) {
      lowBattery();
    }
  }
}

void antiSpam() {
  int noAvailableToRead = Serial.available() - 2;
  for (int i = 0; i < noAvailableToRead; i++) {
    Serial.read();
  }
}

void loop() {
  if(source) {
    measureBatteryLevel();
  }
  if (Serial.available()) {
    antiSpam();
    char c = Serial.read();
    switch (c) {
      case 'w':
        forward();
        break;
      case 'a':
        left();
        break;
      case 's':
        backward();
        break;
      case 'd':
        right();
        break;
      default:
        break;
    }
  }
}
