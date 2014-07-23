#!/bin/bash

cd ~/Rlags_project/scripts/gps
startup=$(date +%d.%H.%M.%S)

./streamInGPS.sh & #begin transferring from ttyUSB0 to buffer file

while true
do
	start=$(date +%s.%N)

	cp gpsStream.txt /media/ssd_0/gps/stream.$startup.txt
	cp gpsStream.txt /media/ssd_1/gps/stream.$startup.txt
	echo "GPS: archived GPS data stream log. "$(date)

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start | bc) #every 10 seconds
done
