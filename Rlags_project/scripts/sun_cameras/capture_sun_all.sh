#!/bin/bash

sudo echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5

sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun0_image
#sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_angle.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/rename_image0.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/mov_to_drive.sh

sudo echo '1-2.1.3.1' | sudo tee /sys/bus/usb/drivers/usb/unbind

#------------------------------------------------------------------------

sudo echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5

sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun1_image
#sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_angle.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/rename_image1.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/mov_to_drive.sh

sudo echo '1-2.1.3.2' | sudo tee /sys/bus/usb/drivers/usb/unbind

#--------------------------------------------------------------------------

sudo echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/bind

sleep 0.5

sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_sun2_image
#sudo /home/linaro/Rlags_project/scripts/sun_cameras/get_angle.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/rename_image2.sh
sudo /home/linaro/Rlags_project/scripts/sun_cameras/mov_to_drive.sh

sudo echo '1-2.1.3.3' | sudo tee /sys/bus/usb/drivers/usb/unbind
