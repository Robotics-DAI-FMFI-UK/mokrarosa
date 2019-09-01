#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>

// zakomentujte nasledujuci riadok ak nemate MPU6050 gyroskop
#define HAVE_GYRO

// zakomentujte nasledujuci riadok ak nemate HC-SR04 ultrazvuk
#define HAVE_ULTRASONIC

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
// sirene:
//    +: D13
// touch:
//    1: A6
//    2: A7
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

#define MAX_SEQ_LENGTH 36
#define MAX_PROG_SIZE (MAX_SEQ_LENGTH * 9 + 2)

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
uint8_t ultrazvuk = 1;
uint8_t quiet = 0;
uint8_t je_hore_nohami = 0;
int pitch;
int roll;
uint8_t current_song = 1;

uint8_t initial[] = {45,135,135,45,175,5,5,175};
uint8_t naopak[] = {45,135,135,45,5,175,175,5};

#ifdef HAVE_GYRO
MPU6050 mpu;
#endif

int cas = 70;
int inp = 0;
int8_t lezi = 0;
int auto_cube = 0;
int auto_safe = 0;
int safe_delay = 500;
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
  legs[LZ2].attach(11);
  legs[LZ1].attach(10);
  legs[LP2].attach(8);
  legs[LP1].attach(9);
  legs[PZ2].attach(3);
  legs[PZ1].attach(5);
  legs[PP2].attach(6);
  legs[PP1].attach(7);
  reset_pozicie();

  pinMode(WARN_LED, OUTPUT);
  digitalWrite(WARN_LED, LOW);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.begin(9600);
  init_tone2();

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
  
#ifdef HAVE_GYRO
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
#endif

  step_size = 1;

  sound_greeting();
    
  mp3_play(1);
  delay(10);
  mp3_set_volume(20);
  
  if (load_autostart()) play_sequence(1);
  else
  {
    seq_length = 0;
    print_usage();
  }
  serial_println_flash(PSTR("\r\nmokrarosa"));
}

void sound_greeting()
{
  tone2(440, 50);
  delay(50);
  tone2(880, 100);
  delay(100);
  tone2(1760, 50);
  delay(50);
}

void reset_pozicie()
{
    for (int i = 0; i < 8; i++)
      legv[i] = initial[i];
       
    for (int i = 0; i < 8; i++)
      legs[i].write(legv[i]);
}

void pozicia_90()
{
    for (int i = 0; i < 8; i++)
      legv[i] = 90;
       
    for (int i = 0; i < 8; i++)
      legs[i].write(legv[i]);
}

void hore_nohami()
{
    for (int i = 0; i < 8; i++)
      legv[i] = naopak[i];
      
    for (int i = 0; i < 8; i++)
      legs[i].write(legv[i]);
}

void dole_nohami()
{
    for (int i = 0; i < 8; i++)
      legv[i] = initial[i];
      
    for (int i = 0; i < 8; i++)
      legs[i].write(legv[i]);
}

// pre chodzu dole nohami

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

// pre chodzu hore nohami

void posunDL_HN(){
  legs[LZ2].write(90);
  delay(cas);
  legs[LZ1].write(95);
  delay(cas);
  legs[LZ2].write(175);
  delay(cas);

  legs[LP2].write(90);
  delay(cas);
  legs[LP1].write(175);
  delay(cas);
  legs[LP2].write(5);
  delay(cas);
  kalibraceD();
}

void posunDR_HN(){
  legs[PZ2].write(90);
  delay(cas);
  legs[PZ1].write(105);
  delay(cas);
  legs[PZ2].write(5);
  delay(cas);

  legs[PP2].write(90);
  delay(cas);
  legs[PP1].write(175);
  delay(cas);
  legs[PP2].write(175);
  delay(cas);
  kalibraceD();
}

void posunZL_HN(){
  legs[LP2].write(90);
  delay(cas);
  legs[LP1].write(70);
  delay(cas);
  legs[LP2].write(5);
  delay(cas);
  
  legs[LZ2].write(90);
  delay(cas);
  legs[LZ1].write(175);
  delay(cas);
  legs[LZ2].write(175);
  delay(cas);
  kalibraceZ();
}

void posunZR_HN(){
  legs[PP2].write(90);
  delay(cas);
  legs[PP1].write(100);
  delay(cas);
  legs[PP2].write(175);
  delay(cas);
  
  legs[PZ2].write(90);
  delay(cas);
  legs[PZ1].write(5);
  delay(cas);
  legs[PZ2].write(5);
  delay(cas);
  kalibraceZ();
}

// ---

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
void cube() {
  for (int i = 4; i < 8; i++)
    legv[i] = 90;

  for (int i = 0; i < 8; i++)
    legs[i].write(legv[i]);
}

void safe()
{
  if (je_hore_nohami) safe_HN();
  else safe_DN();
}

