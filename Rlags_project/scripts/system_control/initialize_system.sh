#!/bin/bash

#wait until SSDs are recognized by the kernel
while [ $(dmesg | grep -c "Apricorn SATAWire") -lt 2 ]; do
	sleep 0.25
done
sleep 0.5
echo "Sys init: SSDs recognized by kernel, will now mount"

echo "Sys init: mounting SSDs..."
sudo ~/Rlags_project/scripts/drives/mount_drives

echo "Sys init: initializing GoQat SEDI camera..."
sudo ~/Rlags_project/scripts/sedi_camera/initialize_goqat.sh

echo "Sys init: resetting ~/latestData"
rm -f ~/latestData/*.jpg
rm -f ~/latestData/*.tar.bz2
rm -f ~/latestData/lock
rm -r -f ~/latestData/sedi
mkdir ~/latestData/sedi

sudo chown linaro ~/latestData/status.log
sudo chmod 666 ~/latestData/status.log

echo "Sys init: initializing Odroid-Arduino communication..."
cd ~/Rlags_project/scripts/communication/build
rm -f serial_input ../serial_output
touch serial_input ../serial_output
tail -f serial_input | grep --line-buffered -E "*" | ./serial &> ../serial_output &

echo "Sys init: setting baudrate for GPS (ttyUSB0) link..."
sudo stty -F /dev/ttyUSB0 600

echo "Sys init: setting baudrate for RX (ttyUSB1) uplink..."
sudo stty -F /dev/ttyUSB1 1200

echo "Sys init: setting baudrate for TX (ttyUSB2) downlink..."
sudo stty -F /dev/ttyUSB2 115200

echo "Sys init: initializing GPS stream reading..."
cd ~/Rlags_project/scripts/gps
./streamInGPS.sh & #begin transferring from ttyUSB0 to buffer file

echo "Sys init: archiving IMU timestamp..."
cd ~/Rlags_project/scripts/imu/build
./get_imu_data.sh
now=$(date +%s.%N)
cat new_cc_data.txt >> /media/ssd_0/time/imu_cc_$now.txt
cat new_d2_data.txt >> /media/ssd_0/time/imu_d2_$now.txt
cat new_cc_data.txt >> /media/ssd_1/time/imu_cc_$now.txt
cat new_d2_data.txt >> /media/ssd_1/time/imu_d2_$now.txt

