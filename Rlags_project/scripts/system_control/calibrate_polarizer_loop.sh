#!/bin/bash

echo "Polarizer: beginning continue polarizer adjustments"
cd ~/Rlags_project/scripts/polarizer/build
counter=1

while true
do
	angle=$(./updatePolarizer.sh)
	((counter++))

	if [[ $((counter % 5)) -eq 0 ]]; then
		echo "Polarizer: set to "$angle", "$(date)
	fi

	sleep 2.5
done