// obratka
void safe_HN(){
  legs[LP1].write(5);
  legs[PP1].write(175);
  delay(safe_delay);
  legs[LP2].write(5);
  legs[PP2].write(175);
  delay(safe_delay);
  legs[LZ1].write(50);
  legs[PZ1].write(130);
  delay(safe_delay);
  legs[LZ2].write(175);
  legs[PZ2].write(5);
  delay(safe_delay);
  legs[LP2].write(175);
  legs[PP2].write(5);
  delay(safe_delay);
  legs[LZ1].write(175);
  legs[PZ1].write(5);
  delay(safe_delay);
}

void safe_DN(){
  legs[LP1].write(5);
  legs[PP1].write(175);
  delay(safe_delay);
  legs[LP2].write(175);
  legs[PP2].write(5);
  delay(safe_delay);
  legs[LZ1].write(50);
  legs[PZ1].write(130);
  delay(safe_delay);
  legs[LZ2].write(5);
  legs[PZ2].write(175);
  delay(safe_delay);
  legs[LP2].write(5);
  legs[PP2].write(175);
  delay(safe_delay);
  legs[LZ1].write(175);
  legs[PZ1].write(5);
  delay(safe_delay);
}

void kalibrace(){
  static uint8_t angles[] = {45, 135, 135, 45, 175, 5, 5, 175 };
  
  for (int i = 0; i < 8; i++)
    legv[i] = angles[i];

  if (je_hore_nohami) 
    for(int i = 4; i < 8; i++)
      legv[i] = 180 - legv[i];

  for (int i = 0; i < 8; i++)
    legs[i].write(legv[i]);
}

void kalibraceD(){
  static uint8_t angles[] = {50, 150, 130, 30};
  for (int i = 0; i < 4; i++)
  legv[i] = angles[i];

  for (int i = 0; i < 4; i++)
    legs[i].write(legv[i]);
}

void kalibraceZ(){
  static uint8_t angles[] = {30, 130, 130, 50};
  for (int i = 0; i < 4; i++)
  legv[i] = angles[i];

  for (int i = 0; i < 4; i++)
    legs[i].write(legv[i]);
}

// chodza

void dopredu(){
  if (je_hore_nohami) dopredu_HN();
  else dopredu_DN();
}

void dozadu(){
if (je_hore_nohami) dozadu_HN();
  else dozadu_DN();
}

void pravo(){
if (je_hore_nohami) pravo_HN();
  else pravo_DN();
}

void lavo(){
if (je_hore_nohami) lavo_HN();
  else lavo_DN();
}

// chodza dole nohami

void dopredu_DN(){
  kalibraceD();
  delay(cas);
  posunDL();
  delay(cas);
  posunDR();
  delay(cas);
}

void dozadu_DN(){
  kalibraceZ();
  delay(cas);
  posunZL();
  delay(cas);
  posunZR();
  delay(cas);
}

void pravo_DN(){
  kalibraceZ();
  delay(cas);
  posunZR();
  kalibraceD();
  delay(cas);
  posunDL();
}

void lavo_DN(){
  kalibraceZ();
  delay(cas);
  posunZL();
  kalibraceD();
  delay(cas);
  posunDR();
}

// chodza hore nohami

void dopredu_HN(){
  kalibraceD();
  delay(cas);
  posunDL_HN();
  delay(cas);
  posunDR_HN();
  delay(cas);
}

void dozadu_HN(){
  kalibraceZ();
  delay(cas);
  posunZL_HN();
  delay(cas);
  posunZR_HN();
  delay(cas);
}

void pravo_HN(){
  kalibraceZ();
  delay(cas);
  posunZR_HN();
  kalibraceD();
  delay(cas);
  posunDL_HN();
}

