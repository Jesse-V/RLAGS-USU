#!/bin/bash

#IMU SCRIPT MUST BE RUNNING FOR THIS TO WORK RIGHT

echo "Polarizer: beginning continue polarizer adjustments"
cd ~/Rlags_project/scripts/polarizer/build
start=$(date +%s.%N)

counter=0

while true
do
	./updatePolarizer.sh > ~/latestData/polarizerInfo.txt

	cat ~/latestData/polarizerInfo.txt >> /media/ssd_0/polarizer/stream.$start.txt
	cat ~/latestData/polarizerInfo.txt >> /media/ssd_1/polarizer/stream.$start.txt

	angle=$(tail -1 ~/latestData/polarizerInfo.txt)
	((counter++))

	if [[ $((counter % 2)) -eq 0 ]]; then
		echo "Polarizer: set to "$angle
	fi

	sleep 10
done

