#include "TM1638.h"
#include "TM1640.h"
#include "Time.h"

// define a module on data pin 5, clock pin 6
TM1640 module(5, 6);

char buffer[17] = {0};
unsigned int points = 0b0101000010101000;

int vyear, vmonth, vday, vhour, vminute, vsecond;

void setup()
{

	Serial.begin(9600);

	char bytes = 0;
	while( bytes < 17 )
		if( Serial.available() > 0)
		{

			char value = Serial.read();
			if( value == 127 )
			{
				if( bytes > 0 )
				{
					bytes--;
					Serial.print((char)8);
					Serial.print(" ");
					Serial.print((char)8);
				}
				continue;
			}
			buffer[bytes] = value;
			Serial.print(value);
			bytes++;

		}

	sscanf(buffer, "%2d %2d %2d %2d %2d %2d", &vyear, &vmonth, &vday, &vhour, &vminute, &vsecond);

	Serial.println();
	Serial.print(vyear, DEC);
	Serial.print(" ");
	Serial.print(vmonth, DEC);
	Serial.print(" ");
	Serial.print(vday, DEC);
	Serial.print(" ");
	Serial.print(vhour, DEC);
	Serial.print(" ");
	Serial.print(vminute, DEC);
	Serial.print(" ");
	Serial.print(vsecond, DEC);
	Serial.println();

	cli();
	//set timer1 interrupt at 1Hz
	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments
	OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS12 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);  
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	sei();

	setTime(vhour,vminute,vsecond,vday,vmonth,vyear);
	set_1Hz_ref(now(), 2, onTick, 0);

}

boolean toggle1 = 0;

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle1){
    digitalWrite(13,HIGH);
    toggle1 = 0;
  }
  else{
    digitalWrite(13,LOW);
    toggle1 = 1;
  }
}

void onTick(){
}

void loop()
{

	sprintf(buffer, "%02d%02d%02d %02d%02d%02d%03d", month(), day(), year()%100, hour(), minute(), second(), millisecond());
	module.setDisplayToString(buffer, points);

}
