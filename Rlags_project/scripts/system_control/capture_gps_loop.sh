#!/bin/bash

cd ~/Rlags_project/scripts/gps
startup=$(date +%s.%N)

./streamInGPS.sh & #begin transferring from ttyUSB0 to buffer file

while true
do
	start=$(date +%s.%N)

	tail -50 gpsStream.txt | grep -v '^$' | tail -15 > ~/latestData/gpsData.txt

	cp gpsStream.txt /media/ssd_0/gps/stream.$startup.txt
	cp gpsStream.txt /media/ssd_1/gps/stream.$startup.txt
	echo "GPS: archived GPS data stream log. "$(date)

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start - 0.009 | bc) #every 10 seconds
done
