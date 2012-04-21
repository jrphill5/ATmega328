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

#define TIMEZONE   -4
#define GPSBAUD     4800
#define CHIPSELECT  4

#include <LiquidCrystal.h>
#include <NewSoftSerial.h>
#include <TinyGPS.h>
#include <SD.h>

LiquidCrystal lcd(10, 9, 8, 7, 6, 5);

TinyGPS gps;
NewSoftSerial nss_gps(0, 1);
File logfile;
bool SDvalid;
String dataString = "";

void printCoordinates();
void printDateTime();
void printAltitude();
void printSpeed();
void printHeading();

void setup()
{

    //Serial.begin(GPSBAUD);
    //Serial.print("Initializing Serial Communications... ");
    //Serial.println("OK");

    lcd.begin(20,4);

    //Serial.print("Initializing GPS Receiver... ");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("GPS Receiver...");
    nss_gps.begin(GPSBAUD);
    //Serial.println("OK");
    lcd.setCursor(18,0);
    lcd.print("OK");

    //Serial.print("Initializing SD SPI Communications... ");
    lcd.setCursor(0,1);
    lcd.print("Secure Digital...");
    lcd.setCursor(18,1);
    if (SD.begin(CHIPSELECT))
    {
        //Serial.println("OK");
        lcd.print("OK");
        SDvalid = true;
    }
    else
    {
        //Serial.println("ERROR");
        lcd.print("XX");
        SDvalid = false;
    }

    //Serial.print("Opening Log File... ");
    lcd.setCursor(0,2);
    lcd.print("Data Logging...");
    lcd.setCursor(18,2);
    if (SDvalid)
    {
        logfile = SD.open("log.txt", FILE_WRITE);
        if (logfile) 
        {
            //Serial.println("OK");
            lcd.print("OK");
            logfile.println("Logging Initialized.");
        }
        else
        {
            //Serial.println("ERROR");
            lcd.print("XX");
            SDvalid = false;
        }
    }
    else
    {
        //Serial.println("ERROR");
        lcd.print("XX");
    }

    //Serial.println("Establishing Satellite Link... ");
    lcd.setCursor(0,3);
    lcd.print("Satellite Link...");

    delay(1000);

}

void loop()
{

    while(nss_gps.available())
    {
        if(gps.encode(nss_gps.read()))
        {
            lcd.setCursor(0,0);
            printCoordinates();
            lcd.setCursor(0,1);
            printDateTime();
            lcd.setCursor(0,2);
            printAltitude();
            printSpeed();
            lcd.setCursor(0,3);
            printHeading();
            if(SDvalid) logfile.println(dataString);
            //Serial.println(dataString);
            dataString = "";
            break;
        }
    }

}

void printCoordinates()
{

    float flatitude, flongitude;
    long latitude, longitude;
    String latDir, lonDir;

    gps.f_get_position(&flatitude, &flongitude);
    gps.get_position(&latitude, &longitude);

    if (flatitude > 0)
    {
        lcd.print(flatitude,5);
        lcd.print("N");
        latDir = "N";        
    }
    else 
    {
        latitude = 0-latitude;
        lcd.print(0-flatitude,5);
        lcd.print("S");
        latDir = "S";
    }

    if (abs(flatitude) < 10) lcd.print(" ");
    lcd.print(" ");

    if (flongitude > 0)
    {
        lcd.print(flongitude,5);
        lcd.print("E");
        lonDir = "E";
    }
    else
    {
        longitude = 0-longitude;
        lcd.print(0-flongitude,5);
        lcd.print("W");
        lonDir = "W";
    }

    if (abs(flongitude) < 100) lcd.print(" ");
    if (abs(flongitude) < 10) lcd.print(" ");

    dataString += (String)latitude;
    dataString += ",";
    dataString += latDir;
    dataString += ",";

    dataString += (String)longitude;
    dataString += ",";
    dataString += lonDir;
    dataString += ",";

}

