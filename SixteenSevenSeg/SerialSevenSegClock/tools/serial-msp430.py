#!/usr/bin/python

from time import sleep
from datetime import datetime
import serial

ser = serial.Serial('/dev/ttyACM0', 9600)

while True:
	ser.write(datetime.now().strftime( '%j %H %M %S  ' ))
	sleep(.05)
