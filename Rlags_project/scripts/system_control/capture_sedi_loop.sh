#!/bin/bash

cd ~/Rlags_project/scripts/sedi_camera

while true
do
	#TODO: turn on calibration lamp
	#TODO: confirm light is on

	./capture.sh calibration_watchfile calibration

	#TODO: turn off lamp
	#TODO: confirm lamp is off

	sleep 0.1

	./capture.sh calibration_watchfile calibration

	sleep 0.1

	./capture capture_watchfile capture
done