void printDateTime()
{

    int year;
    byte month, day, hour, minute, second, hundredths;
    unsigned short smonth, sday, sminute, ssecond;
    signed short shour;
    String date = "";
    String time = "";

    gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);

    smonth  = (unsigned short)month;
    sday    = (unsigned short)day;
    shour   = (signed short)hour;
    sminute = (unsigned short)minute;
    ssecond = (unsigned short)second;

    shour += TIMEZONE;
    if (shour < 0) shour += 24;
    if (shour >= 24) shour -=24;

    if (shour < 10)
    {
        lcd.print("0");
        time += "0";
    }
    lcd.print(shour, DEC);
    time += (String)shour;
    lcd.print(":");
    time += ":";
    if (minute < 10)
    {
        lcd.print("0");
        time += "0";
    }
    lcd.print(minute, DEC);
    time += (String)sminute;
    lcd.print(":");
    time += ":";
    if (second < 10)
    {
        lcd.print("0");
        time += "0";
    }
    lcd.print(second, DEC);
    time += ssecond;

    lcd.print("    ");

    if (month < 10)
    {
        lcd.print("0");
        date += "0";
    }
    lcd.print(month, DEC);
    date += (String)smonth;
    lcd.print("/");
    date += "/";
    if (day < 10)
    {
        lcd.print("0");
        date += "0";
    }
    lcd.print(day, DEC);
    date += (String)sday;
    lcd.print("/");
    date += "/";
    lcd.print(year, DEC);
    date += (String)year;

    dataString += date;
    dataString += ",";

    dataString += time;
    dataString += ",";

    dataString += (String)TIMEZONE;
    //dataString += ",";

}

void printAltitude()
{

    float gps_altitude = gps.f_altitude();

    if (gps_altitude < 10) {
        lcd.print(gps_altitude,1);
        lcd.print("m            ");
    } else if (gps_altitude < 100) {
        lcd.print(gps_altitude,1);
        lcd.print("m         ");
    } else if (gps_altitude < 1000) {
        lcd.print(gps_altitude,1);
        lcd.print("m        ");
    } else if (gps_altitude < 10000) {
        lcd.print(gps_altitude,1);
        lcd.print("m     ");
    } else if (gps_altitude < 100000) {
        lcd.print(gps_altitude,1);
        lcd.print("m    ");
    } else if (gps_altitude < 1000000) {
        lcd.print(gps_altitude,1);
        lcd.print("m ");
    } else if (gps_altitude < 10000000) {
        lcd.print(gps_altitude,0);
        lcd.print("m ");
    } else {
        lcd.print("ERROR         ");
    }

}

void printSpeed()
{

    float gps_speed = gps.f_speed_mph();

    if (gps_speed < 10) {
        lcd.print(gps_speed,2);
        lcd.print("mph     ");
    } else if (gps_speed < 100) {
        lcd.print(gps_speed,2);
        lcd.print("mph    ");
    } else if (gps_speed < 1000) {
        lcd.print(gps_speed,2);
        lcd.print("mph ");
    } else if (gps_speed < 10000) {
        lcd.print(gps_speed,2);
        lcd.print("mph");
    } else if (gps_speed < 100000) {
        lcd.print(gps_speed,1);
        lcd.print("mph");
    } else if (gps_speed < 1000000) {
        lcd.print(gps_speed,0);
        lcd.print("mph");
    } else {
        lcd.print("ERROR         ");
    }

}

void printHeading()
{

    float gps_heading = gps.f_course();

    lcd.print("Heading:    ");

    if (gps_heading >= 0 && gps_heading <= 360)
    {
        lcd.print(gps_heading);
        if (gps_heading < 100) lcd.print(" ");
        if (gps_heading < 10) lcd.print(" ");
        /*   if (gps_heading >=   0.00 && gps_heading <  11.25) lcd.print("  N ");
        else if (gps_heading >=  11.25 && gps_heading <  33.75) lcd.print(" NNE");
        else if (gps_heading >=  33.75 && gps_heading <  56.25) lcd.print("  NE");
        else if (gps_heading >=  56.25 && gps_heading <  78.75) lcd.print(" ENE");
        else if (gps_heading >=  78.75 && gps_heading < 101.25) lcd.print("   E");
        else if (gps_heading >= 101.25 && gps_heading < 123.75) lcd.print(" ESE");
        else if (gps_heading >= 123.75 && gps_heading < 146.25) lcd.print("  SE");
        else if (gps_heading >= 146.25 && gps_heading < 168.75) lcd.print(" SSE");
        else if (gps_heading >= 168.75 && gps_heading < 191.25) lcd.print("  S ");
        else if (gps_heading >= 191.25 && gps_heading < 213.75) lcd.print(" SSW");
        else if (gps_heading >= 213.75 && gps_heading < 236.25) lcd.print("  SW");
        else if (gps_heading >= 236.25 && gps_heading < 258.75) lcd.print(" WSW");
        else if (gps_heading >= 258.75 && gps_heading < 281.25) lcd.print("   W");
        else if (gps_heading >= 281.25 && gps_heading < 303.75) lcd.print(" WNW");
        else if (gps_heading >= 303.75 && gps_heading < 326.25) lcd.print("  NW");
        else if (gps_heading >= 326.25 && gps_heading < 348.75) lcd.print(" NNW");
        else if (gps_heading >= 348.75 && gps_heading <=360.00) lcd.print("  N ");
        else lcd.print(" -- ");*/
        lcd.print("        ");
    }
    else lcd.print("ERROR         ");

}
