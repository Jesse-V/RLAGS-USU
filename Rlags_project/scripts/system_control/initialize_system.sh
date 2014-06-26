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

echo "Resetting ~/latestData"
rm ~/latestData/*.jpg
rm ~/latestData/*.tar.bz2
rm -r ~/latestData/sedi
mkdir ~/latestData/sedi
