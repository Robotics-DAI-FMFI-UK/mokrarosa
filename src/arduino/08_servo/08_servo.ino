int a;

void setup() {
  a = 1000;
  pinMode(4, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(4, HIGH);
  delayMicroseconds(a);
  digitalWrite(4, LOW);
  delay(18);
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == '+') a += 5;
    if (c == '-') a -= 5;
  }
}

