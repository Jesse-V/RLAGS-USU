#!/bin/bash
cd /home/linaro/Rlags_project/scripts/star_camera

#--------------------------------------------------------------------------

echo '1-2.1.2.4' | sudo tee /sys/bus/usb/drivers/usb/bind > /dev/null
sleep 0.75

time=$(date +%d.%H.%M.%S)

echo "auto"
echo "1500 1" | sudo ./get_star_image
cp star_cam.jpg /home/linaro/latestData/
mv star_cam.jpg star_cam.$time.auto.jpg
sleep 0.2

echo "300"
echo "300 0" | sudo ./get_star_image
mv star_cam.jpg star_cam.$time.300.jpg
sleep 0.2

echo "500"
echo "500 0" | sudo ./get_star_image
mv star_cam.jpg star_cam.$time.500.jpg
sleep 0.2

echo "700"
echo "700 0" | sudo ./get_star_image
mv star_cam.jpg star_cam.$time.700.jpg
sleep 0.2

echo '1-2.1.2.4' | sudo tee /sys/bus/usb/drivers/usb/unbind > /dev/null

#--------------------------------------------------------------------------

sudo cp star_cam*.jpg /media/ssd_0/star_camera/
sudo cp star_cam*.jpg /media/ssd_1/star_camera/

sudo rm star_cam*.jpg

