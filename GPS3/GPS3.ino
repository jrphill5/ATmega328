/***************************************************************
 *                                                             *
 *  EM-406 GPS Logger w/Atmega328                              *
 *                                                             *
 *  GPS RX Pin             to Atmega Pin 3  (D1) (TX)          *
 *  GPS TX Pin             to Atmega Pin 2  (D0) (RX)          *
 *                                                             *
 *  SD Card CS  Pin        to Atmega Pin 6  (D4)               *
 *  SD Card DI  Pin        to Atmega Pin 17 (D11) (MOSI)       *
 *  SD Card DO  Pin        to Atmega Pin 18 (D12) (MISO)       *
 *  SD Card CLK Pin        to Atmega Pin 19 (D13) (SCK)        *
 *                                                             *
 *  LCD Pin 1  (Ground)    to GND                              *
 *  LCD Pin 2  (Power)     to +5V                              *
 *  LCD Pin 3  (Contrast)  to 10kohm potentiometer wiper       *
 *  LCD Pin 4  (RS)        to Atmega Pin 16 (D10)              *
 *  LCD Pin 5  (RW)        to GND                              *
 *  LCD Pin 6  (Enable)    to Atmega Pin 15 (D9)               *
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

#define TIMEZONE  -4
#define GPSBAUD    4800
#define LEDSELECT  2
#define QUITPIN    3
#define SDSELECT   4
#define MOSIPIN    11
#define MISOPIN    12
#define SCKPIN     13

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
unsigned long start;
time_t gpsTimestamp();
time_t prevTimestamp = 0;
void lcd_write_buffer(char *buffer, int length);
void format_angle(bool type, long angle, char *buffer, int length, char *dirbuffer, int dirlength);
void format_altitude(unsigned long altitude, char *buffer, int length);
void format_speed(unsigned long speed, char *buffer, int length);
void format_course(unsigned long course, char *buffer, int length);
void format_error(char *buffer, int length, char *format);

void setup()
{

    pinMode(QUITPIN, INPUT);
    pinMode(SDSELECT, OUTPUT);
    digitalWrite(SDSELECT, HIGH);
    pinMode(MOSIPIN, OUTPUT);
    pinMode(MISOPIN, INPUT);
    pinMode(SCKPIN, OUTPUT);

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

    if (SD.begin(SDSELECT))
    {

        logfile = SD.open("log.txt", FILE_WRITE);
        if (logfile)
        {

            SDvalid = true;
            logfile.println("===================================INITIALIZED===================================");
            logfile.println("date,time,latitude,latdir,longitude,londir,fix,elevation,speed,course,used,total");

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

    while (digitalRead(QUITPIN) == LOW)
    {

        while (nss.available())
        {

            if (gps.encode(nss.read()))
            {

                char lcd_data[4][21];

                for (int i=0 ; i < 4 ; i++)
                    for (int j=0 ; j < 21 ; j++)
                        lcd_data[i][j] = ' ';

                if (!link)
                {

                    link = true;
                    lcd.print("OK");
                    delay(1000);
                    start = millis();
                    lcd.clear();

                }

                tmElements_t tm;
                time_t timestamp = gpsTimestamp();
                breakTime(timestamp,tm);

                char bdate[11], btime[9];
                if (timestamp) snprintf(btime,sizeof(btime),"%02d:%02d:%02d",tm.Hour,tm.Minute,tm.Second);
                else format_error(btime,sizeof(btime),"--:--:--");
                if (tm.Year > 1) snprintf(bdate,sizeof(bdate),"%02d/%02d/%02d",tm.Month,tm.Day,1970+tm.Year);
                else format_error(bdate,sizeof(bdate),"--/--/----");

                snprintf(lcd_data[0],sizeof(lcd_data[0]),"%-10s%-10s",btime,bdate);

                char blat[9], blon[10], blatdir[2], blondir[2];

                if (gps.fixtype() > TinyGPS::GPS_FIX_NO_FIX)
                {

                    long lat, lon;
                    unsigned long age;

                    gps.get_position(&lat, &lon, &age);
    
                    format_angle(0, lat, blat, sizeof(blat), blatdir, sizeof(blatdir));
                    format_angle(1, lon, blon, sizeof(blon), blondir, sizeof(blondir));

                }
                else
                {

                    format_error(blat,sizeof(blat),"--.-----");
                    format_error(blon,sizeof(blon),"---.-----");
                    format_error(blatdir,sizeof(blatdir),"-");
                    format_error(blondir,sizeof(blondir),"-");

                }

                char lcdlat[11], lcdlon[11];
                snprintf(lcdlat,sizeof(lcdlat),"%s%s",blat,blatdir);
                snprintf(lcdlon,sizeof(lcdlon),"%s%s",blon,blondir);
                snprintf(lcd_data[1],sizeof(lcd_data[1]),"%-10s%-10s",lcdlat,lcdlon);

                char bspeed[6], bcourse[6], baltitude[6];
                format_altitude(gps.altitude(), baltitude, sizeof(baltitude));
                format_speed(gps.speed(), bspeed, sizeof(bspeed));
                format_course(gps.course(), bcourse, sizeof(bcourse));
                snprintf(lcd_data[2],sizeof(lcd_data[2]),"%sm %sk %sd",baltitude,bspeed,bcourse);

              /*unsigned long chars;
                unsigned short sentences, failed;
                gps.stats(&chars,&sentences,&failed);
                snprintf(lcd_data[2],sizeof(lcd_data[2]),"%u,%d,%d",chars,sentences,failed);*/

                char bfix[3];
                switch (gps.fixtype())
                {

                    case TinyGPS::GPS_FIX_3D: snprintf(bfix,sizeof(bfix),"%s","3D"); break;
                    case TinyGPS::GPS_FIX_2D: snprintf(bfix,sizeof(bfix),"%s","2D"); break;
                    case TinyGPS::GPS_FIX_NO_FIX: snprintf(bfix,sizeof(bfix),"%s","NO"); break;

                }

                unsigned short satsinview = gps.satsinview();
                unsigned short satsused = gps.satsused();
                snprintf(lcd_data[3],sizeof(lcd_data[3]),"Satellites: %02d/%02d %-3s",satsused,satsinview,bfix);

                // Write to SD card only once per second.
                if ( (SDvalid) && (timestamp != prevTimestamp) )
                {

                    char data[80];
                    prevTimestamp = timestamp;
                    snprintf(data,sizeof(data),"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d",bdate,btime,blat,blatdir,blon,blondir,bfix,baltitude,bspeed,bcourse,satsused,satsinview);
                    logfile.println(data);

                }

                for (int i=0 ; i < 4 ; i++)
                {

                    lcd.setCursor(0,i);
                    lcd_write_buffer(lcd_data[i],sizeof(lcd_data[i]));

                }

            }

        }

    }

    if (SDvalid)
    {
    
        logfile.println("====================================FINALIZED====================================");
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

        //digitalWrite(2, HIGH);
        //digitalWrite(2, LOW);

    }

}

