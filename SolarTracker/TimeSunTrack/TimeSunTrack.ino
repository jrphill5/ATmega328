// works to provide the angles for Arduino time, matching the NOAA spreadsheet 'real time'
// tracking is based on axial azi and elev pot voltages
// sync message: letter T followed by ten digit unix time (sec since Jan 1 1970)

#include <Time.h>
#include <math.h>

// for sync
#define TIME_MSG_LEN 11  // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T' // Header tag for serial time sync message
#define TIME_REQUEST 7   // ASCII bell character requests a time sync message

// for tracking
#define PwmPinMotorA       10
#define DirectionPinMotorA 12
#define PwmPinMotorB       11
#define DirectionPinMotorB 13

int azipin = 0;  // select the input pin for the potentiometer
int elevpin = 1;
int tvpin = 2;   // tracker voltage pin
int fvpin = 3;   // fixed voltage pin

int min,sec,count,check,back = 0;

float sunazi_v,sunelev_v;
float trackazi_v,trackelev_v,tv,fv;
const float tol = 0.05;
const int motdel = 500;
const float vazi_max = 4.1;
const float vazi_min = 1.1;
const float velev_max = 3.2;
const float velev_min = 2.0;

// for sunpos
const float lati = 36.2142983;  // broyhill
const float longi = -81.693161;
const float datli = -4.0;       // dateline, where your longitude is
// -4 while on daylight savings time, -5 otherwise?

int i;
//float dater = 40382; // july 23 2010 // days since jan 1 1900
float dater = 25569;   // jan 1 1970, days since jan 1 1900
//float tday = 7.0/24; // start at 7am
float tday,julday,julcent,gmls,gmas,eeo,seoc,stl,sta,srv,sal,moe,oc,sra;
float sd,vy,eot,has,sn,srt,sst,sdur,tst,ha,sza,sea,rsea,aar;
float elev,azi;
float rgmls,rgmas,rsal,roc,rlati;

void setup()
{

    Serial.begin(9600);

    // sync clock before doing anything else
    setSyncProvider(requestSync);
    Serial.println("Waiting for sync message");
    while(timeStatus()==timeNotSet)
    {

        if(Serial.available())
            processSyncMessage();
        if(timeStatus() != timeNotSet)
        {

            digitalWrite(13,timeStatus() == timeSet); // on if synced, off if needs refresh
            digitalClockDisplay();

        }

        delay(1000);

    }

}

