/***************************************************************
 *                                                             *
 *  EM-406 GPS Logger w/Atmega328                              *
 *                                                             *
 *  GPS RX Pin             to Atmega Pin 3  (D1) (TX)          *
 *  GPS TX Pin             to Atmega Pin 2  (D0) (RX)          *
 *                                                             *
 *  SD Card CS Pin         to Atmega Pin 6  (D4)               *
 *  SD Card DI Pin         to Atmega Pin 17 (D11) (MOSI)       *
 *  SD Card DO Pin         to Atmega Pin 18 (D12) (MISO)       *
 *  SD Card CLK Pin        to Atmega Pin 19 (D13) (SCK)        *
 *                                                             *
 *  LCD Pin 1 (Ground)     to GND                              *
 *  LCD Pin 2 (Power)      to +5V                              *
 *  LCD Pin 3 (Contrast)   to 10kohm potentiometer wiper       *
 *  LCD Pin 4 (RS)         to Atmega Pin 16 (D10)              *
 *  LCD Pin 5 (RW)         to GND                              *
 *  LCD Pin 6 (Enable)     to Atmega Pin 15 (D9)               *
 *  LCD Pin 11 (D4)        to Atmega Pin 14 (D8)               *
 *  LCD Pin 12 (D5)        to Atmega Pin 13 (D7)               *
 *  LCD Pin 13 (D6)        to Atmega Pin 12 (D6)               *
 *  LCD Pin 14 (D7)        to Atmega Pin 11 (D5)               *
 *  LCD Pin 15 (Backlight) to +5V                              *
 *  LCD Pin 16 (Ground)    to GND                              *
 *                                                             *
 *  Ends of 10kohm potentiometer to +5V and GND                *
 *                                                             *
 ***************************************************************/

// POTENTIOMETER TO SET VERBOSITY

#define TIMEZONE   -4
#define GPSBAUD     4800
#define CHIPSELECT  4

#include <LiquidCrystal.h>
#include <NewSoftSerial.h>
#include <TinyGPS.h>
#include <SD.h>
#include <Time.h>

LiquidCrystal lcd(10, 9, 8, 7, 6, 5);

TinyGPS gps;
NewSoftSerial nss(0, 1);
File logfile;
bool SDvalid = false;
bool link = false;
bool feedgps();
time_t gpsTimeSync();
time_t gpsTimeToArduinoTime();
String formatDigits(int digits, String append);

void setup() {

    pinMode(2, OUTPUT);
    pinMode(3, INPUT);

    setSyncProvider(gpsTimeSync);
    //setSyncInterval(1);

    lcd.begin(20,4);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("GPS Receiver...");
    nss.begin(GPSBAUD);
    lcd.setCursor(18,0);
    delay(1000);
    if (nss.available()) lcd.print("OK");
    else lcd.print("XX");

    lcd.setCursor(0,1);
    lcd.print("Data Logging...");
    lcd.setCursor(18,1);

    if (SD.begin(CHIPSELECT))
    {

        logfile = SD.open("log.csv", FILE_WRITE);
        if (logfile)
        {

            SDvalid = true;
            logfile.println("==============INITIALIZED==============");
            logfile.println("date,time,latitude,longitude,elevation");

        }

    }

    if (SDvalid) lcd.print("OK");
    else lcd.print("XX");

    lcd.setCursor(0,2);
    lcd.print("Satellite Link...");
    lcd.setCursor(18,2);

}

