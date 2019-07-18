#include <Servo.h>

#include <Wire.h>
#include <MPU6050.h>

#define SERIAL_STATE_IDLE      0
#define SERIAL_STATE_RECEIVING 1
#define SERIAL_BUFFER_LENGTH   20

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

Servo ZLK;
Servo ZL;
Servo PLK;
Servo PL;
Servo ZPK;
Servo ZP;
Servo PPK;
Servo PP;

void setup() {
  init_serial(9600);
  
  ZLK.attach(11);
  ZL.attach(10);
  PLK.attach(8);
  PL.attach(9);
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
  ZLK.write(180);
  ZPK.write(0);
  delay(Time);
  PLK.write(180);
  PPK.write(0);
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

void init_serial(uint32_t baud_rate)
{
  pinMode(2, INPUT);
  pinMode(4, OUTPUT);

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