void lavo_HN(){
  kalibraceZ();
  delay(cas);
  posunZL_HN();
  kalibraceD();
  delay(cas);
  posunDR_HN();
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

void print_bt_usage()
{
  serial_println_flash(PSTR("steps:"));
  serial_println_flash(PSTR(" 1: forward"));
  serial_println_flash(PSTR(" 2: backward"));
  serial_println_flash(PSTR(" 3: right"));
  serial_println_flash(PSTR(" 4: left"));
  serial_println_flash(PSTR("switches:"));
  delay(150);
  serial_println_flash(PSTR(" 9: init position"));
  serial_println_flash(PSTR(" q: mp3 volume on/off"));
  serial_println_flash(PSTR(" u: ultrasonic on/off"));
  serial_println_flash(PSTR(" h: print this help"));
  delay(150); 
  serial_println_flash(PSTR("run eeprom prog: t b g"));
  serial_println_flash(PSTR(" 5: lift oponent"));
  serial_println_flash(PSTR(" 6: lay down (cube)"));
  serial_println_flash(PSTR(" 7: roll over (safe)"));
  delay(150);
  serial_println_flash(PSTR(" 0: switch auto cube"));
  serial_println_flash(PSTR(" 8: switch auto safe"));  
  serial_println_flash(PSTR("mp3 music: m j n"));
  serial_println_flash(PSTR("simple songs: . , ; ' [ ]"));
}

void control_over_bt()
{
  pip();
  antispam();
  inp = buffet[0];
  if (inp == '1'){
    dopredu();
  }
  else if (inp == '2'){
    dozadu();
  }
  else if (inp == '3'){
    pravo();
  }
  else if (inp == '4'){
    lavo();
  }
  else if (inp == '5'){
    Xattack();
    kalibrace();
  }
  else if (inp == '6'){
    if (lezi == 1){
      kalibrace();
      serial_println_flash(PSTR("vztyk"));
      lezi = 0;
    }
    else {
      cube();
      serial_println_flash(PSTR("lah"));
      lezi = 1;
    }
  }
  else if (inp == '7'){
    safe();
    kalibrace();
  }  
  else if (inp == '0') 
  {
    auto_cube ^= 1;
    serial_print_flash(PSTR("auto cube: "));
    serial_println_num(auto_cube);
  }
  else if (inp == '8') 
  {
    auto_safe ^= 1;
    serial_print_flash(PSTR("auto safe: "));
    serial_println_num(auto_safe);
  }
  else if (inp == 't'){
    load_from_EEPROM(1);
    play_sequence(1);
  }
  else if (inp == 'g'){
    load_from_EEPROM(2);
    play_sequence(1);
  }
  else if (inp == 'b'){
    load_from_EEPROM(3);
    play_sequence(1);
  }
  else if (inp == '9'){
    kalibrace();
    lezi = 0;
  }      
  else if (inp == 'q')
  {
    if (quiet)
       mp3_set_volume(20);
    else
    {
       mp3_set_volume(0);
       zastav_melodiu();
    }
       
    quiet = 1 - quiet;
    serial_print_flash(PSTR("music:"));
    serial_println_num(!quiet);
  }
  else if (inp == 'u')
  {
    ultrazvuk = 1 - ultrazvuk;
    serial_print_flash(PSTR("ultra:"));
    serial_println_num(ultrazvuk);
  }
  else if ((inp == 'h') || (inp == 'H'))
  {
    print_bt_usage();
  }
  else try_melodies(inp);
  delay(300);  
}

void refresh_gyro()
{
#ifdef HAVE_GYRO
  Vector normAccel = mpu.readNormalizeAccel();
  pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;  
#endif
}

void react_to_gyro()
{
  refresh_gyro();
  /*
  Serial.print(roll);
  Serial.print(" ");
  Serial.println(pitch);
  delay(100);
  */

  if (!je_hore_nohami && (abs(roll) > 172)) 
  {
    hore_nohami();
    je_hore_nohami = 1;
    serial_println_flash(PSTR("HN"));
  }
  else if (je_hore_nohami && (abs(roll) < 8))
  {
    dole_nohami();
    je_hore_nohami = 0;
    serial_println_flash(PSTR("DN"));
  }
    
  if (auto_cube == 1){
    for (int i = 0; i < 2; i++)
    {
      if (((roll > 34) && (roll < 56)) || ((roll > -56) && (roll < -34))){
        cube();
        serial_print_flash(PSTR("."));
        delay(safe_delay);
      }
    }
  }
  if (auto_safe == 1){
    if ((roll > 169) || (roll < -169)){
      delay(safe_delay);
      safe();
      kalibrace();
      serial_print_flash(PSTR("#"));
      delay(safe_delay);
    }
  }
}

void control_over_serial()
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
    load_from_EEPROM(0);
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
  else if (c == '*')
    reset_pozicie();
  else if (c == '9')
    pozicia_90();
  else if (c == 'K')
  {
    ultrazvuk = 1 - ultrazvuk;
    if (ultrazvuk) Serial.println(F("Ultrazvuk ON"));
    else Serial.println(F("Ultrazvuk OFF"));
  }
  else if (c == 27) zastav_melodiu();
  else if ((c == 'H') || (c == 'h')) print_usage();
  else if (c == 'B') Serial.println(meraj_baterku());
  else try_melodies(c); 
}

void try_melodies(char c)
{
  if (c == ',') zahraj_melodiu(1);
  else if (c == '.') zahraj_melodiu(2);
  else if (c == ';') zahraj_melodiu(3);
  else if (c == '[') zahraj_melodiu(4);
  else if (c == ']') zahraj_melodiu(5);
  else if (c == '\'') zahraj_melodiu(6);
  if (c == 'm')
  {
    mp3_play(current_song);
    delay(20);
    mp3_set_volume(20);
  }
  else if (c == 'M') mp3_set_volume(0);
  else if ((c == 'j') || (c == 'n'))
  {
    if (c == 'j') if (current_song < 255) current_song++;
    if (c == 'n') if (current_song > 1) current_song--;
    Serial.print(F("song "));
    Serial.println(current_song);
    serial_print_flash(PSTR("song "));
    serial_println_num(current_song);
  }
}