void loop()
{

    do
    {

        while (nss.available())
        {

            if (gps.encode(nss.read()))
            {

                if (gps.fixtype() > TinyGPS::GPS_FIX_NO_FIX)
                {

                    if (!link)
                    {

                        link = true;
                        lcd.print("OK");
                        delay(1000);
                        lcd.clear();

                    }

                    long lat, lon;
                    short latD, lonD;
                    unsigned short altW, altF;
                    unsigned int latMS, lonMS;
                    unsigned long age, altitude;
                    String date, time, slat, slon, salt;
                    String data = "";

                    time_t t = gpsTimeSync();

                    date = "";
                    date.concat(formatDigits(month(t),"/"));
                    date.concat(formatDigits(day(t),"/"));
                    date.concat(year(t));

                    time = "";
                    time.concat(formatDigits(hour(t),":"));
                    time.concat(formatDigits(minute(t),":"));
                    time.concat(formatDigits(second(t),""));

                    lcd.setCursor(0,0);
                    lcd.print(date);
                    data += date;
                    data += ",";

                    lcd.setCursor(11,0);
                    lcd.print(time);
                    data += time;
                    data += ",";

                    gps.get_position(&lat, &lon, &age);

                    latD = lat / 100000;
                    lonD = lon / 100000;
                    latMS = (latD < 0) ? (0-lat) - (0-latD)*100000 : lat - latD*100000;
                    lonMS = (lonD < 0) ? (0-lon) - (0-lonD)*100000 : lon - lonD*100000;
                    slat = latD; slat += "."; slat += latMS;
                    slon = lonD; slon += "."; slon += lonMS;

                    lcd.setCursor(0,1);
                    lcd.print("lat: ");
                    lcd.setCursor(5,1);
                    if (latD > 0) lcd.print(" ");
                    lcd.print(slat);
                    data += slat;
                    data += ",";
                    lcd.print((latD > 0) ? "N" : "S");

                    for (int i=15; i<18 ; i++)
                    {

                        lcd.setCursor(i,1);
                        lcd.print(" ");

                    }

                    lcd.setCursor(0,2);
                    lcd.print("lon: ");
                    lcd.setCursor(5,2);
                    if (lonD > 0) lcd.print(" ");
                    lcd.print(slon);
                    data += slon;
                    data += ",";
                    lcd.print((lonD > 0) ? "E" : "W");

                    for (int i=15; i<18 ; i++)
                    {

                        lcd.setCursor(i,2);
                        lcd.print(" ");

                    }

                    altitude = gps.altitude();
                    altW = altitude / 100;
                    altF = (altitude % 100) / 10;
                    salt = (String)altW;
                    salt += ".";
                    salt += (String)altF;
                    lcd.setCursor(0,3);
                    lcd.print("elev: ");
                    lcd.setCursor(6,3);
                    lcd.print(salt);
                    data += salt;
                    lcd.print("m");

                  /*unsigned short gps_heading = (unsigned short) gps.course();
                    lcd.setCursor(18,3);
                    if (gps_heading >= 0 && gps_heading <= 36000)
                    {
                                 if (gps_heading >=         0 && gps_heading <    2250) lcd.print("N ");
                        else if (gps_heading >=    2250 && gps_heading <    6750) lcd.print("NE");
                        else if (gps_heading >=    6750 && gps_heading < 11250) lcd.print(" E");
                        else if (gps_heading >= 11250 && gps_heading < 15750) lcd.print("SE");
                        else if (gps_heading >= 15750 && gps_heading < 20250) lcd.print("S ");
                        else if (gps_heading >= 20250 && gps_heading < 24750) lcd.print("SW");
                        else if (gps_heading >= 24750 && gps_heading < 29250) lcd.print(" W");
                        else if (gps_heading >= 29250 && gps_heading < 33750) lcd.print("NW");
                        else if (gps_heading >= 33750 && gps_heading <=36000) lcd.print("N ");
                        else lcd.print("--");
                    } else lcd.print("--");*/

                    for (int i=7+salt.length(); i<18 ; i++)
                    {

                        lcd.setCursor(i,3);
                        lcd.print(" ");

                    }

                    lcd.setCursor(18,1);
                    lcd.print((short)gps.satsinview());
                    data.concat(",");
                    data.concat((short)gps.satsinview());
                    lcd.setCursor(18,2);
                    lcd.print((short)gps.satsused());
                    data.concat(",");
                    data.concat((short)gps.satsused());
                    lcd.setCursor(18,3);
                    if (gps.fixtype() == TinyGPS::GPS_FIX_2D) lcd.print("2D");
                    else if (gps.fixtype() == TinyGPS::GPS_FIX_3D) lcd.print("3D");
                    else lcd.print("--");

                    if (SDvalid) logfile.println(data);

                }
                else
                {

                  /*lcd.setCursor(0,0);
                    for (int i=0 ; i<20 ; i++) lcd.print(" ");
                    lcd.setCursor(0,1);
                    lcd.print("     Connected To ");
                    lcd.print((short)gps.satsinview());
                    for (int i=16 ; i<20 ; i++) lcd.print(" ");
                    lcd.setCursor(0,2);
                    lcd.print("     GPS Satellites     ");
                    lcd.setCursor(0,3);
                    for (int i=0 ; i<20 ; i++) lcd.print(" ");*/

                }

            }

        }

    } while (digitalRead(3)==LOW);

    if (SDvalid)
    {

        logfile.println("===============FINALIZED===============");
        logfile.close();

    }

    delay(250);
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Logging Complete");
    lcd.setCursor(1,2);
    lcd.print("Remove SD Card Now");

    while (true)
    {

        digitalWrite(2, HIGH);
        delay(1000);
        digitalWrite(2, LOW);
        delay(1000);

    }

}

bool feedgps()
{

    while (nss.available())
        return (gps.encode(nss.read())) ? true : false;

}

time_t gpsTimeSync()
{

    unsigned long age = 0;
    gps.get_datetime(NULL, NULL, &age);
    return gpsTimeToArduinoTime();

}

time_t gpsTimeToArduinoTime()
{

    tmElements_t tm;
    unsigned long date, time;
    gps.get_datetime(&date,&time,NULL);
    tm.Year = 30 + (date % 100);
    tm.Month = (date / 100) % 100;
    tm.Day = date / 10000;
    tm.Hour = time / 1000000;
    tm.Minute = (time / 10000) % 100;
    tm.Second = (time / 100) % 100;
    time_t timestamp = makeTime(tm);
    return timestamp + (TIMEZONE * 3600);

}

String formatDigits(int digits, String append)
{

    String string = "";
    if (digits < 10) string.concat("0");
    string.concat(digits);
    string.concat(append);
    return string;

}