void loop()
{

    // turn arduino time into a /24 scaled time, check for night and then find angles, then track
    tday=hour()/24.0+minute()/1440.0+second()/86400.0;    // may have to evaluate time at one instant - see Arduino Time online
    rlati=lati*M_PI/180;
    // now returns current unix time
    julday = now()/86400.0 + dater + 2415018.5 - datli/24;// from unix time to excel/access time, 1900 date system
    //julday = dater + day() + 2415018.5 + tday - datli/24;// have to fix how day is entered
    julcent = (julday-2451545)/36525;

    gmls = fmod(280.46646+julcent*(36000.76983 + julcent*0.0003032),360); // returns remainder
    rgmls=gmls*M_PI/180; s
    gmas = 357.52911+julcent*(35999.05029 - 0.0001537*julcent);
    rgmas = gmas*M_PI/180; // gmas in rads
    eeo = 0.016708634-julcent*(0.000042037+0.0001537*julcent);
    seoc = sin(rgmas)*(1.914602-julcent*(0.004817+0.000014*julcent))+sin(2*rgmas)*(0.019993-0.000101*julcent)+sin(3*rgmas)*0.000289;
    stl = gmls + seoc;
    sta = gmas + seoc;
    srv = (1.000001018*(1-eeo*eeo))/(1+eeo*cos(sta*M_PI/180));
    sal = stl-0.00569-0.00478*sin(M_PI*(125.04-1934.136*julcent)/180);
    rsal = sal*M_PI/180;
    moe = 23+(26+((21.448-julcent*(46.815+julcent*(0.00059-julcent*0.001813))))/60)/60;
    oc = moe+0.00256*cos(M_PI*(125.04-1934.136*julcent)/180);
    roc=oc*M_PI/180;
    sra = 180*(atan2(cos(roc)*sin(rsal),cos(rsal)))/M_PI;
    sd = asin(sin(roc)*sin(rsal)); // I used this in rads
    vy = tan(roc/2)*tan(roc/2);
    eot = 4*180*(vy*sin(2*rgmls)-2*eeo*sin(rgmas)+4*eeo*vy*sin(rgmas)*cos(2*rgmls)-0.5*vy*vy*sin(4*rgmls)-1.25*eeo*eeo*sin(2*rgmas))/M_PI;
    has = 180*(acos(cos(M_PI*90.833/180)/cos(rlati)*cos(sd)-tan(rlati)*tan(sd)))/M_PI;
    sn =(720-4*longi-eot+datli*60)/1440;    // noon
    srt = sn-has*4/1440;                                    // sun rise time
    sst = sn+has*4/1440;                                    // sunset time

    sdur = 8*has; // minutes
    tst = fmod(tday*1440+eot+4*longi-60*datli,1440); //minutes

    if (tst/4 < 0) ha = tst/4+180;
    else ha = tst/4 - 180;
    sza = acos(sin(rlati)*sin(sd)+cos(rlati)*cos(sd)*cos(M_PI*ha/180));
    sea = 90-sza*180/M_PI;
    rsea = sea*M_PI/180;

    // correct elevation angle for atmospheric refraction, approximate
    if (sea>85) aar=0;
    else if (sea>5) aar = (58.1/tan(rsea)-0.07/pow(tan(rsea),3)+0.000086/pow(tan(rsea),5))/3600;
    else if (sea>-0.575)    aar = (1735+sea*(-518.2+sea*(103.4+sea*(-12.79+sea*0.711))))/3600;
    else aar = -20.772/tan(rsea)/3600;

    elev = sea + aar;

    if (ha>0) azi = fmod(180*(acos(((sin(rlati)*cos(sza))-sin(sd))/(cos(rlati)*sin(sza))))/M_PI+180,360);
    else azi = fmod(540-180*(acos(((sin(rlati)*cos(sza))-sin(sd))/(cos(rlati)*sin(sza))))/M_PI,360);

    // serial text to say what's working
    Serial.print(" looping, is the time OK?");
    Serial.print("\n");
    Serial.print(" ");
    digitalClockDisplay();
    Serial.print("\t");
    Serial.print("AZI: ");
    Serial.print(azi);
    Serial.print("\t");
    Serial.print("ELEV: ");
    Serial.print(elev);
    Serial.print("\t");
    Serial.print("time: ");
    Serial.print(tday);
    Serial.print("\t");
    Serial.print("\n");
    Serial.print(" ");// carriage return

    // compare time with sun rise and sunset and only track if sun is up!
    if(tday > srt + 0.2/24 && tday < sst - 0.2/24) // solar daytime
    {

        // track
        // slope and intercept were for voltage to angle, so invert
        // redo with calibration and azi convention (also, had to change variable names
        sunazi_v =(azi - 19.11)/(63.343);
        sunelev_v =(elev - 213.69)/(-61.64);

        // read voltages before deciding whether to jump back into loop
        trackazi_v = analogRead(azipin);
        trackazi_v = trackazi_v*5/1023;
        trackelev_v = analogRead(elevpin);
        trackelev_v = trackelev_v*5/1023;

        // keep from looping to get to out of range tilts / azi
        if (sunelev_v > velev_max - tol/2) sunelev_v = velev_max;
        if (sunelev_v < velev_min + tol/2) sunelev_v = velev_min;
        if (sunazi_v > vazi_max - tol/2)   sunazi_v  = vazi_max;
        if (sunazi_v < vazi_min + tol/2)   sunazi_v  = vazi_min;

        // print labels for troubleshooting, even if not tracking this iteration
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
        Serial.println(""); // go back to left

        //compare angles and align tracker to within tol
        while(abs(sunazi_v-trackazi_v)>tol || abs(sunelev_v-trackelev_v)>tol)
        {

            //read angle via input lines voltages
            trackazi_v = analogRead(azipin);
            trackazi_v = trackazi_v*5/1023;
            trackelev_v = analogRead(elevpin);
            trackelev_v = trackelev_v*5/1023;

            // print labels for troubleshooting
            //Serial.print("Tracking Goto:");
            //Serial.print("\t");
            //Serial.print(sunazi_v);
            //Serial.print("\t");
            //Serial.print(sunelev_v);
            //Serial.print("\t");
            //Serial.print("Current:");
            //Serial.print("\t");
            //Serial.print(trackazi_v);
            //Serial.print("\t");
            //Serial.print(trackelev_v);
            //Serial.print("\n");
            //Serial.println(""); // go back to left

            if(sunazi_v > trackazi_v+tol)
            {

                 // turn AZI motor forward, 1 sec
                 analogWrite(PwmPinMotorA,255);
                 digitalWrite(DirectionPinMotorA, HIGH);
                 delay(motdel);
                 //time += motdel;
                 analogWrite(PwmPinMotorA,0);

            }

            if(sunazi_v < trackazi_v-tol)
            {

                 // turn AZI motor reverse, 1 sec
                 analogWrite(PwmPinMotorA,255);
                 digitalWrite(DirectionPinMotorA, LOW);
                 delay(motdel);
                 //time += motdel;
                 analogWrite(PwmPinMotorA,0);

            }

            if((sunelev_v > trackelev_v+tol)&&(sunelev_v < 3.2))
            {

                 // turn ELEV motor forward, 1 sec
                 analogWrite(PwmPinMotorB,255);
                 digitalWrite(DirectionPinMotorB, HIGH);
                 delay(motdel);
                 //time += motdel;
                 analogWrite(PwmPinMotorB,0);

            }

            if((sunelev_v < trackelev_v-tol)&&(sunelev_v > 2.00))
            {

                 // turn ELEV motor reverse, 1 sec
                 analogWrite(PwmPinMotorB,255);
                 digitalWrite(DirectionPinMotorB, LOW);
                 delay(motdel);
                 //time += motdel;
                 analogWrite(PwmPinMotorB,0);

            }

        }

        // track every ten minutes for now
        for(int j=0;j<5;j++)
        {

            tv = analogRead(tvpin);
            fv = analogRead(fvpin);
            tv = tv*5/1023;
            fv = fv*5/1023;// then scale for voltage divider
            digitalClockDisplay();
            Serial.print("Track V:");
            Serial.print("\t");
            Serial.print(tv);
            Serial.print("\t");
            Serial.print("Fixed V:");
            Serial.print("\t");
            Serial.print(fv);
            Serial.print("\n");
            Serial.println(""); // go back to left
            delay(120000); // measure every 2 minutes for now

        }

    }
    else if(tday > srt - 0.2/24 || tday < sst + 0.2/24) delay(600000);
    else delay(3600000);// wait 1 hour during night time

}

void digitalClockDisplay()
{

    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(month());
    Serial.print(" ");
    Serial.print(year());
    //Serial.println();

}

void printDigits(int digits)
{

    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);

}

void processSyncMessage()
{

    // if time sync available from serial port, update time and return true
    while(Serial.available() >= TIME_MSG_LEN ) // time message consists of a header and ten ascii digits
    {

        char c = Serial.read();
        Serial.print(c);
        if( c == TIME_HEADER )
        {

            time_t pctime = 0;
            for(int i=0; i < TIME_MSG_LEN -1; i++)
            {

                c = Serial.read();
                if( c >= '0' && c <= '9')
                    pctime = (10 * pctime) + (c - '0') ; // convert digits to a number

            }
            setTime(pctime);     // Sync Arduino clock to the time received on the serial port

        }

    }

}

time_t requestSync()
{

    Serial.print(TIME_REQUEST,BYTE);
    return 0; // the time will be sent later in response to serial mesg

}
