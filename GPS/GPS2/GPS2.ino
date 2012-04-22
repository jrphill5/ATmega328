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

// FIX DATE HANDLING WITH TIMEZONE 

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

void setup()
{

    pinMode(2, OUTPUT);
    pinMode(3, INPUT);

    setSyncProvider(gpsTimeSync);

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

        logfile = SD.open("log.txt", FILE_WRITE);
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

                if (!link)
                {

                    link = true;
                    lcd.print("OK");
                    delay(1000);
                    lcd.clear();

                }

                long lat, lon, altitude;
                short latD, lonD;
                unsigned short altW, altF;
                unsigned int latMS, lonMS;
                unsigned long date, time, age;
                String sdate, stime, slat, slon, salt;
                String data = "";

              /*year = 2000 + (date % 100);
                month = (date / 100) % 100;
                day = date / 10000;*/

                sdate = "";
                if (month() < 10) sdate += "0";
                sdate += (String)month();
                sdate += "/";
                if (day() < 10) sdate += "0";
                sdate += (String)day();
                sdate += "/";
                sdate += (String)year();

              /*hour = time / 1000000;
                minute = (time / 10000) % 100;
                second = (time / 100) % 100;*/

                stime = "";
                if (hour() < 10) stime += "0";
                stime += (String)hour();
                stime += ":";
                if (minute() < 10) stime += "0";
                stime += (String)minute();
                stime += ":";
                if (second() < 10) stime += "0";
                stime += (String)second();

                lcd.setCursor(0,0);
                lcd.print(sdate);
                data += sdate;
                data += ",";

                lcd.setCursor(11,0);
                lcd.print(stime);
                data += stime;
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

                lcd.setCursor(0,2);
                lcd.print("lon: ");
                lcd.setCursor(5,2);
                if (lonD > 0) lcd.print(" ");
                lcd.print(slon);
                data += slon;
                data += ",";
                lcd.print((lonD > 0) ? "E" : "W");

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
                
                for (int i=7+salt.length(); i<20 ; i++)
                {

                    lcd.setCursor(i,3);
                    lcd.print(" ");

                }

                //feedgps();
                if (SDvalid) logfile.println(data);

            }

        }

    } while (digitalRead(3)==LOW);

    if (SDvalid)
    {

        logfile.println("===============FINALIZED===============");
        logfile.close();

    }

    delay(250);

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
        if (gps.encode(nss.read()))
            return true;
        else return false;

}

time_t gpsTimeSync()
{

    unsigned long fix_age = 0 ;
    gps.get_datetime(NULL,NULL, &fix_age);
    unsigned long time_since_last_fix;
    if(fix_age < 1000)
        return gpsTimeToArduinoTime();
    else return 0;

}

time_t gpsTimeToArduinoTime()
{

    tmElements_t tm;
    int year;
    gps.crack_datetime(&year, &tm.Month, &tm.Day, &tm.Hour, &tm.Minute, &tm.Second, NULL, NULL);
    tm.Year = year - 1970;
    time_t time = makeTime(tm);
    return time + (TIMEZONE * 3600);

}
