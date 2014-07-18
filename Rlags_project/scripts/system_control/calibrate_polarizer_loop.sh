#!/bin/bash

echo "Polarizer: beginning continue polarizer adjustments"
cd ~/Rlags_project/scripts/polarizer/build

while true
do
	./updatePolarizer.sh
	sleep 2.5
done

