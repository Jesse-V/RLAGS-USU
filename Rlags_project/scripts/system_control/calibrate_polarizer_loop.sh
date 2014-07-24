#!/bin/bash

echo "Polarizer: beginning continue polarizer adjustments"
cd ~/Rlags_project/scripts/polarizer/build
start=$(date +%s.%N)

counter=0

while true
do
	./updatePolarizer.sh > ~/latestData/polarizerInfo.txt

	cat ~/latestData/polarizerInfo.txt >> /media/ssd_0/polarizer/stream.$start.txt
	cat ~/latestData/polarizerInfo.txt >> /media/ssd_1/polarizer/stream.$start.txt

	cat ~/latestData/polarizerInfo.txt

#	angle=$(tail -1 ~/latestData/polarizerInfo.txt)
#	((counter++))

#	if [[ $((counter % 5)) -eq 0 ]]; then
#		echo "Polarizer: set to "$angle", "$(date)
#	fi

	sleep 2.5
done

