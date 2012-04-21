const float lati = 36.2142983; // broyhill
const float longi = -81.693161;
const float datli = -6;

float dat = 40382;  // july 23 2010
float tday = 7.0/24; // start at 7am
float julday,julcent,gmls,gmas,eeo,seoc,stl,sta,srv,sal,moe,oc,sra;
float sd,vy,eot,has,sn,srt,sst,sdur,tst,ha;

// NOAA daily solar position calculator, based on lat, long, dateline, date, time

julday = dat+2415018.5+tday-datli/24;
julcent = (julday-2451545)/36525;
// need mod fcn
gmls = MOD(280.46646+julcent*(36000.76983 + julcent*0.0003032),360);
gmas = 357.52911+julcent*(35999.05029 - 0.0001537*julcent);
eeo = 0.016708634-julcent*(0.000042037+0.0001537*julcent);
seoc = SIN(RADIANS(gmas))*(1.914602-julcent*(0.004817+0.000014*julcent))+SIN(RADIANS(2*gmas))*(0.019993-0.000101*julcent)+SIN(RADIANS(3*gmas))*0.000289;
stl = gmls + seoc;
sta = gmas + seoc;
srv = (1.000001018*(1-eeo*eeo))/(1+eeo*COS(RADIANS(sta)));
sal = stl-0.00569-0.00478*SIN(RADIANS(125.04-1934.136*julcent));
moe = 23+(26+((21.448-julcent*(46.815+julcent*(0.00059-julcent*0.001813))))/60)/60;
oc = moe+0.00256*COS(RADIANS(125.04-1934.136*julcent));
sra = DEGREES(ATAN2(COS(RADIANS(oc))*SIN(RADIANS(sal)),COS(RADIANS(sal))));
sd = DEGREES(ASIN(SIN(RADIANS(oc))*SIN(RADIANS(sal))));
vy = TAN(RADIANS(oc/2))*TAN(RADIANS(oc/2));
eot = 4*DEGREES(vy*SIN(2*RADIANS(gmls))-2*eeo*SIN(RADIANS(gmas))+4*eeo*vy*SIN(RADIANS(gmas))*COS(2*RADIANS(gmls))-0.5*vy*vy*SIN(4*RADIANS(gmls))-1.25*eeo*eeo*SIN(2*RADIANS(gmas)));
has = DEGREES(ACOS(COS(RADIANS(90.833))/COS(RADIANS(lati))*COS(RADIANS(sd))-TAN(RADIANS(lati))*TAN(RADIANS(sd))));
sn = =(720-4*longi-eot+datli*60)/1440;
srt = sn-has*4/1440;
sst = sn+has*4/1440;
sdur = 8*has;
tst = MOD(tday*1440+eot+4*longi-60*datli,1440);
if (tst/4 < 0) ha = tst/4+180;
else ha = tst/4 - 180;
//ha = IF(tst/4<0,tst/4+180,tst/4-180); // deg
//sza = DEGREES(ACOS(SIN(RADIANS(lati))*SIN(RADIANS(sd))+COS(RADIANS(lati))*COS(RADIANS(sd))*COS(RADIANS(ha)))); // deg
//sea = 90-sza; // deg
//// logic!
//aar = IF(sea>85,0,IF(sea>5,58.1/TAN(RADIANS(sea))-0.07/POWER(TAN(RADIANS(sea)),3)+0.000086/POWER(TAN(RADIANS(sea)),5),IF(sea>-0.575,1735+sea*(-518.2+sea*(103.4+sea*(-12.79+sea*0.711))),-20.772/TAN(RADIANS(sea)))))/3600; // deg
//elev = sea + aar;
//// logic!!!
//azi = IF(ha>0,MOD(DEGREES(ACOS(((SIN(RADIANS(lati))*COS(RADIANS(sza)))-SIN(RADIANS(sd)))/(COS(RADIANS(lati))*SIN(RADIANS(sza)))))+180,360),MOD(540-DEGREES(ACOS(((SIN(RADIANS(lati))*COS(RADIANS(sza)))-SIN(RADIANS(sd)))/(COS(RADIANS(lati))*SIN(RADIANS(sza))))),360));

// use azi and elev to find the solar voltages, based on our 1-day field calibration
