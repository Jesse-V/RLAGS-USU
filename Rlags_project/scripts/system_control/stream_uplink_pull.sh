#!/bin/bash

time=$(date +%s.%N)

distributeCommand()
{
	if [ $# -eq 0 ]; then
		echo "Link: no arguments supplied, ignoring"
		return
	fi

	echo "Link: received command "$1

	if [[ $1 == *10EF* ]]; then
		sudo reboot -f
	fi

	if [[ $1 == *EF10* ]]; then
                sudo reboot -f
	fi
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
tail -f uplinkBuffer.txt | grep --line-buffered -E "*" | waitForCommand
