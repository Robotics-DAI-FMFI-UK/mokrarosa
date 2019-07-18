#define TRIG 4
#define ECHO 5

void setup() 
{
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
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

void loop() 
{
  Serial.println(meraj());
  delay(100);
}
