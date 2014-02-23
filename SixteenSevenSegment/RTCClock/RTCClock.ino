#include "Wire.h"
#include "Time.h"
#include "TM1638.h"
#include "TM1640.h"

#define RTC_CTRL_ID B1101000
TM1640 module(5, 6);

char display[17] = {0};
char datetime[17] = {0};
int rtc[7] = { 0, 0, 0, 0, 0, 0, 0 };
unsigned int points = 0b0001010000101010;
char setTimeFlag = 0;

char DECtoBCD( char value )
{

    return ( value / 10 << 4 ) + value % 10;

}

char BCDtoDEC( char value )
{

    return (( value & 0xF0 ) >> 4 ) * 10 + ( value & 0x0F );

}

void writeRTC()
{

	Wire.beginTransmission( RTC_CTRL_ID );
	Wire.write( (uint8_t)0x00 );
	for( int i = 0; i < 7; i++ )
		Wire.write( DECtoBCD( rtc[i] ) );
	Wire.endTransmission();

}

void readRTC()
{

	Wire.beginTransmission( RTC_CTRL_ID );
	Wire.write( (uint8_t)0x00 );
	Wire.endTransmission();

	Wire.requestFrom( RTC_CTRL_ID, 7 );
	for( int i = 0; i < 7; i++ )
		if( Wire.available() )
			rtc[i] = BCDtoDEC( Wire.read() );

}

void printRTC()
{

	sprintf( datetime, "%02d/%02d/%02d %02d:%02d:%02d", rtc[5], rtc[4], rtc[6], rtc[2], rtc[1], rtc[0] );

	Serial.println( datetime );

}

void serialReadTimeISR()
{

	setTimeFlag = ( setTimeFlag ) ? 0 : 1;

}

void serialReadTime()
{

	sprintf( display, "%16s", "  SEND  SERIAL  ");
	module.setDisplayToString(display, 0x00);

	char bytes = 0;
	while( bytes < 17 )
	{

		if( setTimeFlag == 0 ) return;

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

			datetime[bytes] = value;
			Serial.print(value);
			bytes++;

		}

	}

	sscanf(datetime, "%2d %2d %2d %2d %2d %2d", &rtc[6], &rtc[5], &rtc[4], &rtc[2], &rtc[1], &rtc[0]);

	writeRTC();

	setTimeFlag = false;

}

void setup()
{

	Wire.begin();
	Wire.beginTransmission(RTC_CTRL_ID);
	Wire.write(0x0E);
	Wire.write(0x00);
	Wire.endTransmission();

	attachInterrupt(1, serialReadTimeISR, FALLING);

	Serial.begin(9600);
	//serialReadTime();

}

void onTick()
{

}

void loop()
{

	if( setTimeFlag ) serialReadTime();

	readRTC();
	setTime(rtc[2], rtc[1], rtc[0], rtc[4], rtc[5], rtc[6]);
    set_1Hz_ref(now(), 2, onTick, FALLING);

	sprintf( display, "%02d%02d%02d%02d %02d%02d%02d%01d", 20, rtc[6], rtc[5], rtc[4], rtc[2], rtc[1], rtc[0], millisecond()/100 );
	module.setDisplayToString(display, points);

}
