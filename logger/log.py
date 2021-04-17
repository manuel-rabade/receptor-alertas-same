#!/usr/bin/python

import serial
import time
import csv
import sys
import os

port = serial.Serial(sys.argv[1])
port.flushInput()

while True:
    line = port.readline()
    txt = line.decode('ascii').rstrip()
    timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
    print(timestamp + ' -> ' + txt)
    filename = time.strftime('%Y-%m-%d.log')
    with open(os.path.join(sys.argv[2], filename),"a") as f:
        writer = csv.writer(f,delimiter=",")
        writer.writerow([time.strftime('%Y-%m-%d'), time.strftime('%H:%M:%S'), time.time(), txt])
