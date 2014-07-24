#!/bin/bash

while true
do
	start=$(date +%s.%N)
	echo "Comm: preparing for downlink transmission. "$(date)

	cd ~/control_scripts
	./assemble_housekeeping.sh

	cd ~/latestData
	cat -en bundle.txt > /dev/ttyUSB2

	echo "Comm: transmission completed. Archiving."

	cp bundle.txt /media/ssd_0/link_sent/data_$start.txt
	cp bundle.txt /media/ssd_1/link_sent/data_$start.txt

	rm bundle.txt

	end=$(date +%s.%N)
	sleep $(echo 60 - $end + $start - 0.009 | bc)
done
