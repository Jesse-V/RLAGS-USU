#!/bin/bash

sudo echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25
sudo echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25
sudo echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25

while true
do
	start=$(date +%s.%N)
	echo "Sun capture started "$start

	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_0.sh > cameras.txt
	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_1.sh > cameras.txt
	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_2.sh > cameras.txt

	#sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_0.sh
        #sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_1.sh
        #sudo /home/linaro/Rlags_project/scripts/sun_cameras/capture_sun_2.sh

	cd /home/linaro/Rlags_project/scripts/sun_cameras/
	./capture_sun_all.sh
	./get_angle.sh > new_angles.txt &

	echo "Sun capture ended   "$(date +%s.%N)

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start - 0.05 | bc)
done
