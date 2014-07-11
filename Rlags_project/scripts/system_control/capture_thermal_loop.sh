#!/bin/bash

cd ~/Rlags_project/scripts/communication
begin=$(date +%d.%H.%M.%S)

while true
do
	start=$(date +%s.%N)

	echo "Thermal: Injected timestamp, archived."
	echo "{{ "$(date)" }}" >> serial_output
	cp serial_output /media/ssd_0/thermal_data/stream.$begin.txt
	cp serial_output /media/ssd_1/thermal_data/stream.$begin.txt

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start | bc) #every 10 seconds
done