time_t gpsTimestamp()
{

    tmElements_t tm;
    unsigned long date, time;

    gps.get_datetime(&date,&time,NULL);

    if (date != TinyGPS::GPS_INVALID_DATE)
    {

        tm.Year = 30 + (date % 100);
        tm.Month = (date / 100) % 100;
        tm.Day = date / 10000;

    }
    else
    {
    
        tm.Year = 1;
        tm.Month = 1;
        tm.Day = 1;

    }    

    if (time != TinyGPS::GPS_INVALID_TIME)
    {
    
        tm.Hour = time / 1000000;
        tm.Minute = (time / 10000) % 100;
        tm.Second = (time / 100) % 100;

    }
    else return 0;

    time_t timestamp = makeTime(tm) + (TIMEZONE * 3600);
    return (timestamp < 1300000000) ? 0 : timestamp;

}

void lcd_write_buffer(char *buffer, int length)
{

    bool terminate = false;
    for (int i=0 ; i<length-1 ; i++)
    {

        if (buffer[i] == '\0') terminate = true;
        if (terminate) lcd.write(' ');
        else lcd.write(buffer[i]);

    }

}

void format_angle(bool type, long angle, char *buffer, int length, char *dirbuffer, int dirlength)
{

    if (angle != TinyGPS::GPS_INVALID_ANGLE)
    {

        unsigned short angleD = (angle > 0) ? angle / 100000 : (0-angle) / 100000;
        unsigned int angleMS = (angle > 0) ? angle - angleD*100000 : (0-angle) - angleD*100000;
        snprintf(dirbuffer,dirlength,"%s", ((angle) ? ((angle > 0) ? ((type) ? "E" : "N") : ((type) ? "W" : "S")) : " "));
        snprintf(buffer,length,"%d.%05u",angleD,angleMS);

    }
    else (type) ? format_error(buffer,length,"---.-----") : format_error(buffer,length,"--.-----");

}

void format_altitude(unsigned long altitude, char *buffer, int length)
{

    if ((gps.fixtype() == TinyGPS::GPS_FIX_3D) && (altitude != TinyGPS::GPS_INVALID_ALTITUDE))
    {

        unsigned short altW = altitude / 100;
        unsigned short altF = (altitude % 100) / 10;
        snprintf(buffer,length,"%d.%d",altW,altF);

    }
    else format_error(buffer,length,"-.-");

}

void format_speed(unsigned long speed, char *buffer, int length)
{

    if (speed != TinyGPS::GPS_INVALID_SPEED)
    {

        unsigned short speedW = speed / 100;
        unsigned short speedF = speed % 100;
        snprintf(buffer,length,"%d.%d",speedW,speedF);

    }
    else format_error(buffer,length,"-.-");

}

void format_course(unsigned long course, char *buffer, int length)
{

    if (course != TinyGPS::GPS_INVALID_ANGLE)
    {

        unsigned short courseW = course / 100;
        unsigned short courseF = course % 100;
        snprintf(buffer,length,"%d.%d",courseW,courseF);

    }
    else format_error(buffer,length,"-.-");

}

void format_error(char *buffer, int length, char *format)
{

    snprintf(buffer,length,"%s",format);

}
