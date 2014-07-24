#!/bin/bash

echo "IMU: starting IMU capture"
cd ~/Rlags_project/scripts/imu/build

start=$(date +%s.%N)

while true
do
	./get_imu_data.sh

	now=$(date +%s.%N)
	ccData=$now": "$(cat new_cc_data.txt)
	d2Data=$now": "$(cat new_d2_data.txt)

	echo $ccData > ~/latestData/cc_imu.txt
	echo $d2Data > ~/latestData/d2_imu.txt

	echo $ccData >> /media/ssd_0/imu/cc_stream_$start.txt
	echo $ccData >> /media/ssd_1/imu/cc_stream_$start.txt

	echo $d2Data >> /media/ssd_0/imu/d2_stream_$start.txt
	echo $d2Data >> /media/ssd_1/imu/d2_stream_$start.txt

	sleep 0.5
done

