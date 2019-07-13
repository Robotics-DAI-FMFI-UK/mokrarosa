void setup() 
{
  Serial.begin(9600);  
}

float zmeraj_baterku()
{
  analogReference(INTERNAL); 
  float volt = analogRead(A3); 
  analogReference(DEFAULT);
  return volt * 0.01181640625;  // volt * 1.1 * 242 / 22 / 1023  (22 KOhm out of 220+22=242 KOhm)
}

void loop() 
{
  float bat = zmeraj_baterku();
  Serial.println(bat);
}
