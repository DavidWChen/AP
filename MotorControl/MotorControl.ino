#define pmw 16
#define upper_bound 100

void setup() {
  Serial.begin(9600);
  pinMode(pmw, OUTPUT);
  analogWrite(pmw, 0);
}

void loop() {
  
  for (int i = 0; i <= upper_bound; i++)
  {
    analogWrite(pmw, i);
    delay(10);
  }
  
  for (int i = upper_bound; i >= 0; i--)
  {
    analogWrite(pmw, i);
    delay(10);
  }
  
}
