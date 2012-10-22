/*
 *  AS3935 Lightning Sensor
 *
 *  IRQ:  pin 6
 *  CS:   pin 7
 *  MOSI: pin 11
 *  MISO: pin 12
 *  SCK:  pin 13
 */
 
#include <SPI.h>

const byte INT_NH = 0x01;
const byte INT_D  = 0x04;
const byte INT_L  = 0x08;

const byte IRQ = 6;
const byte CS  = 7;

void setup()
{

  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);

  pinMode(IRQ, INPUT);
  pinMode(CS,  OUTPUT);
  digitalWrite(CS, HIGH);
  
  writeRegister(0x08, 0x00);
  
  delay(100);

  Serial.println("Waiting for lightning events...");

}

void loop()
{

  if( digitalRead(IRQ) == HIGH )
  {
    
    Serial.println();
    
    delay(2);
    
    unsigned long time = millis()/1000;
    byte type = (readRegister(0x03) & 0x0F);
    switch( type )
    {
      
      case 0x00:
        Serial.print("Distance has changed! [");
        Serial.print(time);
        Serial.println("]");
        stormDistance();
        break;
      case INT_NH:
        Serial.print("Noise level too high! [");
        Serial.print(time);
        Serial.println("]");
        break;
      case INT_D:
        Serial.print("Disturber detected! [");
        Serial.print(time);
        Serial.println("]");
        strikeEnergy();
        break;
      case INT_L:
        Serial.print("Lightning strike detected! [");
        Serial.print(time);
        Serial.println("]");
        stormDistance();
        strikeEnergy();
        break;
      default:
        Serial.print("Invalid interrupt code! [");
        Serial.print(time);
        Serial.println("]");
        break;
        
    }

  }
  
}

void stormDistance()
{
      
  byte distance = (readRegister(0x07) & 0x3F);
  
  if( distance == 0x3F )
    Serial.println("Storm out of range.");
  else
  {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println("km");
  }

}

void strikeEnergy()
{

  long energy = (readRegister(0x06) & 0x0F);
  energy << 8;
  energy |= readRegister(0x05);
  energy << 8;
  energy |= readRegister(0x04);

  Serial.print("Energy: ");
  Serial.println(energy);
  
}

byte readRegister(byte reg)
{

  reg &= 0x7F; // Register bit 1 should be 0 always.
  reg |= 0x40; // Register bit 2 should be 1 for read.

  digitalWrite(CS, LOW);
  SPI.transfer(reg);
  byte data = SPI.transfer(0x00);
  digitalWrite(CS, HIGH);

  return data;

}

void writeRegister(byte reg, byte data)
{

  reg &= 0x7F; // Register bit 1 should be 0 always.
  reg &= 0xBF; // Register bit 2 should be 0 for write.

  digitalWrite(CS, LOW);
  SPI.transfer(reg);
  SPI.transfer(data);
  digitalWrite(CS, HIGH);

}
