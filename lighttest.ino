void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(0); 

  Serial.print("The light level is ");
  Serial.println(sensorValue);
  delay(1000);
}
