#include "SPI.h"

const char RTCCS = 7;

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
	RTC_read(0x0F);

	Serial.println("RTC Setup Complete!");

}

void loop()
{

	byte year   = RTC_read(0x06);
	byte month  = RTC_read(0x05);
	byte date   = RTC_read(0x04);
	byte day    = RTC_read(0x03);
	byte hour   = RTC_read(0x02);
	byte minute = RTC_read(0x01);
	byte second = RTC_read(0x00);

	char buffer[20];
	snprintf(buffer, sizeof(buffer), "20%02X-%02X-%02X %02X:%02X:%02X", year, month, date, hour, minute, second);
	Serial.println(buffer);

	delay(1000);

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
