int gasVal;    // analog value read from the gas sensor on pin A0
int LPIN = 2;  // port number of the lowest value LED
int HPIN = 11; // port number of the highest value LED

void light( int num );

void setup()
{

    Serial.begin(9600);
    for ( int i = LPIN; i <= HPIN; i++ )
        pinMode(i, OUTPUT);

}

void loop()
{

    gasVal = analogRead(0);
    Serial.println(gasVal, DEC);
    light(gasVal/100 - 1);
    delay(250);

}

// light up string of LEDs from port LPIN to port num
void light( int num )
{
 
    for ( int i = LPIN; i <= num + LPIN; i++ )
        digitalWrite(i, HIGH);
    for ( int i = num + LPIN + 1; i <= HPIN; i++ )
        digitalWrite(i, LOW);
  
}
