#!/bin/bash

cd ~/Rlags_project/scripts/gps
startup=$(date +%s.%N)

while true
do
	start=$(date +%s.%N)

	#tail -50 gpsStream.txt | grep -v '^$' | tail -15 >> ~/tempGPS.txt
	#cp gpsStream.txt gpsStream.temp.txt
	#tail -50 gpsStream.temp.txt >> ~/tempGPS.txt
	#mv ~/tempGPS.txt ~/gpsExp.txt

	tail -50 gpsStream.txt | grep -v '^$' | tail -20 > ~/latestData/gpsData.txt

	cp gpsStream.txt /media/ssd_0/gps/stream.$startup.txt
	cp gpsStream.txt /media/ssd_1/gps/stream.$startup.txt
	echo "GPS: archived GPS data stream log. "$(date)

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start - 0.009 | bc) #every 10 seconds
done
