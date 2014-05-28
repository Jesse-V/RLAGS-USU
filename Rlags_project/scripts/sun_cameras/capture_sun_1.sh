#!/bin/bash
sudo echo '1-2.1.5.1' | sudo tee /sys/bus/usb/drivers/usb/unbind
sudo echo '1-2.1.5.2' | sudo tee /sys/bus/usb/drivers/usb/bind
sudo echo '1-2.1.5.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
sudo echo '1-2.1.5.4' | sudo tee /sys/bus/usb/drivers/usb/unbind
sleep 0.5
sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun1_image
#sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_angle.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/rename_image1.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/mov_to_drive.sh
