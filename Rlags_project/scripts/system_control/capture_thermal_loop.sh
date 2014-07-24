#!/bin/bash

cd ~/Rlags_project/scripts/communication
begin=$(date +%s.%N)

while true
do
	start=$(date +%s.%N)

	odroidTemp=$(cat /sys/devices/virtual/thermal/thermal_zone0/temp | sed 's/000//g')
	odroidData=$(echo $odroidTemp" : "$start)

	tail -20 serial_output > ~/latestData/thermal_sensors.txt
	echo $odroidData > ~/latestData/odroidTemperature.txt

	echo $odroidData >> /media/ssd_0/thermal_data/odroidTemp.$begin.txt
	echo $odroidData >> /media/ssd_1/thermal_data/odroidTemp.$begin.txt

	cp serial_output /media/ssd_0/thermal_data/stream.$begin.txt
	cp serial_output /media/ssd_1/thermal_data/stream.$begin.txt

	echo "Thermal: Odroid at "$odroidTemp"C, archived Arduino stream, "$start

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start - 0.009 | bc) #every 10 seconds
done
