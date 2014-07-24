#!/bin/bash
cd /home/linaro/Rlags_project/scripts/star_camera

#--------------------------------------------------------------------------

echo '1-2.1.2.4' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null
sleep 0.5

#echo "auto"
now=$(date +%s.%N)
echo "1500 1" | sudo ./get_star_image
cp star_cam.jpg /home/linaro/latestData/
mv star_cam.jpg starcam_$now.auto.jpg
#sleep 0.2

#echo "1"
now=$(date +%s.%N)
echo "10 0" | sudo ./get_star_image
mv star_cam.jpg starcam_$now.10.jpg
#sleep 0.2

#echo "5"
now=$(date +%s.%N)
echo "25 0" | sudo ./get_star_image
mv star_cam.jpg starcam_$now.25.jpg
#sleep 0.2

#echo "10"
now=$(date +%s.%N)
echo "50 0" | sudo ./get_star_image
mv star_cam.jpg starcam_$now.50.jpg
#sleep 0.2

echo '1-2.1.2.4' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#--------------------------------------------------------------------------

sudo cp starcam_*.jpg /media/ssd_0/star_camera/
sudo cp starcam_*.jpg /media/ssd_1/star_camera/

sudo rm starcam_*.jpg
