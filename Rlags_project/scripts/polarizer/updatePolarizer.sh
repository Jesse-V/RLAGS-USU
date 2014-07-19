#!/bin/bash

gps=$(cat ../../gps/latestGps)

cd ../../imu/build/
./get_imu_data.sh
imu=$(cat new_cc_data.txt | python parseCC.py)

cd ../../polarizer/build/

year=$(date +'%Y')
month=$(date +'%m')
day=$(date +'%d')
hour=$(date +'%H')
min=$(date +'%M')
sec=$(date +'%S')

angle=$(./polarizer ${year} ${month} ${day} ${hour} ${min} ${sec} ${gps} ${imu})
#echo ${year} ${month} ${day} ${hour} ${min} ${sec} ${gps} ${imu}
echo $angle
echo $angle >> ~/Rlags_project/scripts/communication/build/serial_input
