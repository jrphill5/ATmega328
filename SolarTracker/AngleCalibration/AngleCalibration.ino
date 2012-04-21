// to help us get the pot voltages calibrated
#define PwmPinMotorA 10
#define DirectionPinMotorA 12
#define PwmPinMotorB 11
#define DirectionPinMotorB 13

int azipin = 0; // select the input pin for the potentiometer
int elevpin = 1;
int min,sec,count,check,back=0;

float sunazi,sunelev,sunazi_v,sunelev_v, time;
float trackazi_v,trackelev_v;
const float tol=0.05;
const int motdel=500;
const float vazi_max = 4.1;
const float vazi_min = 1.1;
const float velev_max = 3.2;
const float velev_min = 2.0;
char c;

void setup()
{

    //check = 0;
    Serial.begin(9600);
    time = 0;

}

void loop()
{

    trackazi_v = analogRead(azipin);
    trackazi_v = trackazi_v*5/1023;
    trackelev_v = analogRead(elevpin);
    trackelev_v = trackelev_v*5/1023;

    // print labels for troubleshooting
    Serial.print("Current:");
    Serial.print("\t");
    Serial.print(trackazi_v);
    Serial.print("\t"); 
    Serial.print(trackelev_v);
    Serial.print("\n");

    Serial.print("Which way do I go, boss?");
    Serial.print("\n");
    Serial.println("");

    c = Serial.read();
    switch( c )
    {

        case 's':
            delay(500);
            break;

        case 'r':
            // turn AZI motor forward, 1 sec
            analogWrite(PwmPinMotorA,255);
            digitalWrite(DirectionPinMotorA, HIGH);
            delay(motdel);
            time = time + motdel;
            analogWrite(PwmPinMotorA,0);
            break;

        case 'l':
            // turn AZI motor reverse, 1 sec
            analogWrite(PwmPinMotorA,255);
            digitalWrite(DirectionPinMotorA, LOW);
            delay(motdel);
            time = time + motdel;
            analogWrite(PwmPinMotorA,0);
            break;

        case 'd':
            // turn ELEV motor forward, 1 sec
            analogWrite(PwmPinMotorB,255);
            digitalWrite(DirectionPinMotorB, HIGH);
            delay(motdel);
            time = time + motdel;
            analogWrite(PwmPinMotorB,0);
            break;

        case 'u':
            // turn ELEV motor reverse, 1 sec
            analogWrite(PwmPinMotorB,255);
            digitalWrite(DirectionPinMotorB, LOW);
            delay(motdel);
            time = time + motdel;
            analogWrite(PwmPinMotorB,0);
            break;

    }

    delay(3000);

}
