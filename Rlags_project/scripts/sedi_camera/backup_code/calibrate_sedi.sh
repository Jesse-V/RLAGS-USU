#!/bin/bash
sudo rm -f /root/GoQat/ccd_display.fit
#turn on the lamp here
sudo cp /home/linaro/Rlags_project/scripts/sedi_camera/calibration_watchfile /home/my_watch
while [ ! -f /root/GoQat/ccd_display.fit ];
do
#if file doesn't show up in 1 minute, kill initialize_system
#       echo "Doesn't exist yet"
        sleep 0.001
done
#turn off the lamp here
sudo rm -f /root/GoQat/ccd_display.fit
