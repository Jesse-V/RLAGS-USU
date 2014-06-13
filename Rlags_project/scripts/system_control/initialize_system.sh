#!/bin/bash

#wait until SSDs are recongized by the kernel
while [ $(dmesg | grep -c "Apricorn SATAWire") -lt 2 ]; do
	sleep 0.25
done
sleep 0.5
echo "SSDs recognized by kernel, will now mount"

sudo /home/linaro/Rlags_project/scripts/drives/mount_drives
sudo rm -f /root/GoQat/ccd_display.fit
