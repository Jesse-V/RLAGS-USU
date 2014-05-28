#!/bin/bash
sudo chown linaro /root/GoQat/ccd_display.fit
sudo chgrp linaro /root/GoQat/ccd_display.fit

sudo mv /root/GoQat/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/ccd_display.fit /home/linaro/Rlags_project/scripts/sedi_camera/sedi.`date +"%d.%H.%M.%S"`.fit
sudo cp /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /home/linaro/Rlags_project/scripts/fake_drive_0/
sudo mv /home/linaro/Rlags_project/scripts/sedi_camera/sedi.* /home/linaro/Rlags_project/scripts/fake_drive_1

