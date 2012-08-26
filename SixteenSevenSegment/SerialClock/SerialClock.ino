#include "TM1638.h"
#include "TM1640.h"

// define a module on data pin 5, clock pin 6
TM1640 module(5, 6);

char buffer[16] = {0};
char value;
char bytes;
unsigned int points = 0b0000000000000010;

void setup()
{

	module.setDisplayToString(buffer, points);

	Serial.begin(9600);

}

void loop()
{

	if( Serial.available() > 0 )
	{
		// begin transmission
		if( (value = Serial.read()) == '^' )
		{

			bytes = 0;

			// read sixteen bytes
			while( bytes < 16 )
			{
				if( Serial.available() > 0)
				{
					value = Serial.read();
					if( value == '$' ) break;
					buffer[bytes] = value;
					Serial.print(value);
					bytes++;
				}
			}

			Serial.println(" ... updating!");
			module.setDisplayToString(buffer, points);

		}

	}

}
