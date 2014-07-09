#!/bin/bash

gpspipe -r > gpsData.txt & tail -f gpsData.txt | grep --line-buffered -E "GPRMC" | python parse.py

while true
do
	start=$(date +%s.%N)

	#TODO: grab last line from gpsData.txt, get it ready to send to IMU

	end=$(date +%s.%N)
	sleep $(echo 60 - $end + $start | bc) #once a minute
done
