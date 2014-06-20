#!/bin/bash

while true
do
	cd ~/control_scripts

	echo "Assembling data bundle, beginning transmission. "$(date)
	./assemble_housekeeping.sh
	#there shouldn't be any status message from this point on, since the log has been bundled for transmission

	#transmit bundle.tar.bz2

	#this is temporary! simulates transmit
	sleep 30
done
