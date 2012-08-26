#include "TM1638.h"
#include "TM1640.h"

// define a module on data pin 5, clock pin 6
TM1640 module(5, 6);

unsigned long lcount,hcount;
char buffer[16];

void setup()
{
  lcount = 0;
  hcount = 0;
}

void loop()
{
  sprintf(buffer,"%08lu%08lu",hcount,lcount);
  module.setDisplayToString(buffer, 0b0000000000000000);
  lcount++;
  if( lcount == 100000000 )
  {
    lcount = 0;
    hcount++;
    if( hcount == 100000000 )
    {
      hcount = 0;
    }
  }
}
