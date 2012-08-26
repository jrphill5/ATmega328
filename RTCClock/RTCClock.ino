#define DATAOUT 11 //MOSI
#define DATAIN 12 //MISO
#define SPICLOCK 13 //sck
#define RTC_CHIPSELECT 7 // chip select (ss/ce) for RTC, active high
#define LED 10

byte clr;
//Function SPI_Transfer
char spi_transfer(volatile char data)
{
/*
Writing to the SPDR register begins an SPI transaction
*/
SPDR = data;
/*
Loop here until the transaction complete.
SPIF bit = SPI Interrupt Flag. When interrupts are enabled, &
SPIE bit = 1 enabling SPI interrupts, this bit sets =1 when
transaction finished.
*/
while (!(SPSR & (1<<SPIF)))
{

};
// received data appears in the SPDR register

return SPDR;
}
void setup()
{

char in_byte;
clr = 0;
in_byte = clr;

Serial.begin(9600);
// set direction of pins

pinMode(LED, OUTPUT);
pinMode(DATAOUT, OUTPUT);

pinMode(DATAIN, INPUT);
pinMode(SPICLOCK,OUTPUT);

pinMode(RTC_CHIPSELECT,OUTPUT);
digitalWrite(RTC_CHIPSELECT,LOW); //disable RTC

// set up the RTC by enabling the oscillator, disabling the write protect in the control register,
// enabling AIE0 and AIE1 and the 1HZ Output

// 0×8F to 00000111 = 0×07
// EOSC Active Low

// WP Active High, so turn it off
in_byte = read_rtc_register(0x0F);

Serial.print("CTRL REG [");
Serial.print(in_byte, HEX);
write_rtc_register(0x8F,0x01|0x02|0x04);
// little sanity checks
in_byte = read_rtc_register(0x0F);

Serial.print("CTRL REG [");
Serial.print(in_byte, HEX);

Serial.println("]");
delay(10);

in_byte = read_rtc_register(0x10);
Serial.print("STATUS REG [");

Serial.print(in_byte, BIN);
Serial.println("]");

// set up both alarms at 00 seconds?
write_rtc_register(0x87,0x00);

// mask all the other registers
write_rtc_register(0x88,0x80);

write_rtc_register(0x89,0x80);
write_rtc_register(0x8A,0x80);

write_rtc_register(0x8B,0x00);
write_rtc_register(0x8C,0x80);

write_rtc_register(0x8D,0x80);
write_rtc_register(0x8E,0x80);

in_byte = read_rtc_register(0x06);
Serial.print("YEAR [");

Serial.print(in_byte, HEX);
Serial.println("]");

in_byte = read_rtc_register(0x05);
Serial.print("MONTH [");

Serial.print(in_byte, HEX);
Serial.println("]");

digitalWrite(LED, HIGH);

}
//End Function

void write_rtc_register(char register_name, byte data)
{
write_register(register_name, data, RTC_CHIPSELECT, HIGH, true, true);
}

char read_rtc_register(char register_name)
{
return read_register(register_name, RTC_CHIPSELECT, HIGH, false, true);
}

// reads a register
char read_register(char register_name, byte cs_pin, byte cs_active_level, boolean read_high, boolean cpha_trailing)
{
char in_byte;
if(cpha_trailing)
{
SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA)|(0<<SPR1)|(0<<SPR0);
}
else
{
SPCR = (1<<SPE)|(1<<MSTR)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
}
clr = SPCR;
clr = SPDR;
if(read_high)
{
// need to set bit 7 to indicate a read for the slave device
register_name |= 128;
}
else
{
// if read low, means A7 bit should be cleared when reading for the slave device
register_name &= 127;
}
// SS is active low
digitalWrite(cs_pin, cs_active_level);
// send the address of the register we want to read first
spi_transfer(register_name);
// send nothing, but here‘s when the device sends back the register‘s value as an 8 bit byte
in_byte = spi_transfer(0);
// deselect the device..
if(cs_active_level == HIGH)
{
digitalWrite(cs_pin, LOW);
}
else
{
digitalWrite(cs_pin, HIGH);
}
return in_byte;
}

// write to a register
// write_high if true indicates set A7 bit to 1 during a write
void write_register(char register_name, byte data, byte cs_pin, byte cs_active_level, boolean write_high, boolean cpha_trailing)
{
if(cpha_trailing)
{
SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA)|(0<<SPR1)|(0<<SPR0);
}
else
{
SPCR = (1<<SPE)|(1<<MSTR)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
}
clr=SPCR;
clr=SPDR;
// char in_byte;
if(write_high)
{
// set A7 bit to 1 during a write for this device
register_name |= 128;
}
else
{
// clear bit 7 to indicate we‘re doing a write for this device
register_name &= 127;
}
// SS is active low
digitalWrite(cs_pin, cs_active_level);
// send the address of the register we want to write
spi_transfer(register_name);
// send the data we‘re writing
spi_transfer(data);
if(cs_active_level == HIGH)
{
digitalWrite(cs_pin, LOW);
}
else
{
digitalWrite(cs_pin, HIGH);
}
//return in_byte;
}

void loop()
{

char hour   = read_rtc_register(0x02);
char minute = read_rtc_register(0x01);
char second = read_rtc_register(0x00);

char buffer[9];
snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X", hour, minute, second);
Serial.println(buffer);

digitalWrite(LED, LOW);
delay(500);

}
