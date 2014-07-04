#!/bin/bash

cd ~/control_scripts

while true
do
	start=$(date +%s.%N)
	echo "Comm: assembling data bundle, beginning transmission. "$(date)

	cd ~/control_scripts
	./assemble_housekeeping.sh

	cd ~/Rlags_project/scripts/communication
	./push.sh #transmits bundle.tar.bz2
	#this is temporary! simulates transmit
	sleep 30

	echo "Comm: transmission completed. "$(date)

	waitTime=$(cat transmission_rate) #number of minutes between the beginning of each transmission
	end=$(date +%s.%N)

	if [ $waitTime -gt 0 ] #if it's > 0
	then
		sleep $(echo $waitTime * 60 - $end + $start | bc) #question: what if $waitTime < end-start?
	fi
done
