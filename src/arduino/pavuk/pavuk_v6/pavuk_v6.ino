#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>

// pin connections:
// servos:
//   left front upper:  D9
//   left front lower:  D8
//   left back upper:   D10
//   left back lower:   D11
//   right front upper: D7
//   right front lower: D6
//   right back upper:  D5
//   right back lower:  D3
// bluetooth:
//    BT TX:  D2
//    BT RX:  D4
// mp3 player:
//    RX:     A2
// ultrasonic HC-SR04
//    TRIG:   A0
//    ECHO:   A1
// gyro (MPU-6050)
//    SDA:    A4
//    SCL:    A5
// battery voltage:
//    Vbat:   A3

#define MP3_OUTPUT_PIN 16   // connect Rx pin of DFPlayer to A2

#define TRIG 14    // pin A0
#define ECHO 15    // pin A1

#define SERIAL_STATE_IDLE      0
#define SERIAL_STATE_RECEIVING 1
#define SERIAL_BUFFER_LENGTH   20

#define LP1 0
#define LZ1 1
#define PP1 2
#define PZ1 3
#define LP2 4
#define LZ2 5
#define PP2 6
#define PZ2 7

#define MAX_SEQ_LENGTH 50

#define WARN_LED 13

Servo legs[8];
uint8_t legv[8];
char pluskey[] = {'q', 'w', '3', '4', 'a', 'x', 'c', 'f'};
char minuskey[] = {'1', '2', 'e', 'r', 'z', 's', 'd', 'v'};
char key_step_plus = '+';
char key_step_minus = '-';
int8_t step_size;
int seq_length;
uint8_t seq[MAX_SEQ_LENGTH][8];
uint8_t delaj[MAX_SEQ_LENGTH];
uint8_t del = 0;

uint8_t initial[] = {45,135,135,45,175,5,5,175};

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

static volatile uint8_t serial_state;
static uint8_t serial_buffer[SERIAL_BUFFER_LENGTH];
static volatile uint8_t serial_buf_wp, serial_buf_rp;

static volatile uint8_t receiving_byte;

static volatile uint32_t time_startbit_noticed;
static volatile uint8_t next_bit_order;
static volatile uint8_t waiting_stop_bit;
static uint16_t one_byte_duration;
static uint16_t one_bit_duration;
static uint16_t one_bit_write_duration;
static uint16_t half_of_one_bit_duration;
static uint8_t ignore_batteries = 0;

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.begin(9600);

  while (Serial.available()) Serial.read();
  Serial.println(F("Hi! Press a key for USB-powered run."));
  delay(2500);
  if (Serial.available())
  {
    ignore_batteries = 1;
    Serial.read();
    Serial.println(F("USB-powered"));
  }
  else Serial.println(F("Bat.powered"));

  init_serial(9600);
  
  legs[LZ2].attach(11);
  legs[LZ1].attach(10);
  legs[LP2].attach(8);
  legs[LP1].attach(9);
  legs[PZ2].attach(3);
  legs[PZ1].attach(5);
  legs[PP2].attach(6);
  legs[PP1].attach(7);
  // AK MATE MPU, tak odkomentujte:
  //mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);

  step_size = 1;

  mp3_play(1);
  delay(10);
  mp3_set_volume(20);
  
  if (load_autostart()) play_sequence(1);
  else
  {
    for (int i = 0; i < 8; i++)
      legv[i] = initial[i];
      
    seq_length = 0;
  
    for (int i = 0; i < 8; i++)
      legs[i].write(legv[i]);
    print_usage();
  }
}

void posunDL(){
  legs[LZ2].write(90);
  delay(cas);
  legs[LZ1].write(95);
  delay(cas);
  legs[LZ2].write(5);
  delay(cas);

  legs[LP2].write(90);
  delay(cas);
  legs[LP1].write(5);
  delay(cas);
  legs[LP2].write(175);
  delay(cas);
  kalibraceD();
}

void posunDR(){
  legs[PZ2].write(90);
  delay(cas);
  legs[PZ1].write(105);
  delay(cas);
  legs[PZ2].write(175);
  delay(cas);

  legs[PP2].write(90);
  delay(cas);
  legs[PP1].write(175);
  delay(cas);
  legs[PP2].write(5);
  delay(cas);
  kalibraceD();
}

void posunZL(){
  legs[LP2].write(90);
  delay(cas);
  legs[LP1].write(70);
  delay(cas);
  legs[LP2].write(175);
  delay(cas);
  
  legs[LZ2].write(90);
  delay(cas);
  legs[LZ1].write(175);
  delay(cas);
  legs[LZ2].write(5);
  delay(cas);
  kalibraceZ();
}

