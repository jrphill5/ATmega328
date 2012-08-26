#!/usr/bin/python

from time import sleep
from datetime import datetime
import serial

ser = serial.Serial('/dev/ttyUSB0', 9600)

while True:
	ser.write(datetime.now().strftime( '^%y %j %H %M %S%f$' ))
	sleep(.05)
