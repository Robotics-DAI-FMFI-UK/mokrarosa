#include <Servo.h>

#define LP1 0
#define LZ1 1
#define PP1 2
#define PZ1 3
#define LP2 4
#define LZ2 5
#define PP2 6
#define PZ2 7

#define MAX_SEQ_LENGTH 100

Servo legs[8];
uint8_t legv[8];
char pluskey[] = {'1', '2', '3', '4', '!', '@', '#', '$'};
char minuskey[] = {'0', '9', '8', '7', ')', '(', '*', '&'};
char key_step_plus = '+';
char key_step_minus = '-';
int8_t step_size;
int seq_length;
uint8_t seq[MAX_SEQ_LENGTH][8];
uint8_t delaj[MAX_SEQ_LENGTH];
uint8_t del = 0;

uint8_t initial[] = {104, 55, 55, 125, 17, 17, 17, 17};

void setup() 
{
  Serial.begin(9600);
  legs[LP1].attach(3);
  legs[LZ1].attach(4);
  legs[PP1].attach(10);
  legs[PZ1].attach(8);

  legs[LP2].attach(2);
  legs[LZ2].attach(5);
  legs[PP2].attach(11);
  legs[PZ2].attach(9);
  Serial.println(F("Hi!"));

  for (int i = 0; i < 8; i++)
    legv[i] = initial[i];
    
  seq_length = 0;

  for (int i = 0; i < 8; i++)
    legs[i].write(legv[i]);
  step_size = 1;
  print_usage();
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

void play_sequence()
{
  for (int i = 0; i < seq_length; i++)
  {
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
  for (int i = 0; i < 8; i++)
    if (seq[seq_length][i] != legv[i]) changed = 1;
  if (changed)
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
    if (Serial.read() == 'n') return;
  }
  for (int i = 0; i < 8; i++)
  {
    legv[i] = seq[seq_length - 1][i];
    legs[i].write(legv[i]);
  }
}

void loop() 
{
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
      play_sequence();
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
  delay(1);
}