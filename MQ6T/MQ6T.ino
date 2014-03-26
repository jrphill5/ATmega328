unsigned short max = 3000;
unsigned short set = 250;
unsigned short val = 0;

#define ALARM_PIN 10
#define MQ6T_PIN  0

void setup()
{

	Serial.begin(9600);
	pinMode(MQ6T_PIN, OUTPUT);

}

void loop()
{

	val = analogRead(ALARM_PIN);
	while(val > set)
	{
		Serial.print(val);
		Serial.println(" - ALARM");
		for( unsigned short i = 0; i < max; i++ )
		    tone(ALARM_PIN, 4100 + 1000*sin(2*3.141592654*i/max));
		val = analogRead(ALARM_PIN);
	}
	noTone(ALARM_PIN);
	Serial.print(val);
	delay(1000);

}
