#include <Servo.h>
#include <SoftwareSerial.h>

#define LPB 0
#define LPD 1
#define LZB 2
#define LZD 3
#define PPB 4
#define PPD 5
#define PZB 6
#define PZD 7

#define WARN_LED 13

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;


SoftwareSerial Bluetooth(6, 7);
;
Servo nohy[8];

char fakeBuffer[1];
int fakeIndex = 0;

int ND = 170; //nohy dole
int NH = 135; //nohz hore
int dlj = 50; //delaj

bool set = false;
bool swhl = false;

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  nohy[LPB].attach(3);
  nohy[LZB].attach(4);
  nohy[PPB].attach(10);
  nohy[PZB].attach(8);

  nohy[LPD].attach(12);
  nohy[LZD].attach(5);
  nohy[PPD].attach(11);
  nohy[PZD].attach(9);
  Serial.println("zaciatok");
  stoj();
}

void stoj() {
  nohy[LPB].write(50);
  nohy[LPD].write(ND);
  nohy[LZB].write(150);
  nohy[LZD].write(180 - ND);
  nohy[PPB].write(130);//130
  nohy[PPD].write(180 - ND);
  nohy[PZB].write(30);
  nohy[PZD].write(ND);
}

void swNoh(int ind,int dole1,int dole2,int bok){ //switch noh
  nohy[ind+1].write(dole1);
  delay(dlj);
  nohy[ind].write(bok);
  delay(dlj);
  nohy[ind+1].write(dole2);
  delay(dlj);
}

void krok(int dl){
  
   swNoh(LZB, 180 - NH, 180 - ND, 90);
   delay(dl);
   swNoh(LPB, NH, ND, 0);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);
   
   swNoh(PZB, NH, ND, 90);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 180);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
   
}

void otocL(int dl){
  
   swNoh(PZB, NH, ND, 50);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 150);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
   
   swNoh(LZB, 180 - NH, 180 - ND, 170);
   delay(dl);
   swNoh(LPB, NH, ND, 70);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);
   
}

void otocP(int dl){

   swNoh(LZB, 180 - NH, 180 - ND, 130);
   delay(dl);
   swNoh(LPB, NH, ND, 30);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);

   swNoh(PZB, NH, ND, 10);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 110);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
     
}

void chodD(int dl){
  
   swNoh(LZB, 180 - NH, 180 - ND, 180);
   delay(dl);
   swNoh(LPB, NH, ND, 90);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);
   
   swNoh(PZB, NH, ND, 0);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 90);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
  
}

void otocLDH(int dl){
  
   swNoh(PZB, NH, ND, 50);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 150);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
   
   swNoh(LZB, 180 - NH, 180 - ND, 170);
   delay(dl);
   swNoh(LPB, NH, ND, 70);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);
   
}

void otocPDH(int dl){

   swNoh(LZB, 180 - NH, 180 - ND, 130);
   delay(dl);
   swNoh(LPB, NH, ND, 30);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);

   swNoh(PZB, NH, ND, 10);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 110);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
     
}

void chodDDH(int dl){
  
   swNoh(LZB, 180 - NH, 180 - ND, 180);
   delay(dl);
   swNoh(LPB, NH, ND, 90);
   delay(dl);
   nohy[LZB].write(150);
   nohy[LPB].write(50);
   delay(dl);
   
   swNoh(PZB, NH, ND, 0);
   delay(dl);
   swNoh(PPB, 180 - NH, 180 - ND, 90);
   delay(dl);
   nohy[PZB].write(30);
   nohy[PPB].write(130);
   delay(dl);
  
}

bool zastav(char x){
  if (Serial.available()){
    if (Serial.read() == x){
      return true;  
    }
  }
  return false;
}

void nohyHore(int pocet){
  if ((NH + pocet <=180)&&(NH + pocet >=0)){
    NH += pocet;
    nohy[PZD].write(NH);
    nohy[LZD].write(180 - NH);
    nohy[PPD].write(180 - NH);
    nohy[LPD].write(NH);
  }
}
void nohyDole(int pocet){
  if ((ND + pocet <= 180)&&(ND + pocet >= 0)){
    ND += pocet;
    nohy[PZD].write(ND);
    nohy[LZD].write(180 - ND);
    nohy[PPD].write(180 - ND);
    nohy[LPD].write(ND);
  }  
}

float zmeraj_baterku(){
  float avg = 0;
  analogReference(INTERNAL); 
  float volt = analogRead(A3); 
  analogReference(DEFAULT);
  for (int i=0; i < 10; i++){
    avg = avg + volt * 0.01181640625;
    delay(10);
  }
  avg = avg / 10;
  if (avg <= 6){
    return 1;
  }
  else if (avg > 6){
    return 0;
  }
}

void AntiSpam(){
  while(Bluetooth.available() > 0) {
    fakeBuffer[fakeIndex] = Bluetooth.read();
    fakeIndex++;
    if (fakeIndex > 0){
      fakeIndex = 0;
    }
  }
}

void DoleHlavou(int nh,int nd){
    NH = nh;
    ND = nd;
    swhl = !swhl;
    nohy[PZD].write(ND);
    nohy[LZD].write(180 - ND);
    nohy[PPD].write(180 - ND);
    nohy[LPD].write(ND);
    delay(500);
}

void loop() {
  Vector normAccel = mpu.readNormalizeAccel();
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  Serial.print(roll);
  if (zmeraj_baterku() == 1){
    while(1){
        digitalWrite(WARN_LED, HIGH);
        delay(100);
        digitalWrite(WARN_LED, LOW);
        delay(50);
    }
  }
  if ((swhl == false) && ((roll > 165) || (roll < -165))){
    DoleHlavou(180 - NH, 180 - ND);
  }
  if ((swhl == true) && ((roll < 20) && (roll > -20))){
    DoleHlavou(180 - NH, 180 - ND);
  }
  if (Bluetooth.available()){
    AntiSpam();
    char c = fakeBuffer[0];
    Serial.print(c);
    
    if (c == 'm'){
      set = !set;
      Serial.print(set);
    }
    
    if (set == false){
      digitalWrite(WARN_LED,LOW);
      if (c == 'w'){
        krok(dlj);  
      }
      if (c == 'd'){
        otocP(dlj);  
      }
      if (c == 'a'){
        otocL(dlj);  
      }
      if (c == 's'){
        chodD(dlj);
      }
    }else
    {
      digitalWrite(WARN_LED,HIGH);
      if (c == 'w'){
        nohyDole(-10);
        Serial.println(ND);
      }
      if (c == 's'){
        nohyDole(+10);
        Serial.println(ND);
      }
      if (c == 'i'){
        nohyHore(-10);
        Serial.println(NH);
      }
      if (c == 'k'){
        nohyHore(+10);
        Serial.println(NH);
      }
      if (c == 'j'){
        dlj = 50;
        NH = 95;
        ND = 135;
      }
      if (c == 'l'){
        dlj = 50;
        NH = 95;
        ND = 135;
      }
    }
  }
  delay(5);
}
