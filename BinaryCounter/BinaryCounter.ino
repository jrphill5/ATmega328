const int number = 1024;
const int del = 100;

void setup()
{

	for ( int i = 1; i < 13; i++ )
		pinMode(i, OUTPUT);

}
	
void loop()
{

	for ( int i = 0; i < number; i++ )
	{

		int number = i;
		for ( int j = 1; j < 13; j++ )
		{

			int numValue = number / 2;
			int bitValue = number % 2;
			digitalWrite(j, (bitValue) ? HIGH : LOW);
			number = numValue;

		}
		delay(del);

	}

} 
