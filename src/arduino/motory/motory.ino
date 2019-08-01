#include <Servo.h>

Servo motors[8];
int ms[] = {11,10,8,9,3,5,6,7};
char *mnames[] = {"LZ2", "LZ1", "LP2", "LP1", "PZ2", "PZ1", "PP2", "PP1"};
int mi;
int a[8];

void setup() {
  mi = 0;
  for (int i = 0; i < 8; i++)
  {
    motors[i].attach(ms[i]);
    a[i] = 90;
  }
  Serial.begin(9600);
}

void loop() {
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == '+') a[mi]++;
    if (c == '-') a[mi]--;
    if (c == ' ') mi = (mi + 1) % 8;
    Serial.print(mnames[mi]);
    Serial.print(": ");
    Serial.println(a[mi]);
    motors[mi].write(a[mi]);
  }
}
