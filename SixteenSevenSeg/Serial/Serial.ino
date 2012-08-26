#include "TM1638.h"// yes you need both
#include "TM1640.h"

// define a module on data pin 5, clock pin 6
TM1640 module(5, 6);

char buffer[16];
byte counter;

void setup()
{
  
  counter = 0;
  Serial.begin(9600);
  
}

void loop()
{
  
  if( Serial.available() )
  {
   
    buffer[counter] = Serial.read();
    module.setDisplayToString(buffer, 0b0000000000000000);

    counter++; if( counter == 16 ) counter = 0;
    
  }
    
}
