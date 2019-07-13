void setup() 
{
  Serial.begin(9600);
  Serial.print("Ahoj, mas sa dobre? [a/n]:");
}

void loop() 
{
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 'a') Serial.println("To je super!");
    if (c == 'n') Serial.println("Nevadi, zajtra bude lepsie");
  }
}

