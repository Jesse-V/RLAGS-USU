#!/bin/bash

sudo echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.2
sudo echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.2
sudo echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.2

#while true
#do
	start=$(date +%s)
	echo $start

	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_0.sh > cameras.txt
	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_1.sh > cameras.txt
	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_2.sh > cameras.txt

	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_0.sh
        #sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_1.sh
        #sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_2.sh

	sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_all.sh

	duration=$(( $(date +%s) - $start ))
	sleep $(( 10 - duration ))
	echo $(date +%s)
#done
