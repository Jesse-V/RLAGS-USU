#!/bin/bash
while true
do
	start=$(date +%s)

	sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_0.sh > cameras.txt 2> /dev/null
	sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_1.sh > cameras.txt 2> /dev/null
	sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_2.sh > cameras.txt 2> /dev/null

	echo "waiting..."
	while [ $(expr $(date +%s) - $start)  -lt 10 ]; do
		sleep 0.001
	done
done
