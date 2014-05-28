#!/bin/bash
#echo "Started imu"
while :
do
	sudo /home/linaro/Rlags_project/scripts/imu/get_imu_data.sh
	sudo /home/linaro/Rlags_project/scripts/imu/move_imu_data.sh
        sleep 1
done

