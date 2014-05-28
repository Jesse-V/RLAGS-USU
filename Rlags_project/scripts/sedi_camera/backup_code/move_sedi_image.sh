#!/bin/bash
while [ ! -f /root/GoQat/ccd_display.fit ]; 
do
#	echo "Doesn't exist yet"
	sleep 1
done

sudo chown linaro /root/GoQat/ccd_display.fit
sudo chgrp linaro /root/GoQat/ccd_display.fit

sudo mv /root/GoQat/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera/sedi.`date +"%d.%H.%M.%S"`.fit
sudo cp /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /media/ssd_0/sedi_camera/
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /media/ssd_1/sedi_camera/

