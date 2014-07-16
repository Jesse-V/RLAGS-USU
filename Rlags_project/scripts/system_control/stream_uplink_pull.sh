#!/bin/bash

time=$(date +%d.%H.%M.%S)

distributeCommand()
{
	if [ $# -eq 0 ]; then
		echo "Link: no arguments supplied, ignoring"
		return
	fi

	if [ -z "$4" ]; then
		echo "Link: not enough arguments, ignoring"
		return
	fi

	echo "Link: received command "$1" "$2" "$3" "$4

	cd ~/Rlags_project/scripts/sedi_camera
	./update_watchfile $1 exposure_watchfile #changes integration time for capture
	./update_watchfile $2 calibration_watchfile

	if [ $3 -eq 911 ]
	then
		sudo reboot
	fi

	echo -n $4 > transmission_rate
}

waitForCommand()
{
	while true
	do
		read command

		echo $command >> /media/ssd_0/link_received/received_$time.txt
		echo $command >> /media/ssd_1/link_received/received_$time.txt

		distributeCommand $command
	done
}

cd ~/Rlags_project/scripts/communication
rm -r uplink_in_stream
touch uplink_in_stream
tail -f uplink_in_stream | grep --line-buffered -E "*" | waitForCommand
