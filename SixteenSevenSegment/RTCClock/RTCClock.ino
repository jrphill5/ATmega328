#include "SPI.h"

#define RTCCS 7

#define RTC_SECOND	0x00
#define RTC_MINUTE	0x01
#define RTC_HOUR	0x02
#define RTC_DAY		0x03
#define RTC_DATE	0x04
#define RTC_MONTH	0x05
#define RTC_YEAR	0x06

void RTC_write(byte address, byte data);
byte RTC_read(byte address);

void setup()
{

	Serial.begin(9600);

	pinMode(RTCCS, OUTPUT);

	SPI.begin();
	SPI.setDataMode(SPI_MODE1);
	SPI.setBitOrder(MSBFIRST);
	digitalWrite(RTCCS, LOW);

	RTC_write(0x8F,0x07);

	char year, month, date, hour, minute, second;

	Serial.println("Send time data now...");

	while( Serial.available() < 6 );

	if( Serial.available() >= 6 )
	{

		year   = Serial.read();
		month  = Serial.read();
		date   = Serial.read();
		hour   = Serial.read();
		minute = Serial.read();
		second = Serial.read();

	}

	RTC_write(RTC_YEAR, year);
	RTC_write(RTC_MONTH, month);
	RTC_write(RTC_DATE, date);
	RTC_write(RTC_HOUR, hour);
	RTC_write(RTC_MINUTE, minute);
	RTC_write(RTC_SECOND, second);

	Serial.println("RTC Setup Complete!");

}

void loop()
{

	byte year   = RTC_read(RTC_YEAR);
	byte month  = RTC_read(RTC_MONTH);
	byte date   = RTC_read(RTC_DATE);
	byte day    = RTC_read(RTC_DAY);
	byte hour   = RTC_read(RTC_HOUR);
	byte minute = RTC_read(RTC_MINUTE);
	byte second = RTC_read(RTC_SECOND);

	char buffer[20];
	snprintf(buffer, sizeof(buffer), "20%02X-%02X-%02X %02X:%02X:%02X", year, month, date, hour, minute, second);
	Serial.println(buffer);

	delay(500);

}

// First bit of address byte is 1 when writing.
void RTC_write(byte address, byte data)
{

	digitalWrite(RTCCS, HIGH);
	SPI.transfer(address |= 0x80);
	SPI.transfer(data);
	digitalWrite(RTCCS, LOW);

}

// First bit of address byte is 0 when reading.
byte RTC_read(byte address)
{

	digitalWrite(RTCCS, HIGH);
	SPI.transfer(address &= 0x7F);
	byte data = SPI.transfer(0x00);
	digitalWrite(RTCCS, LOW);
	return data;

}
