Servo l, r;
int jazda;

void setup() {
  pinMode(5, INPUT);
  l.attach(9);
  r.attach(10);
  l.write(90);
  r.write(90);
  jazda = 0;
}

void loop() {
  if (jazda)
  {
    if (digitalRead(5) == 1)
    {
      l.write(90);
      r.write(90);
      delay(500);
      jazda = 0;
    }
  }
  else
  {
    if (digitalRead(5) == 1)
    {
      l.write(180);
      r.write(0);#define MP3_OUTPUT_PIN 5   // connect to Rx pin of DFPlayer
#define MP3_BUSY_PIN 8     // connect to BUSY pin of DFPlayer

void setup() 
{
 Serial.begin(9600);
 mp3_set_volume(30);
 delay(10);
 mp3_play(1);
}

void loop() 
{
  Serial.println(digitalRead(MP3_BUSY_PIN));    // shows 0 while playing, 1 when idle
  delay(300);
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

      delay(500);
      jazda = 1;
    }    
  }
}

