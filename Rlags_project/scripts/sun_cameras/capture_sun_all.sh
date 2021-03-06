#!/bin/bash
cd /home/linaro/Rlags_project/scripts/sun_cameras/

echo '1-2.1.2.1' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null

sleep 0.5
time0=$(date +%s.%N)
#echo "sun0 cap"
sudo ./get_sun0_image
#echo "end"
sleep 0.1

echo '1-2.1.2.1' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#------------------------------------------------------------------------

echo '1-2.1.2.2' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null

sleep 0.5
time1=$(date +%s.%N)
#echo "sun1 cap"
sudo ./get_sun1_image
#echo "end"
sleep 0.1

echo '1-2.1.2.2' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#--------------------------------------------------------------------------

echo '1-2.1.2.3' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null

sleep 0.5
time2=$(date +%s.%N)
#echo "sun2 cap"
sudo ./get_sun2_image
#echo "end"
sleep 0.1

echo '1-2.1.2.3' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#--------------------------------------------------------------------------

(cd ~/Rlags_project/scripts; ./getDataLock.sh) #request mutex on latestData/
sudo cp sun_cam_*.jpg /home/linaro/latestData/
(cd ~/Rlags_project/scripts; ./releaseDataLock.sh) #release mutex on latestData/

sudo mv sun_cam_0.jpg suncam_$time0.0.jpg
sudo mv sun_cam_1.jpg suncam_$time1.1.jpg
sudo mv sun_cam_2.jpg suncam_$time2.2.jpg

sudo cp suncam_*.jpg /media/ssd_0/sun_cameras/
sudo cp suncam_*.jpg /media/ssd_1/sun_cameras/

sudo rm suncam_*.jpg

