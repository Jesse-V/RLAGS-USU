#!/bin/bash

cd ~/Rlags_project/scripts/sedi_camera
echo "SEDI: capture loop started"

#echo "SEDI: initializing SEDI lamp pin"
#echo 87  > /sys/class/gpio/export
#echo out > /sys/class/gpio/gpio87/direction

while true
do
	echo "SEDI: capture cycle started, "$(date)

	#get current time, this will be our working directory
	dirName=$(date +"%d.%H.%M.%S")

	echo "SEDI: turning lamp on"
	#echo 1 > /sys/class/gpio/gpio87/value
	#pinVal=$(cat /sys/class/gpio/gpio87/value)
        #echo "SEDI: lamp pin is now "$pinVal

	#calibration with lamp
	sudo ./capture.sh calibration_watchfile calibration_lamp $dirName

	echo "SEDI: turning lamp off"
        #echo 0 > /sys/class/gpio/gpio87/value
	#pinVal=$(cat /sys/class/gpio/gpio87/value)
        #echo "SEDI: lamp pin is now "$pinVal

	#calibration with no lamp
	sudo ./capture.sh calibration_watchfile calibration_nolamp $dirName

	#scientific capture
	sudo ./capture.sh exposure_watchfile capture $dirName

	echo "SEDI: storing camera data"
	(cd ~/Rlags_project/scripts; ./getDataLock.sh) #request mutex on latestData/
	cp $dirName/* ~/latestData/sedi/
	(cd ~/Rlags_project/scripts; ./releaseDataLock.sh) #release mutex on latestData/

	#archive SEDI data
	cp -r $dirName /media/ssd_0/sedi_camera/
	cp -r $dirName /media/ssd_1/sedi_camera/

	if [ -d /media/ssd_0/sedi_camera/$dirName ]; then
        	echo "SEDI: capture directory confirmed on SSD 0"
	else
		echo "SEDI: error: camera data not stored on SSD 0"
	fi

	if [ -d /media/ssd_1/sedi_camera/$dirName ]; then
                echo "SEDI: capture directory confirmed on SSD 1"
        else
                echo "SEDI: error: camera data not stored on SSD 1"
        fi

	echo "SEDI: cleaning up working directory"
	rm -r $dirName

	echo "SEDI: capture cycle ended, "$(date)

	sleep 2 #leaves time to control-C if need be
done
