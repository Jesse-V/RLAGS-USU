#!/bin/bash
cd /home/linaro/Rlags_project/scripts/star_camera

#--------------------------------------------------------------------------

echo '1-2.1.5.4' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null

sleep 1
time=$(date +%d.%H.%M.%S)
#echo "star cap"
sudo ./get_star_image
#echo "end"
sleep 0.1

echo '1-2.1.5.4' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#--------------------------------------------------------------------------

sudo cp star_cam.jpg /home/linaro/latestData/
sudo mv star_cam.jpg star_cam.$time.jpg

sudo cp star_cam*.jpg /media/ssd_0/star_camera/
sudo cp star_cam*.jpg /media/ssd_1/star_camera/

sudo rm star_cam*.jpg

