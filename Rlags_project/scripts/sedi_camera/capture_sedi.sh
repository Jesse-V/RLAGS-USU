#!/bin/bash
sudo rm -f /root/GoQat/ccd_display.fit
sudo cp /home/linaro/Rlags_project/scripts/sedi_camera/exposure_watchfile /home/my_watch
start=$(date +"%d.%H.%M.%S")
while [ ! -f /root/GoQat/ccd_display.fit ];
do
#       echo "Doesn't exist yet"
        sleep 0.001
done

sudo mv /root/GoQat/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera/
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera/sedi.$start.fit
sudo cp /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /media/ssd_0/sedi_camera/
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /media/ssd_1/sedi_camera/