void loop() {

#ifdef HAVE_ULTRASONIC
  if ((meraj() < 15) && ultrazvuk)
  {
    // sem si dajte kod, ktory sa vykonava, ak je pred robotom prekazka:
    dozadu();
  }
#endif

  if (serial_available()) control_over_bt();
  if (Serial.available()) control_over_serial();
  
  pitch = 0;
  roll = 0;
#ifdef HAVE_GYRO
  react_to_gyro();
#endif

  skontroluj_baterku();
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
  Serial.println(F("Erase sequence: R"));
  Serial.println(F("Initial position: *"));
  Serial.println(F("Battery level (*100 V): B"));
  Serial.println(F("Position 90: 9"));
  Serial.println(F("Ultrazvuk ON/OFF: 'K'"));
  Serial.println(F("Play melody: . , ; ' [ ]"));
  Serial.println(F("Stop melody: ESC"));
  Serial.println(F("Play/change music: m j n"));
  Serial.println(F("Stop music: M"));
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
  if (repete) zahraj_melodiu(6);
  do 
  {
    for (int i = 0; i < seq_length; i++)
    {
      if (Serial.available() || serial_available())
      { Serial.read(); repete = 0; break; }
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
	  skontroluj_baterku();
    }
    if (Serial.available()) { Serial.read(); break; }
    if (serial_available()) { serial_read(); break; }
  } while (repete);
  for (int i = 0; i < 8; i++)
    legv[i] = seq[seq_length-1][i];
  zastav_melodiu();
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

int8_t ask_for_slot_num()
{
  Serial.print(F("Seq.slot 1-3: "));
  while (!Serial.available());
  char c = Serial.read();
  Serial.println(c);
  if ((c < '1') || (c > '3')) 
  {
    Serial.println("range is 1-3");
    return -1;
  }
  return c - '1';
}

void store_to_EEPROM()
{
  if (seq_length == 0)
  {
    Serial.println("nothing to store");
    return;
  }
  int8_t prognum = ask_for_slot_num();
  if (prognum < 0) return;
  
  Serial.print(F("Write sequence to EEPROM? [y/n]: "));
  while (!Serial.available());
  char c = Serial.read();
  Serial.println(c);
  if (c != 'y') return;
  
  EEPROM.write(0 + prognum * MAX_PROG_SIZE, '@'); 
  EEPROM.write(1 + prognum * MAX_PROG_SIZE, (uint8_t)seq_length);
  for (int i = 0; i < seq_length; i++)
  {
    for (int j = 0; j < 8; j++)
      EEPROM.write(prognum * MAX_PROG_SIZE + 2 + i*9 + j, seq[i][j]);
    EEPROM.write(prognum * MAX_PROG_SIZE + 2 + i*9 + 8, delaj[i]);
  }

  Serial.println(F("Written."));
}

uint8_t load_autostart()
{
  if (EEPROM.read(1022) == '~')
  {
    uint8_t autostart = EEPROM.read(1023);
    if (autostart) load_from_EEPROM(autostart);
    return autostart;
  }
  else return 0;  
}

void toggle_autostart()
{
  int8_t autostart = ask_for_slot_num();
  if (autostart < 0)
  {
      EEPROM.write(1023, 0);
      Serial.println(F("Autostart is OFF"));
  }
  else
  {
    Serial.print(F("Autostart is ON: "));
    Serial.println(autostart + 1);
    EEPROM.write(1022, '~');
    EEPROM.write(1023, autostart + 1);
  }
}

void load_from_EEPROM(int8_t prognum)
{
  if (prognum == 0)
  {
    prognum = ask_for_slot_num();
    if (prognum < 0) return;
    Serial.print(F("Read sequence from EEPROM [y/n]: "));
    while (!Serial.available());
    char c = Serial.read();
    Serial.println(c);
    if (c != 'y') return;
  }
  else prognum--;
  if (EEPROM.read(prognum * MAX_PROG_SIZE + 0) != '@')
  {
    Serial.println("nothing in EEPROM");
    return;
  }
  
  seq_length = EEPROM.read(prognum * MAX_PROG_SIZE + 1);
  for (int i = 0; i < seq_length; i++)
  {
    for (int j = 0; j < 8; j++)
      seq[i][j] = EEPROM.read(prognum * MAX_PROG_SIZE + 2 + i*9 + j);
    delaj[i] = EEPROM.read(prognum * MAX_PROG_SIZE + 2 + i*9 + 8);
  }
  Serial.print(seq_length);
  Serial.println(" positions.");
}

int meraj_baterku()
{
  analogReference(INTERNAL); 
  float volt = analogRead(A3); 
  analogReference(DEFAULT);
  // volt * 1.1 * 242 / 22 / 1023  (22 KOhm out of 220+22=242 KOhm)
  return (int)(0.5 + 100.0 * volt * 0.01181640625);   
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
    if (meraj_baterku() < 620)  
    {
      pocet_merani++;
      if (pocet_merani < 5) return;
      delay(50);
      Serial.println(F("!!!!!!!!!!!!!!!! Nabit baterky !!!!!!!!!!!!!!!!!!!!")); 
      mp3_set_volume(0);
      for (int i = 0; i < 8; i++)
        legs[i].detach();
      for (int i = 2; i < 20; i++)
        pinMode(i, INPUT);
      pinMode(WARN_LED, OUTPUT);
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
    tone2(1680, 100);
    delay(100);
    digitalWrite(WARN_LED, LOW);
    delay(50);
  }
  delay(300);
  for (uint8_t i = 0; i < 3; i++)
  {
    tone2(1680, 300);
    digitalWrite(WARN_LED, HIGH);
    delay(300);
    digitalWrite(WARN_LED, LOW);
    delay(50);
  }
  delay(300);
  for (uint8_t i = 0; i < 3; i++)
  {
    digitalWrite(WARN_LED, HIGH);
    tone2(1680, 100);
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
  unsigned long pocitadlo = micros() + 10000;
  while ((digitalRead(ECHO) == 0) & (micros() < pocitadlo)) { }
  if (digitalRead(ECHO) == 0) return 255;
  unsigned long cas1 = micros();
  pocitadlo = cas1 + 10000; 
  while ((digitalRead(ECHO) == 1) && (micros() < pocitadlo)) { }
  if (digitalRead(ECHO) == 1) return 255;
  unsigned long cas2 = micros();
  return (cas2 - cas1) / 58;
}

//-------------------------------- nasleduje prehravanie melodie a hranie cez timer2 v pozadi
#define SIRENE_PORT  PORTB
#define SIRENE_DDR   DDRB
#define SIRENE_PIN   5

#define FIS3 2960
#define G3 3136

float octave_4[] = { 2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.00, 3729.31, 3951.07 };

//popcorn
uint16_t dlzka_melodia[] = {0, 386, 26, 281, 217, 36, 468};
const uint8_t melodia1[] PROGMEM = { 252, 50, 149,  49,
                                     28, 31, 35, 40, 49, 99, 38, 49, 99, 40, 49, 99, 35, 49, 99, 31, 49, 99, 35, 49, 99, 28, 49, 99, 49,
                                     28, 31, 35, 40, 49, 99, 38, 49, 99, 40, 49, 99, 35, 49, 99, 31, 49, 99, 35, 49, 99, 28, 49, 99, 149,
                                     40, 49, 99, 42, 49, 99, 43, 49, 99, 42, 49, 99, 43, 49, 99, 40, 49, 99, 42, 49, 99, 40, 49, 99, 42, 49, 99, 38, 49, 99, 40, 49, 99, 38, 49, 99, 40, 49, 99, 36, 49, 99, 40, 49, 99,
                                     28, 31, 35, 40, 49, 99, 38, 49, 99, 40, 49, 99, 35, 49, 99, 31, 49, 99, 35, 49, 99, 28, 49, 99, 49,
                                     28, 31, 35, 40, 49, 99, 38, 49, 99, 40, 49, 99, 35, 49, 99, 31, 49, 99, 35, 49, 99, 28, 49, 99, 149,
                                     40, 49, 99, 42, 49, 99, 43, 49, 99, 42, 49, 99, 43, 49, 99, 40, 49, 99, 42, 49, 99, 40, 49, 99, 42, 49, 99, 38, 49, 99, 40, 49, 99, 38, 49, 99, 40, 49, 99, 42, 49, 99, 43, 49, 99,
                                     49, 35, 38, 43, 47, 49, 99, 45, 49, 99, 47, 49, 99, 43, 49, 99, 38, 49, 99, 43, 49, 99, 35, 49, 99,
                                     49, 35, 38, 43, 47, 49, 99, 45, 49, 99, 47, 49, 99, 43, 49, 99, 38, 49, 99, 43, 49, 99, 35, 49, 99, 149 ,
                                     47, 49, 99, 254, 49, 99, 255, 49, 99, 254, 49, 99, 255, 49, 99, 47, 49, 99, 254, 49, 99, 47, 49, 99, 254, 49, 99, 45, 49, 99, 47, 49, 99, 45, 49, 99, 47, 49, 99, 43, 49, 99, 47, 49, 99,
                                     49, 35, 38, 43, 47, 49, 99, 45, 49, 99, 47, 49, 99, 43, 49, 99, 38, 49, 99, 43, 49, 99, 35, 49, 99,
                                     49, 35, 38, 43, 47, 49, 99, 45, 49, 99, 47, 49, 99, 43, 49, 99, 38, 49, 99, 43, 49, 99, 35, 49, 99, 149 ,
                                     47, 49, 99, 254, 49, 99, 255, 49, 99, 254, 49, 99, 255, 49, 99, 47, 49, 99, 254, 49, 99, 47, 49, 99, 254, 49, 99, 45, 49, 99, 47, 49, 99, 45, 49, 99, 47, 49, 99, 254, 49, 99, 255, 49, 99
                                   };

//kohutik jarabi
const uint8_t melodia2[] PROGMEM = { 252, 150, 119, 121, 173, 174, 124, 124, 124, 123, 171, 173, 123, 123, 123, 121, 169, 171, 121, 121, 121, 123, 171, 169, 119, 119 };

//kankan
const uint8_t melodia3[] PROGMEM = { 252, 100,
                                     251, 1, 184, 1, 32, 126, 149, 251, 1, 184, 1, 32, 126, 149, 251, 1, 184, 1, 32, 126, 251, 1, 184, 1, 32, 126, 251, 1, 184, 1, 32, 126, 251, 1, 184, 1, 32, 126,
                                     64, 71, 71, 73, 71, 69, 69, 73, 74, 78, 81, 78, 78, 76, 251, 1, 184, 1, 32, 126, 78, 68, 68, 78, 76, 69, 69, 73, 73, 71, 73, 71, 85, 83, 85, 83,
                                     64, 71, 71, 73, 71, 69, 69, 73, 74, 78, 81, 78, 78, 76, 251, 1, 184, 1, 32, 126, 78, 68, 68, 78, 76, 69, 69, 73, 73, 71, 73, 71, 71, 69, 119,
                                     135, 131, 128, 126, 75, 76, 78, 80, 81, 76, 80, 76, 81, 76, 80, 76, 81, 76, 80, 76, 81, 76, 80, 76,
                                     251, 2, 11, 3, 16, 19, 251, 1, 4, 3, 16, 19,
                                     251, 1, 4, 3, 16, 19, 251, 1, 4, 3, 16, 19,
                                     251, 1, 4, 3, 16, 19, 251, 1, 4, 3, 16, 19,
                                     251, 1, 4, 3, 16, 19, 251, 1, 4, 3, 16, 19,
                                     174, 88, 91, 90, 88, 143, 143, 93, 95, 90, 91, 138, 138, 88, 91, 90, 88, 86, 86, 85, 83, 81, 79, 78, 76,
                                     174, 88, 91, 90, 88, 143, 143, 93, 95, 90, 91, 138, 138, 88, 91, 90, 88, 86, 93, 89, 90, 136,
                                     64, 71, 71, 73, 71, 69, 69, 73, 74, 78, 81, 78, 78, 76, 251, 1, 184, 1, 32, 126,
                                     78, 76, 126, 78, 76, 126, 78, 76, 126, 78, 76, 126, 78, 76, 126, 78, 76, 126,
                                     78, 76, 78, 76, 78, 76, 78, 76,
                                     131, 119, 119, 119, 169
                                   };

//labutie jazero
const uint8_t melodia4[] PROGMEM = {
  252, 220, 66, 69, 73, 69, 66, 69, 73, 69, 66, 69, 73, 69, 66, 69, 73, 69,
  185, 78, 80, 81, 83, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 78, 81, 78, 73, 81, 178,
  99, 83, 81, 80,
  185, 78, 80, 81, 83, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 78, 81, 78, 73, 81, 178,
  149, 128, 130, 131, 133, 85, 86, 251, 6, 32, 3, 8, 86, 135, 86, 88, 251, 6, 224, 3, 8, 88, 136, 88, 90, 251, 7, 184, 3, 8, 90, 85, 81, 80, 78,
  130, 131, 133, 85, 86, 251, 6, 32, 3, 8, 86, 135, 86, 88, 251, 6, 224, 3, 8, 88, 136, 88, 90, 251, 7, 73, 3, 8, 86, 133, 86, 91, 251, 7, 184, 3, 8, 87, 251, 7, 184, 3, 8, 85,
  185, 78, 80, 81, 83, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 78, 81, 78, 73, 81, 178,
  99, 83, 81, 80,
  185, 78, 80, 81, 83, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 81, 251, 5, 39, 3, 8, 78, 81, 78, 73, 81, 178
};

//let it be https://www.musicnotes.com/sheetmusic/mtd.asp?ppn=MN0101556
const uint8_t melodia5[] PROGMEM = { 252, 200, 26, 26, 76, 26, 253, 78, 73, 76, 76, 31, 253, 83, 35, 85, 253, 85, 
                                     83, 83, 81, 131, 35, 253, 85, 86, 35, 35, 85, 83, 99, 35, 33, 83, 81, 181 
};

//Turkish march by Mozart
const uint8_t melodia6[] PROGMEM = { 80, 78, 77, 78, 131, 149, 83, 81, 80, 81, 135, 149, 86, 85, 84, 85, 92, 90, 
89, 90, 92, 90, 89, 90, 193, 140, 143, 142, 140, 138, 140, 142, 140, 138, 
140, 142, 140, 138, 137, 185, 80, 78, 77, 78, 131, 149, 83, 81, 80, 81, 
135, 149, 86, 85, 84, 85, 92, 90, 89, 90, 92, 90, 89, 90, 193, 140, 143, 
142, 140, 138, 140, 142, 140, 138, 140, 142, 140, 138, 137, 185, 135, 136, 
138, 138, 90, 88, 86, 85, 183, 135, 136, 138, 138, 90, 88, 86, 85, 183, 
131, 133, 135, 135, 86, 85, 83, 81, 180, 131, 133, 135, 135, 86, 85, 83, 
81, 180, 80, 78, 77, 78, 131, 149, 83, 81, 80, 81, 135, 149, 86, 85, 84, 
85, 92, 90, 89, 90, 92, 90, 89, 90, 193, 140, 142, 143, 142, 140, 139, 140, 
135, 136, 133, 181, 130, 78, 80, 178, 128, 130, 182, 128, 130, 132, 130, 
128, 127, 125, 127, 128, 130, 127, 123, 128, 130, 182, 128, 130, 132, 130, 
128, 127, 125, 130, 127, 123, 178, 94, 95, 94, 92, 140, 139, 87, 90, 89, 
87, 136, 139, 82, 84, 85, 82, 137, 139, 90, 89, 90, 92, 144, 93, 94, 95, 
94, 92, 140, 139, 87, 90, 89, 87, 135, 139, 82, 84, 85, 82, 134, 137, 81, 
82, 84, 81, 182, 94, 95, 94, 92, 140, 139, 87, 90, 89, 87, 136, 139, 82, 
84, 85, 82, 137, 139, 90, 89, 90, 92, 144, 93, 94, 95, 94, 92, 140, 139, 
87, 90, 89, 87, 135, 139, 82, 84, 85, 82, 134, 137, 81, 82, 84, 81, 182, 
128, 130, 182, 128, 130, 132, 130, 128, 127, 125, 127, 128, 130, 127, 123, 
128, 130, 182, 128, 130, 132, 130, 128, 127, 125, 130, 127, 123, 178, 80, 
78, 77, 78, 131, 149, 83, 81, 80, 81, 135, 149, 86, 85, 84, 85, 92, 90, 89, 
90, 92, 90, 89, 90, 193, 140, 143, 142, 140, 138, 140, 142, 140, 138, 140, 
142, 140, 138, 137, 185, 80, 78, 77, 78, 131, 149, 83, 81, 80, 81, 135, 
149, 86, 85, 84, 85, 92, 90, 89, 90, 92, 90, 89, 90, 193, 140, 143, 142, 
140, 138, 140, 142, 140, 138, 140, 142, 140, 138, 137, 185, 135, 136, 138, 
138, 90, 88, 86, 85, 183, 135, 136, 138, 138, 90, 88, 86, 85, 183, 131, 
133, 135, 135, 86, 85, 83, 81, 180, 131, 133, 135, 135, 86, 85, 83, 81, 
180, 80, 78, 77, 78, 131, 149, 83, 81, 80, 81, 135, 149, 86, 85, 84, 85, 
92, 90, 89, 90, 92, 90, 89, 90, 193, 140, 142, 143, 142, 140, 139, 140, 
135, 136, 133, 181, 130, 78, 80, 178, 131, 135, 190, 199 };

volatile int16_t music_speed = 800 / 16;
volatile const uint8_t *current_note ;
volatile uint16_t notes_remaining;
uint8_t dotted_note = 0;

void zahraj_melodiu(uint8_t cislo)
{
  if (cislo == 0) {
    zastav_melodiu();
    return;
  }

  if (cislo == 1) current_note = melodia1;
  else if (cislo == 2) current_note = melodia2;
  else if (cislo == 3) current_note = melodia3;
  else if (cislo == 4) current_note = melodia4;
  else if (cislo == 5) current_note = melodia5;
  else if (cislo == 6) current_note = melodia6;
  notes_remaining = dlzka_melodia[cislo];

  next_note();
}

void next_note()
{
  uint16_t freq = 0, dur = 0;
  if (!notes_remaining) return;
  otto_translate_tone_flash(&freq, &dur);
  tone2(freq, dur);
}

void otto_translate_tone_flash(uint16_t *freq, uint16_t *del)
{
  do {
    uint8_t n = pgm_read_byte(current_note);
    if (n > 249)
    {
      if (n == 251)
      {
        current_note++;
        uint8_t f1 = pgm_read_byte(current_note);
        current_note++;
        uint8_t f2 = pgm_read_byte(current_note);
        current_note++;
        uint8_t d1 = pgm_read_byte(current_note);
        current_note++;
        uint8_t d2 = pgm_read_byte(current_note);
        *freq = (f1 << 8) + f2;
        *del = (music_speed * 16 * (long)d1) / d2;
        notes_remaining -= 4;
      }
      else if (n == 252)
      {
        current_note++;
        music_speed = pgm_read_byte(current_note);
        current_note++;
        notes_remaining -= 2;
        continue;
      }
      else if (n == 253) 
      {
        dotted_note = 1;
        current_note++;
        notes_remaining--;        
        continue;
      }
      else if (n == 254)
      {
        *freq = FIS3;
        *del = music_speed;
      }
      else if (n == 255)
      {
        *freq = G3;
        *del = music_speed;
      }
    }
    else
    {
      uint8_t len = n / 50;
      *del = music_speed;
      while (len--) *del *= 2;
      n = n % 50;
      if (n != 49)
      {
        uint8_t octave = (n + 5) / 12;
        n = (n + 5) % 12;
        float ffreq = octave_4[n];
        octave = 4 - octave;
        while (octave > 0)
        {
          ffreq /= 2.0;
          octave--;
        }
        *freq = (uint16_t)ffreq;
      }
      else *freq = 0;
    }
    notes_remaining--;
    current_note++;
    break;
  } while (1);
  if (dotted_note) 
  {
    *del += (*del) / 2;
    dotted_note = 0;
  }
}

static volatile uint8_t tone2_state;
static volatile uint8_t tone2_pause;
static volatile uint32_t tone2_len;

void init_tone2()
{
  notes_remaining = 0;
  tone2_pause = 0;
  dotted_note = 0;
  TCCR2A = 2;
  TCCR2B = 0;
  TIMSK2 = 2;
  SIRENE_DDR |= (1 << SIRENE_PIN);
}

ISR(TIMER2_COMPA_vect)
{
  if (!tone2_pause)
  {
    if (tone2_state)
    {
      SIRENE_PORT |= (1 << SIRENE_PIN);
      tone2_state = 0;
    }
    else
    {
      SIRENE_PORT &= ~(1 << SIRENE_PIN);
      tone2_state = 1;
    }
  }
  if ((--tone2_len) == 0)
  {
    TCCR2B = 0;
    tone2_pause = 0;
    next_note();
  }
}

void tone2(uint16_t freq, uint16_t duration)
{
  uint32_t period = ((uint32_t)1000000) / (uint32_t)freq;

  if (freq >= 977)  // prescaler 32
  {
    tone2_state = 0;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    if (tone2_len == 0) tone2_len++;
    TCNT2 = 0;
    OCR2A = (uint8_t) (250000 / (uint32_t)freq);
    TCCR2B = 3;
  }
  else if (freq >= 488) // prescaler 64
  {
    tone2_state = 0;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    if (tone2_len == 0) tone2_len++;
    TCNT2 = 0;
    OCR2A = (uint8_t) (125000 / (uint32_t)freq);
    TCCR2B = 4;
  }
  else if (freq >= 244) // prescaler 128
  {
    tone2_state = 0;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    if (tone2_len == 0) tone2_len++;
    TCNT2 = 0;
    OCR2A = (uint8_t) (62500 / (uint32_t)freq);
    TCCR2B = 5;
  }
  else if (freq >= 122) //prescaler 256
  {
    tone2_state = 0;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    if (tone2_len == 0) tone2_len++;
    TCNT2 = 0;
    OCR2A = (uint8_t) (31250 / (uint32_t)freq);
    TCCR2B = 6;
  }
  else if (freq >= 30) //prescaler 1024
  {
    tone2_state = 0;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    if (tone2_len == 0) tone2_len++;
 
   TCNT2 = 0;
    OCR2A = (uint8_t) (7813 / (uint32_t)freq);
    TCCR2B = 7;
  }
  else if (freq == 0)
  {
    tone2_pause = 1;
    tone2_state = 0;

    period = 1000000 / 500;
    tone2_len = ((uint32_t)duration * (uint32_t)1000) * 2 / period;
    TCNT2 = 0;
    OCR2A = (uint8_t) (125000 / (uint32_t)500);
    TCCR2B = 4;
  }
  else
  {
    TCCR2B = 0;
  }
}

void pip()
{
  if (notes_remaining == 0) tone2(698, 30);
}

void zastav_melodiu()
{
  notes_remaining = 0;
}
