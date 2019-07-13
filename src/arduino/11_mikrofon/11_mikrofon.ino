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
      r.write(0);
      delay(500);
      jazda = 1;
    }    
  }
}

