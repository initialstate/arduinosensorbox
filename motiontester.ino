#define PIR_MOTION_SENSOR 3//Use pin 3 to receive the signal from the module 

void setup()
{
	Serial.begin(9600);
	pinMode(PIR_MOTION_SENSOR, INPUT);
}

void loop() 
{
	delay(2000);
	int sensorValue = digitalRead(PIR_MOTION_SENSOR);

	if(sensorValue==HIGH)//if it detects moving people
		Serial.println("Motion Detected");
	else
		Serial.println("No Movement");
}
