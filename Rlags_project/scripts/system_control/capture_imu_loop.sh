#!/bin/bash

echo "IMU: starting IMU capture"
cd ~/Rlags_project/scripts/imu/build

while true
do
	echo "IMU: querying IMU"
	./get_imu_data.sh
	./move_imu_data.sh

	echo "temp"
	sleep 5
done