void posunZR(){
  legs[PP2].write(90);
  delay(cas);
  legs[PP1].write(100);
  delay(cas);
  legs[PP2].write(5);
  delay(cas);
  
  legs[PZ2].write(90);
  delay(cas);
  legs[PZ1].write(5);
  delay(cas);
  legs[PZ2].write(175);
  delay(cas);
  kalibraceZ();
}

void Xattack(){
  legs[PP1].write(175);
  legs[LP1].write(5);
  legs[PZ1].write(90);
  legs[LZ1].write(90);
  delay(cas);
  legs[PP2].write(90);
  legs[LP2].write(90);
  delay(cas);
  legs[PZ1].write(5);
  legs[LZ1].write(175);
  delay(cas * 3);
  legs[PP2].write(175);
  legs[LP2].write(5);
  delay(cas * 2);
}

// lahnut
void cube(){
    legs[LZ2].write(90);
    legs[LP2].write(90);
    legs[PZ2].write(90);
    legs[PP2].write(90);
}

// obratka
void safe(){
  legs[LP1].write(5);
  legs[PP1].write(175);
  delay(Time);
  legs[LP2].write(5);
  legs[PP2].write(175);
  delay(Time);
  legs[LZ1].write(50);
  legs[PZ1].write(130);
  delay(Time);
  legs[LZ2].write(175);
  legs[PZ2].write(5);
  delay(Time);
  legs[LP2].write(175);
  legs[PP2].write(5);
  delay(Time);
  legs[LZ1].write(175);
  legs[PZ1].write(5);
  delay(Time);
}

void kalibrace(){
  legs[LZ2].write(5);
  legs[LZ1].write(135);
  legs[LP2].write(175);
  legs[LP1].write(45);
  legs[PZ2].write(175);
  legs[PZ1].write(45);
  legs[PP2].write(5);
  legs[PP1].write(135);
}

void kalibraceD(){
  legs[LZ1].write(150);
  legs[LP1].write(50);
  legs[PZ1].write(30);
  legs[PP1].write(130);
}

