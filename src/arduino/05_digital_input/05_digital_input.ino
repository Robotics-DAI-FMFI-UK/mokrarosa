#define SENSOR 4

void setup() 
{
  pinMode(SENSOR, INPUT);
  Serial.begin(9600);
}

void loop() 
{
  if (digitalRead(SENSOR) == 0)
    Serial.println("0");
  else Serial.println("1");
  delay(200);
}

