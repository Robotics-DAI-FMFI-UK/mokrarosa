/*
 * SOFTWARE FOR ROBOT CONTROL
 * v4.1
 *
 * Marko Tomcik
 * 13th July 2019
 * 
 * Author of the way of movement: Lukáš Johny Jonathan Jacko
 */


// include
#include <Servo.h>
#include <SoftwareSerial.h>


// define
#define MAX_VOLTAGE 8.4
#define MIN_VOLTAGE 6

#define LED 13

#define BATTERY 1
#define USB 0

// variables declaration
SoftwareSerial Bluetooth(10, 11);

Servo ZLK, ZL, PLK, PL;
Servo ZPK, ZP, PPK, PP;

int latency = 70;
float batteryLevel, batteryVoltage;
int source;
unsigned long lastTime = 0;

// finding out source - usb/battery
float findOutSource() {
  analogReference(INTERNAL); 
  float volt = analogRead(A7); 
  analogReference(DEFAULT);
  return volt * 0.006129032;  // volt * 1.1 * 57 / 10 / 1023
}

void basicPosition() {
  ZL.write(110);
  PL.write(70);
  ZP.write(70);
  PP.write(110);
  ZLK.write(90);
  PLK.write(90);
  ZPK.write(90);
  PPK.write(90);
}

void sleep() {
  ZL.write(130);
  PL.write(50);
  ZP.write(50);
  PP.write(130);
}

void standBy() { 
  ZL.write(150);
  PL.write(30);
  ZP.write(30);
  PP.write(150);
  ZLK.write(135);
  PLK.write(45);
  ZPK.write(45);
  PPK.write(135);

  Serial.println("STAND BY! PRESS 's'!");

  while (true) {
    digitalWrite(LED, HIGH);
    delay(750);
    digitalWrite(LED, LOW);
    delay(750);

    if (Serial.available()) {
      char c = Serial.read();
      if (c == 's') {
        break;
      }
    }
    else if (Bluetooth.available()) {
      char c = Bluetooth.read();
      if (c == 's') {
        break;
      }
    }
  }

  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);

  setup();
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
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
  }
  else {
    Serial.println("Source: USB");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
  }
  basicPosition();
}

void moveFL() {
  ZLK.write(90);
  delay(latency);
  ZL.write(90);
  delay(latency);
  ZLK.write(5);
  delay(latency);

  PLK.write(90);
  delay(latency);
  PL.write(5);
  delay(latency);
  PLK.write(175);
  delay(latency);
  kalibrationF();
}

void moveFR() {
  ZPK.write(90);
  delay(latency);
  ZP.write(90);
  delay(latency);
  ZPK.write(175);
  delay(latency);

  PPK.write(90);
  delay(latency);
  PP.write(175);
  delay(latency);
  PPK.write(5);
  delay(latency);
  kalibrationF();
}

void moveBL() {
  PLK.write(90);
  delay(latency);
  PL.write(90);
  delay(latency);
  PLK.write(175);
  delay(latency);
  
  ZLK.write(90);
  delay(latency);
  ZL.write(175);
  delay(latency);
  ZLK.write(5);
  delay(latency);
  kalibrationB();
}

void moveBR() {
  PPK.write(90);
  delay(latency);
  PP.write(90);
  delay(latency);
  PPK.write(5);
  delay(latency);
  
  ZPK.write(90);
  delay(latency);
  ZP.write(5);
  delay(latency);
  ZPK.write(175);
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
  ZLK.write(5);
  ZL.write(135);
  PLK.write(175);
  PL.write(45);
  ZPK.write(175);
  ZP.write(45);
  PPK.write(5);
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
  int noAvailableToRead = Serial.available() - 1;
  for (int i = 0; i < noAvailableToRead; i++) {
    Serial.read();
  }
}

void loop() {
  unsigned long tm = millis();
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
    lastTime = millis();
  }
  else if (Bluetooth.available()) {
    antiSpam();
    char c = Bluetooth.read();
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
    lastTime = millis();
  }
  else if (tm - lastTime > 120000) {
    standBy();
    lastTime = tm;
  }
  else if (tm - lastTime > 5000) {
    sleep();
    Serial.print("Sleeping time: ");
    Serial.println(tm - lastTime);
  }  
}
