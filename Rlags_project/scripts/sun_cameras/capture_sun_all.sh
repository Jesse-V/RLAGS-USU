#!/bin/bash

#unbind all
sudo echo '1-2.1.5.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
sudo echo '1-2.1.5.2' | sudo tee /sys/bus/usb/drivers/usb/unbind
sudo echo '1-2.1.5.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
sudo echo '1-2.1.5.4' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.5

#get image from camera 0
sudo echo '1-2.1.5.1' | sudo tee /sys/bus/usb/drivers/usb/bind
#sleep 0.5
sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun0_image
#sleep 0.5
sudo echo '1-2.1.5.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
#sleep 0.5

#get image from camera 1
sudo echo '1-2.1.5.2' | sudo tee /sys/bus/usb/drivers/usb/bind
#sleep 0.5
sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun1_image
#sleep 0.5
sudo echo '1-2.1.5.2' | sudo tee /sys/bus/usb/drivers/usb/unbind
#sleep 0.5

#get image from camera 2
sudo echo '1-2.1.5.3' | sudo tee /sys/bus/usb/drivers/usb/bind
#sleep 0.5
sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun2_image
#sleep 0.5
sudo echo '1-2.1.5.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
#sleep 0.5

#post-capture processing and file placement
#sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_angle.sh

sudo cp /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_*.jpg /home/linaro/latestData/

sudo mv /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_0.jpg /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_0.`date +"%d.%H.%M.%S"`.jpg
sudo mv /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_1.jpg /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_1.`date +"%d.%H.%M.%S"`.jpg
sudo mv /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_2.jpg /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_2.`date +"%d.%H.%M.%S"`.jpg

sudo cp /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_*.jpg /media/ssd_0/sun_cameras/
sudo cp /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_*.jpg /media/ssd_1/sun_cameras/

sudo rm /home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_*.jpg
