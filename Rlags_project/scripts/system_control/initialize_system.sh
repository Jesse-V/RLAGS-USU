#!/bin/bash

#wait until SSDs are recognized by the kernel
while [ $(dmesg | grep -c "Apricorn SATAWire") -lt 2 ]; do
	sleep 0.25
done
sleep 0.5
echo "SSDs recognized by kernel, will now mount"

echo "Mounting SSDs..."
sudo /home/linaro/Rlags_project/scripts/drives/mount_drives

echo "Initializing GoQat SEDI camera..."
sudo /home/linaro/Rlags_project/scripts/sedi_camera/start_goqat.sh
