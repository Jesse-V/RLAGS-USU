#!/bin/bash

distributeCommand()
{
	cd ~/Rlags_project/scripts/sedi_camera
	./update_watchfile $1 exposure_watchfile #changes integration time for capture
	./update_watchfile $2 calibration_watchfile

	if [ $3 -eq 911 ]
	then
		sudo reboot
	fi

	echo -n $4 > transmission_rate
}

#TODO: reads from serial-USB, creates pulled.txt

time=$(date +%d.%H.%M.%S)
cp pulled.txt /media/ssd_0/link_received/received_$time.txt
cp pulled.txt /media/ssd_1/link_received/received_$time.txt

#command=$(cat pulled.txt | sed 's/ /\n/g')
distributeCommand $(cat pulled.txt)
