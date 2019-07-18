#include <Servo.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define WARN_LED 13

SoftwareSerial Bluetooth(8, 9);
int cas = 70;
int inp = 0;
int a = 0;
int b = 0;
int Time = 500;

#define TRIG 16
#define ECHO 15

#define MP3_OUTPUT_PIN 18   // connect to Rx pin of DFPlayer
#define MP3_BUSY_PIN 19     // connect to BUSY pin of DFPlayer
int m;

Servo ZLK;
Servo ZL;
Servo PLK;
Servo PL;
Servo ZPK;
Servo ZP;
Servo PPK;
Servo PP;

uint8_t ignorovat_baterku;

void setup() {

  kalibrace();
  
  pinMode(WARN_LED, OUTPUT);
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode (ECHO, INPUT);

  m = 25;
  mp3_set_volume(m);

  
  Bluetooth.begin(9600);
  ZLK.attach(10);
  ZL.attach(11);
  PLK.attach(2);
  PL.attach(3);
  ZPK.attach(4);
  ZP.attach(5);
  PPK.attach(6);
  PP.attach(7);

  detect_usb_cable();
}

void detect_usb_cable()
{
  while (Serial.available()) Serial.read();
  long cas = millis();
  while (millis() - cas > 1000)
    if (Serial.available())
    {
      ignorovat_baterku = 1;
      Serial.read();
      return;
    }
  ignorovat_baterku = 0;
}

void skontroluj_baterku()
{
  static uint8_t pocitadlo = 0;
  static long posledne_meranie = 0;

  if (ignorovat_baterku) return;
  
  long tm = millis();
  if (tm - posledne_meranie > 500)
  {
    posledne_meranie = tm;
    analogReference(INTERNAL); 
    float volt = analogRead(A3); 
    analogReference(DEFAULT);
    if (volt * 0.01181640625 < 6.0)  // volt * 1.1 * 242 / 22 / 1023  (22 KOhm out of 220+22=242 KOhm)
    {
      pocitadlo++;
      delay(50);
      if (pocitadlo > 10)
      {
        Serial.println(F("!!!!!!!!!!!!!!!! Nabit baterky !!!!!!!!!!!!!!!!!!!!"));
        long v = volt * 0.01181640625 * 100.0;
        Serial.println(v);
        mp3_set_volume(0);
        while(1)
        {
          for (uint8_t i = 0; i < 3; i++)
          {
            digitalWrite(WARN_LED, HIGH);
            delay(100);
            digitalWrite(WARN_LED, LOW);
            delay(50);
          }
          delay(300);
          for (uint8_t i = 0; i < 3; i++)
          {
            digitalWrite(WARN_LED, HIGH);
            delay(300);
            digitalWrite(WARN_LED, LOW);
            delay(50);
          }
          delay(300);
          for (uint8_t i = 0; i < 3; i++)
          {
            digitalWrite(WARN_LED, HIGH);
            delay(100);
            digitalWrite(WARN_LED, LOW);
            delay(50);
          }
          delay(500);
        }
      }
    }
    else pocitadlo = 0;
  }
}

  // volume 0-30
void mp3_set_volume(uint8_t volume)
{
 mp3_send_packet(0x06, volume);  
}

void mp3_play(uint8_t song_number)
{
 mp3_send_packet(0x03, song_number);  
}

void mp3_send_byte(uint8_t pin, uint8_t val)
{
  pinMode(MP3_OUTPUT_PIN, OUTPUT);
  float start_transmission = micros();
  float one_bit = 1000000 / 9600.0;
  float next_change = start_transmission + one_bit;
  digitalWrite(pin, LOW);
  while (micros() < next_change);
  
  for (int i = 2; i < 10; i++)
  {
    if (val & 1) digitalWrite(pin, HIGH);
    else digitalWrite(pin, LOW);
    next_change = start_transmission + one_bit * i;
    val >>= 1;
    while (micros() < next_change);
  }

  digitalWrite(pin, HIGH);
  next_change = micros() + 2 * one_bit;
  while (micros() < next_change);
  pinMode(MP3_OUTPUT_PIN, INPUT);
}

void mp3_send_packet(uint8_t cmd, uint16_t param)
{
  mp3_send_byte(MP3_OUTPUT_PIN, 0x7E);
  mp3_send_byte(MP3_OUTPUT_PIN, 0xFF);
  mp3_send_byte(MP3_OUTPUT_PIN, 0x06);
  mp3_send_byte(MP3_OUTPUT_PIN, cmd);
  mp3_send_byte(MP3_OUTPUT_PIN, 0x00);
  mp3_send_byte(MP3_OUTPUT_PIN, (uint8_t)(param >> 8));
  mp3_send_byte(MP3_OUTPUT_PIN, (uint8_t)(param & 0xFF));
  uint16_t chksm = 0xFF + 0x06 + cmd + (param >> 8) + (param & 0xFF);
  chksm = -chksm;
  mp3_send_byte(MP3_OUTPUT_PIN, (uint8_t)(chksm >> 8));
  mp3_send_byte(MP3_OUTPUT_PIN, (uint8_t)(chksm & 0xFF));
  mp3_send_byte(MP3_OUTPUT_PIN, 0xEF);
}

void posunDL(){
  ZLK.write(90);
  delay(cas);
  ZL.write(90);
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
  ZP.write(90);
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
  PL.write(90);
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
  PP.write(90);
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

void 
kalibrace(){
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

int zmeraj (){
  digitalWrite(TRIG,HIGH);
  delayMicroseconds (10);
  digitalWrite(TRIG, LOW);
  while(digitalRead(ECHO)== 0){}
  long cas1 = micros ();
   while(digitalRead(ECHO)== 1){}
   long cas2 = micros ();
   return (cas2 - cas1) / 58;
}

long naposledy_sa_zacalo_hrat = 0;
long cas_ked_nebola_prekazka  = 0;


void loop() {
  Serial.print(".");delay(20);
  skontroluj_baterku();
  int vzdialenost = zmeraj ();
  if (vzdialenost <= 30){
     if (millis() - naposledy_sa_zacalo_hrat > 10000)
     {
       mp3_play(1);
       naposledy_sa_zacalo_hrat = millis();
     }
  }
  if (vzdialenost <= 20) {
    if (millis() - cas_ked_nebola_prekazka > 5000)
    {
      for (int l = 0; l < 5; l++) {
        pravo();
      }
    }
  }
  else cas_ked_nebola_prekazka = millis();
  
   
  //kalibrace();
  if (Bluetooth.available()){
    cas_ked_nebola_prekazka = millis();
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
      a = 0;
    }
  }

}
