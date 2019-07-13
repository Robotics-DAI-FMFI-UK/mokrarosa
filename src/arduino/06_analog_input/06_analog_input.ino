#define SENSOR 4

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  int hodnota = analogRead(SENSOR);
  Serial.println(hodnota);
  delay(200);
}

