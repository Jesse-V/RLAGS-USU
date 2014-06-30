#!/bin/bash

echo "TODO: THIS NEEDS TO TRANSMIT EVERY 10 MINUTES, NOT CONTINOUSLY, WHOLE SEDI?"

while true
do
	cd ~/control_scripts

	echo "Comm stream: assembling data bundle, beginning transmission. "$(date)
	./assemble_housekeeping.sh
	#there shouldn't be any status message from this point on, since the log has been bundled for transmission

	#transmit bundle.tar.bz2

	#this is temporary! simulates transmit
	sleep 30
done
