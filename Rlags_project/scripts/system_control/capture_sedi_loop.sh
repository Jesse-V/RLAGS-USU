#!/bin/bash

cd ~/Rlags_project/scripts/communication
commPID=$(./get_arduino_comm_pid.sh)
echo "SEDI: Arduino communication PID is: "$commPID

cd ~/Rlags_project/scripts/sedi_camera
rm -rf workingDir
mkdir workingDir

echo "SEDI: capture loop started"

# takes one argument: the capture trial number, 1, 2, or 3
function startCycle() {
	echo "SEDI: capture cycle started, "$(date)

	#get current time, this will be our working directory
	dirName=$(date +%s.%N)_$1

	echo "SEDI: turning lamp on"
	echo 200 >> ~/Rlags_project/scripts/communication/build/serial_input

	#calibration with lamp
	sudo ./capture.sh calibration_watchfile calibration_lamp $dirName

	echo "SEDI: turning lamp off"
	echo 201 >> ~/Rlags_project/scripts/communication/build/serial_input

	#calibration with no lamp
	sudo ./capture.sh calibration_watchfile calibration_nolamp $dirName

	#scientific capture
	sudo ./capture.sh exposure_watchfile$1 capture_trial$1 $dirName

	echo "SEDI: storing camera data"
	cd workingDir

	(cd ~/Rlags_project/scripts; ./getDataLock.sh) #request mutex on latestData/
	cp $dirName/* ~/latestData/sedi/
	(cd ~/Rlags_project/scripts; ./releaseDataLock.sh) #release mutex on latestData/

	#archive SEDI data
	cp -r $dirName /media/ssd_0/sedi_camera/
	cp -r $dirName /media/ssd_1/sedi_camera/

	if [ -d /media/ssd_0/sedi_camera/$dirName ]; then
        	echo "SEDI: data directory confirmed on SSD 0"
	else
		echo "SEDI: error: data not stored on SSD 0"
	fi

	if [ -d /media/ssd_1/sedi_camera/$dirName ]; then
                echo "SEDI: data directory confirmed on SSD 1"
        else
                echo "SEDI: error: data not stored on SSD 1"
        fi

	rm -r $dirName
	cd ..

	echo "SEDI: capture cycle ended, "$(date)
	echo "UPTIME: "$(uptime)
	# sleep 5 #leaves time to control-C if need be
}

while true
do
	# run each calibration_lamp, calibration_nolamp, capture cycle
	# each scientific capture is one of three possible watchfiles
	# Landon wanted a consistent integration time for calibration, but 120/180/240 seconds for capture
	# these values have been stored to the Odroid in the appropriate watchfile
	startCycle 1
	startCycle 2
	startCycle 3
done
