#!/usr/bin/python

import time
from datetime import datetime
import serial

ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
	#datestring = datetime.now().strftime( '^%y %j %H %M %S' ) + datetime.now().strftime('%f')[:1] + '$'
	datestring = '^' + str(time.time()) + '$'
	ser.write(datestring)
	print time.time()
	time.sleep(.1)