void kalibraceZ(){
  legs[LZ1].write(130);
  legs[LP1].write(30);
  legs[PZ1].write(50);
  legs[PP1].write(130);
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
  while (serial_available() > 0){
    buffet[index] = serial_read();
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

  if (serial_available()){
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
    int pitch = 0;
    int roll = 0;
    // AK MATE MPU, tak odkomentujte:
    //Vector normAccel = mpu.readNormalizeAccel();
    //pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
    //roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;

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

  skontroluj_baterku();
  if (Serial.available())
  {
    char c = Serial.read();
    for (int i = 0; i < 8; i++)
      if (c == pluskey[i])
      {
        if (legv[i] <= 175 - step_size) legv[i] += step_size; 
        legs[i].write(legv[i]); 
        dump_state();
      }
      else if (c == minuskey[i])
      {
        if (legv[i] >= 5 + step_size) legv[i] -= step_size; 
        legs[i].write(legv[i]);
        dump_state(); 
      }
      
    if (c == key_step_plus)
    {
        step_size++;
        Serial.print(F("step: "));
        Serial.println(step_size);
    }
    else if (c == key_step_minus)
    {
        if (step_size > 1) step_size--;
        Serial.print(F("step: "));
        Serial.println(step_size);
    }
    else if (c == 13)
      store_new_point();
    else if (c == ' ')
      dump_sequence();
    else if (c == '/')
      play_sequence(0);
    else if (c == '\\')
      play_sequence(1);
    else if (c == 'E')
      store_to_EEPROM();
    else if (c == 'O')
      load_from_EEPROM(1);
    else if (c == 'A')
      toggle_autostart();
    else if (c == 'R')
    {
      Serial.print(F("Discard? [y/n]: "));
      do { c = Serial.read(); } while ((c != 'y') && (c != 'n'));
      Serial.println(c);
      if (c == 'y')
      {
         seq_length = 0;
         Serial.println(F("Sequence discarded."));
      }
    }
    else if (c == 'L')
      load_sequence();
    else if (c == 'U')
      undo_step();
    else if (c == 'H')
      print_usage();
  }  
}

void print_usage()
{
  Serial.println(F("Servo control:"));
  for (int i = 0; i < 8; i++)
  {
    Serial.print(minuskey[i]);
    Serial.print(F(" ... "));
    Serial.println(pluskey[i]);
  }
  Serial.println(F("Step size control:"));
  Serial.print(key_step_minus);
  Serial.print(F(" ... "));
  Serial.println(key_step_plus);
  Serial.println(F("Store next point: ENTER"));
  Serial.println(F("Print the sequence: SPACE"));
  Serial.println(F("Load the sequence: L"));
  Serial.println(F("Play the sequence: /"));
  Serial.println(F("Repetitive play: \\"));
  Serial.println(F("Save to EEPROM: E"));
  Serial.println(F("Load from EEPROM: O"));
  Serial.println(F("Toggle autostart: A")); 
  Serial.println(F("Restart: R"));
  Serial.println(F("Print help: H"));
  Serial.println(F("Undo to last saved position: U"));
  Serial.println(F("(to insert a break, repeat the same position again with delay)"));
}

void store_new_point()
{
  char c = 0;
  if (seq_length == MAX_SEQ_LENGTH) return;
  while (c != 13)
  {
    Serial.print(" ");
    Serial.print((char)13);
    Serial.print(F("(+/-/ENTER) delay = "));
    Serial.print(del);
    do { c = Serial.read(); } while (c == -1);
    if (c == '+') if (del < 30) del++;
    if (c == '-') if (del > 0) del--;
  }
  Serial.println();
  for (int i = 0; i < 8; i++)
    seq[seq_length][i] = legv[i];
  delaj[seq_length] = del;
  dump_row(seq_length);
  seq_length++;
}

void dump_row(int i)
{
    for (int j = 0; j < 8; j++)
    {
      Serial.print(seq[i][j]);
      Serial.print(F(" "));
    }
    Serial.println(delaj[i]);
}

void dump_sequence()
{
  Serial.println(F("---"));
  for (int i = 0; i < seq_length; i++)
  {
    dump_row(i);
  }
  Serial.println(F("---"));
}

void play_sequence(uint8_t repete)
{
  do 
  {
    for (int i = 0; i < seq_length; i++)
    {
      if (Serial.available()) { Serial.read(); repete = 0; break; }
      dump_row(i);
      if ((delaj[i] == 0) || (i == 0))
        for (int j = 0; j < 8; j++)
          legs[j].write(seq[i][j]);
      else for (int k = 0; k < 100; k++)
      {
        for (int j = 0; j < 8; j++)
        {
          legs[j].write(seq[i - 1][j] + ((int)(seq[i][j] - seq[i - 1][j]) * k) / 100);
        }
        delay(delaj[i]);
      } 
    }
    if (Serial.available()) { Serial.read(); break; }
  } while (repete);
  for (int i = 0; i < 8; i++)
    legv[i] = seq[seq_length-1][i];
}

void load_sequence()
{
  Serial.print(F("Paste the sequence (or type SPACE to cancel): "));
  char c;
  uint8_t ok;
  
  do {
    c = Serial.peek();
  } while (c == -1);
  
  if (c == ' ') 
  {
    Serial.read();
    Serial.println(F("cancelled."));
    return;
  }
  seq_length = 0;
  Serial.println();
  Serial.println(F("loading... (terminate with empty line)"));

  do {
    while (!Serial.available());
    c = Serial.peek();
    if ((c == 13) || (c == 10)) break;
    ok = 1;
    for (int i = 0; ok & (i < 8); i++)
      seq[seq_length][i] = read_number(&ok);
    if (!ok) break;   
    delaj[seq_length] = read_number(&ok);
    if (!ok) break;
    Serial.println();
    seq_length++;
  } while (1);
  if (!ok) 
  {
    Serial.println();
    Serial.println(F("loading failed."));
  }
  else 
  {
    Serial.read();
    Serial.println(F("Done."));
  }
}

int read_number(uint8_t *ok)
{
  int x = 0;
  *ok = 0;
  while (1) {
    if (Serial.available())
    {
      int c = Serial.read();
      Serial.write(c);
      if (c == 8) x /= 10;
      if ((c >= '0') && (c <= '9'))
      {
        *ok = 1;
        x = x * 10 + (c - '0');
      } else return x;
    }
  }
}

void dump_state()
{
    for (int j = 0; j < 8; j++)
    {
      Serial.print(legv[j]);
      Serial.print(F(" "));
    }
    Serial.println();
}

void undo_step()
{
  uint8_t changed = 0;
  if (seq_length > 0)
    for (int i = 0; i < 8; i++)
      if (seq[seq_length - 1][i] != legv[i]) changed = 1;
  else
  {
    for (int i = 0; i < 8; i++)
    {
      legv[i] = initial[i];
      legs[i].write(legv[i]);
    } 
    return;
  }
  
  if (!changed)
  {
    if (seq_length == 0)
    {
      Serial.println(F("Nothing to undo."));
      return; 
    }
    Serial.print(F("Nothing changed, erase last saved step? [y/n]: "));
    while (!Serial.available());
    char c = Serial.read();
    Serial.println(c);
    if (c == 'n') return;
    seq_length--; 
    Serial.println(F("erased."));
  }
  else
  {
    Serial.print(F("Undo? [y/n]: "));
    while (!Serial.available());
    char c = Serial.read();
    Serial.println(c);
    if (c == 'n') return;
  }
  for (int i = 0; i < 8; i++)
  {
    legv[i] = seq[seq_length - 1][i];
    legs[i].write(legv[i]);
  }
}

void store_to_EEPROM()
{
  if (seq_length == 0)
  {
    Serial.println("nothing to store");
    return;
  }
  Serial.print(F("Write sequence to EEPROM? [y/n]: "));
  while (!Serial.available());
  char c = Serial.read();
  Serial.println(c);
  if (c != 'y') return;
  
  EEPROM.write(0, '@'); 
  EEPROM.write(1, (uint8_t)seq_length);
  for (int i = 0; i < seq_length; i++)
  {
    for (int j = 0; j < 8; j++)
      EEPROM.write(2 + i*9 + j, seq[i][j]);
    EEPROM.write(2 + i*9 + 8, delaj[i]);
  }

  Serial.println(F("Written."));
}

uint8_t load_autostart()
{
  if (EEPROM.read(999) == '~')
  {
    uint8_t autostart = EEPROM.read(1000);
    if (autostart) load_from_EEPROM(0);
    return autostart; 
  }
  else return 0;  
}

void toggle_autostart()
{
  uint8_t autostart = 0;
  if (EEPROM.read(999) == '~')
    autostart = EEPROM.read(1000);
  if (autostart)
  {
    Serial.println(F("Autostart is OFF"));
    EEPROM.write(1000, 0);
  }
  else
  {
    Serial.println(F("Autostart is ON"));
    EEPROM.write(999, '~');
    EEPROM.write(1000, 1);
  }
}

void load_from_EEPROM(uint8_t confirm)
{
  if (confirm)
  {
    Serial.print(F("Read sequence from EEPROM [y/n]: "));
    while (!Serial.available());
    char c = Serial.read();
    Serial.println(c);
    if (c != 'y') return;
  }
  if (EEPROM.read(0) != '@')
  {
    Serial.println("nothing in EEPROM");
    return;
  }

  seq_length = EEPROM.read(1);
  for (int i = 0; i < seq_length; i++)
  {
    for (int j = 0; j < 8; j++)
      seq[i][j] = EEPROM.read(2 + i*9 + j);
    delaj[i] = EEPROM.read(2 + i*9 + 8);
  }
  Serial.print(seq_length);
  Serial.println(" positions.");
}

void skontroluj_baterku()
{
  static long posledne_meranie = 0;
  static long pocet_merani = 0;

  if (ignore_batteries) return;
  long tm = millis();
  if (tm - posledne_meranie > 500)
  {
    posledne_meranie = tm;
    analogReference(INTERNAL); 
    float volt = analogRead(A3); 
    analogReference(DEFAULT);
    if (volt * 0.01181640625 < 6.2)  // volt * 1.1 * 242 / 22 / 1023  (22 KOhm out of 220+22=242 KOhm)
    {
      pocet_merani++;
      if (pocet_merani < 10) return;
      delay(50);
      Serial.println(F("!!!!!!!!!!!!!!!! Nabit baterky !!!!!!!!!!!!!!!!!!!!"));
      while(1) SOS();
    }
    else pocet_merani = 0;
  }
}

void SOS()
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

void init_serial(uint32_t baud_rate)
{
  pinMode(2, INPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, HIGH);

  serial_state = SERIAL_STATE_IDLE;

  one_byte_duration = 9500000 / baud_rate;
  one_bit_duration = 1000000 / baud_rate;
  one_bit_write_duration = one_bit_duration - 1;
  half_of_one_bit_duration = 500000 / baud_rate;

  PCMSK2 |= 4; //PCINT18;
  PCIFR &= ~4; //PCIF2;
  PCICR |= 4; // PCIE2;
}


ISR(PCINT2_vect)
{
  uint32_t tm = micros();
  if (serial_state == SERIAL_STATE_IDLE)
  {
    time_startbit_noticed = tm;
    serial_state = SERIAL_STATE_RECEIVING;
    receiving_byte = 0xFF;
    next_bit_order = 0;
  }
  else if (tm - time_startbit_noticed > one_byte_duration)
  {
    serial_buffer[serial_buf_wp] = receiving_byte;
    serial_buf_wp++;
    if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
    time_startbit_noticed = tm;
    receiving_byte = 0xFF;
    next_bit_order = 0;
  }
  else if (PIND & 4)
  {
    int8_t new_next_bit_order = (tm - time_startbit_noticed - half_of_one_bit_duration) / one_bit_duration;
    while (next_bit_order < new_next_bit_order)
    {
      receiving_byte &= ~(1 << next_bit_order);
      next_bit_order++;
    }
    if (next_bit_order == 8)
    {
      serial_buffer[serial_buf_wp] = receiving_byte;
      serial_buf_wp++;
      if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
      serial_state = SERIAL_STATE_IDLE;
    }
  } else
    next_bit_order = (tm - time_startbit_noticed - half_of_one_bit_duration) / one_bit_duration;
}


uint8_t serial_available()
{
  cli();
  if (serial_buf_rp != serial_buf_wp)
  {
    sei();
    return 1;
  }
  if (serial_state == SERIAL_STATE_RECEIVING)
  {
    uint32_t tm = micros();
    if (tm - time_startbit_noticed > one_byte_duration)
    {
      serial_state = SERIAL_STATE_IDLE;
      serial_buffer[serial_buf_wp] = receiving_byte;
      serial_buf_wp++;
      if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
      sei();
      return 1;
    }
  }
  sei();
  return 0;
}

int16_t serial_read()
{
  cli();
  if (serial_buf_rp != serial_buf_wp)
  {
    uint8_t ch = serial_buffer[serial_buf_rp];
    serial_buf_rp++;
    if (serial_buf_rp == SERIAL_BUFFER_LENGTH) serial_buf_rp = 0;
    sei();
    return ch;
  }

  if (serial_state == SERIAL_STATE_RECEIVING)
  {
    uint32_t tm = micros();
    if (tm - time_startbit_noticed > one_byte_duration)
    {
      uint8_t ch = receiving_byte;
      serial_state = SERIAL_STATE_IDLE;
      sei();
      return ch;
    }
  }
  sei();
  return -1;
}

void serial_write(uint8_t ch)
{
#ifdef ECHO_BT_TO_USB
  Serial.print((char)ch);
#endif
  PORTD &= ~16;
  delayMicroseconds(one_bit_write_duration);
  for (uint8_t i = 0; i < 8; i++)
  {
    if (ch & 1) PORTD |= 16;
    else PORTD &= ~16;
    ch >>= 1;
    delayMicroseconds(one_bit_write_duration);
  }
  PORTD |= 16;
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
}

uint16_t serial_readln(uint8_t *ln, uint16_t max_length)
{
  uint16_t len;
  int16_t ch;
  do {
    ch = serial_read();
    if (ch == 13) continue;
  } while (ch == -1);

  do {
    if ((ch != 13) && (ch != 10) && (ch != -1))
    {
      *(ln++) = ch;
      max_length--;
      len++;
    }
    ch = serial_read();
  } while ((ch != 13) && max_length);
  *ln = 0;
  return len;
}

void serial_print_num(int32_t number)
{
  if (number < 0)
  {
    serial_write('-');
    number = -number;
  }
  int32_t rad = 1;
  while (number / rad) rad *= 10;
  if (number > 0) rad /= 10;
  while (rad)
  {
    serial_write((char)('0' + (number / rad)));
    number -= (number / rad) * rad;
    rad /= 10;
  }
}

void serial_print_char(char ch)
{
  serial_write(ch);
}

void serial_print(const char *str)
{
  while (*str) serial_write(*(str++));
}

void serial_println(const char *str)
{
  serial_print(str);
  serial_write(13);
  serial_write(10);
}

void serial_print_flash(const char *str)
{
  int ln = strlen_P(str);
  for (int i = 0; i < ln; i++)
    serial_write(pgm_read_byte(str + i));
}

void serial_println_flash(const char *str)
{
  serial_print_flash(str);
  serial_write(13);
  serial_write(10);
}

void serial_println_num(int32_t number)
{
  serial_print_num(number);
  serial_println();
}

void serial_println_char(char ch)
{
  serial_write(ch);
  serial_println();
}

void serial_println()
{
  serial_write(13);
  serial_write(10);
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

int meraj()
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  while (digitalRead(ECHO) == 0) {}
  long cas1 = micros();
  while (digitalRead(ECHO) == 1) {}
  long cas2 = micros();
  return (cas2 - cas1) / 58;
}
