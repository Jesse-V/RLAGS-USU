#!/bin/bash
cd /home/linaro/Rlags_project/scripts/sun_cameras/

echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5
time0=$(date +%d.%H.%M.%S)
sudo ./get_sun0_image

echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/unbind

#------------------------------------------------------------------------

echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5
time1=$(date +%d.%H.%M.%S)
sudo ./get_sun1_image

echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/unbind

#--------------------------------------------------------------------------

echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5
time2=$(date +%d.%H.%M.%S)
sudo ./get_sun2_image

echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/unbind

#--------------------------------------------------------------------------

sudo cp sun_cam_*.jpg /home/linaro/latestData/

sudo mv sun_cam_0.jpg sun_cam_0.$time0.jpg
sudo mv sun_cam_1.jpg sun_cam_1.$time1.jpg
sudo mv sun_cam_2.jpg sun_cam_2.$time2.jpg

sudo cp sun_cam_*.jpg /media/ssd_0/sun_cameras
sudo cp sun_cam_*.jpg /media/ssd_1/sun_cameras

sudo rm sun_cam_*.jpg
