#!/bin/bash

cd ~/Rlags_project/scripts/communication
begin=$(date +%d.%H.%M.%S)

while true
do
	start=$(date +%s.%N)

	odroidTemp=$(cat /sys/devices/virtual/thermal/thermal_zone0/temp | sed 's/000//g')
	odroidData=$(echo $odroidTemp" : "$(date))
	echo $odroidData >> /media/ssd_0/thermal_data/odroidTemp.$begin.txt
	echo $odroidData >> /media/ssd_1/thermal_data/odroidTemp.$begin.txt

	echo "Thermal: Odroid at "$odroidTemp"C, archived, timestamped Dua stream"
	echo "{{ "$(date)" }}" >> serial_output
	cp serial_output /media/ssd_0/thermal_data/stream.$begin.txt
	cp serial_output /media/ssd_1/thermal_data/stream.$begin.txt

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start | bc) #every 10 seconds
done
