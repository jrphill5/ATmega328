#define PwmPinMotorA 10
#define DirectionPinMotorA 12
#define PwmPinMotorB 11
#define DirectionPinMotorB 13

int azipin = 0;
int elevpin = 1;
int min,sec,count,check,back=0;

float sunazi,sunelev,sunazi_v,sunelev_v,time;
float trackazi_v,trackelev_v;
const float tol = 0.05;
const int motdel = 500;
//const float vazi_max = ;
//const float vazi_min = ;
const float velev_max = 3.2;
const float velev_min = 2.0;

const float anglesTable[61][3] = {

    {  0,  98.87,  0.50},
    { 10, 100.36,  2.29},
    { 20, 101.85,  4.18},
    { 30, 103.36,  6.10},
    { 40, 104.90,  8.03},
    { 50, 106.46,  9.95},
    { 60, 108.05, 11.86},
    { 70, 109.67, 13.76},
    { 80, 111.33, 15.65},
    { 90, 113.03, 17.51},
    {100, 114.78, 19.35},
    {110, 116.58, 21.17},
    {120, 118.43, 22.96},
    {130, 120.34, 24.71},
    {140, 122.32, 26.43},
    {150, 124.36, 28.12},
    {160, 126.49, 29.76},
    {170, 128.69, 31.36},
    {180, 130.97, 32.91},
    {190, 133.35, 34.41},
    {200, 135.82, 35.85},
    {210, 138.39, 37.22},
    {220, 141.06, 38.53},
    {230, 143.84, 39.76},
    {240, 146.72, 40.91},
    {250, 149.71, 41.97},
    {260, 152.80, 42.94},
    {270, 156.00, 43.82},
    {280, 159.29, 44.59},
    {290, 162.67, 45.25},
    {300, 166.12, 45.79},
    {310, 169.57, 45.25},
    {320, 172.95, 44.59},
    {330, 176.24, 43.82},
    {340, 179.44, 42.94},
    {350, 182.53, 41.97},
    {360, 185.52, 40.91},
    {370, 188.40, 39.76},
    {380, 191.18, 38.53},
    {390, 193.85, 37.22},
    {400, 196.42, 35.85},
    {410, 198.89, 34.41},
    {420, 201.27, 32.91},
    {430, 203.55, 31.36},
    {440, 205.75, 29.76},
    {450, 207.88, 28.12},
    {460, 209.92, 26.43},
    {470, 211.90, 24.71},
    {480, 213.81, 22.96},
    {490, 215.66, 21.17},
    {500, 217.46, 19.35},
    {510, 219.21, 17.51},
    {520, 220.91, 15.65},
    {530, 222.57, 13.76},
    {540, 224.19, 11.86},
    {550, 225.78,  9.95},
    {560, 227.34,  8.03},
    {570, 228.88,  6.10},
    {580, 230.39,  4.18},
    {590, 231.88,  2.29},
    {600, 233.37,  0.50},

};

void setup()
{

    //check = 0;
    Serial.begin(9600);
    time = 0;

}

void loop()
{

    check = 0;
    while(check < 62)
    {

        //we will read our calculated sun position
        sunazi = anglesTable[check][1];
        sunelev = anglesTable[check][2];
        Serial.print("Angles:");
        Serial.print(sunazi);//value
        Serial.print("\t");
        Serial.print(sunelev);//value
        Serial.print("\t"); 
        Serial.print("Time:");
        Serial.print("\t");
        Serial.print(time); //value
        Serial.print("\n"); // newline
        Serial.println(""); // go back to left  
    
        // slope and intercept were for voltage to angle, so invert
        sunazi_v =(sunazi - 19.11)/(63.343);
        sunelev_v =(sunelev - 213.69)/(-61.64);
        
        // keep from looping to get to out of range tilts
        if (sunelev_v > velev_max - tol/2)
           sunelev_v = velev_max;
        if (sunelev_v < velev_min + tol/2)   
           sunelev_v = velev_min;

        //compare angles and align tracker
        while(abs(sunazi_v-trackazi_v)>tol || abs(sunelev_v-trackelev_v)>tol)
        {

            //read angle via input lines voltages
            trackazi_v = analogRead(azipin);
            trackazi_v = trackazi_v*5/1023;
            trackelev_v = analogRead(elevpin);
            trackelev_v = trackelev_v*5/1023;  
      
            // print labels for troubleshooting
            Serial.print("Goto:");
            Serial.print("\t");
            Serial.print(sunazi_v);
            Serial.print("\t");
            Serial.print(sunelev_v);
            Serial.print("\t");
            Serial.print("Current:");
            Serial.print("\t");
            Serial.print(trackazi_v);
            Serial.print("\t");
            Serial.print(trackelev_v);
            Serial.print("\n");

            if(sunazi_v > trackazi_v+tol)
            {

                // turn AZI motor forward, 1 sec
                analogWrite(PwmPinMotorA,255);
                digitalWrite(DirectionPinMotorA, HIGH);
                delay(motdel);
                time += motdel;
                analogWrite(PwmPinMotorA,0);

            }

            if(sunazi_v < trackazi_v-tol)
            {

                // turn AZI motor reverse, 1 sec
                analogWrite(PwmPinMotorA,255);
                digitalWrite(DirectionPinMotorA, LOW);
                delay(motdel);
                time += motdel;
                analogWrite(PwmPinMotorA,0);

            }

            if((sunelev_v > trackelev_v+tol)&&(sunelev_v < 3.2))
            {

                // turn ELEV motor forward, 1 sec
                analogWrite(PwmPinMotorB,255);
                digitalWrite(DirectionPinMotorB, HIGH);
                delay(motdel);
                time += motdel;
                analogWrite(PwmPinMotorB,0);

            }

            if((sunelev_v < trackelev_v-tol)&&(sunelev_v > 2.00)) 
            { 

                // turn ELEV motor reverse, 1 sec
                analogWrite(PwmPinMotorB,255);
                digitalWrite(DirectionPinMotorB, LOW);
                delay(motdel);
                time += motdel;
                analogWrite(PwmPinMotorB,0);

            }

            delay(3000);
            time += 3000;

        }

        check++;
        // wait a whole day
        delay(15000);
        time += 15000;

    }

}
