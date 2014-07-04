#!/bin/bash

sudo echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25
sudo echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25
sudo echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25
sudo echo '1-2.1.3.4' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.25

while true
do
	start=$(date +%s.%N)
	echo "Sun/star cameras: capture started "$start", "$(date)

	cd /home/linaro/Rlags_project/scripts/sun_cameras/
	./capture_sun_all.sh

	cd /home/linaro/Rlags_project/scripts/star_camera/
        ./capture_star.sh

	echo "Sun/star cameras: capture ended "$(date +%s.%N)

	end=$(date +%s.%N)
	sleep $(echo 10 - $end + $start - 0.009 | bc)
done
