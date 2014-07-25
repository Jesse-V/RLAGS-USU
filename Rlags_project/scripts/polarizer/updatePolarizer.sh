#!/bin/bash

gps=$(cat ../../gps/latestGps)

cd ../../imu/build/
./get_imu_data.sh
imu=$(cat new_cc_data.txt | python parseCC.py)

cd ../../polarizer/build/

year=$(date -u +'%Y')
month=$(date -u +'%m')
day=$(date -u +'%d')
hour=$(date -u +'%H')
min=$(date -u +'%M')
sec=$(date -u +'%S')

angle=$(./polarizer ${year} ${month} ${day} ${hour} ${min} ${sec} ${gps} ${imu})
echo ${gps}
echo ${imu}
echo ${year} ${month} ${day} ${hour} ${min} ${sec} ${gps} ${imu}
echo $angle", "$(date)
#echo $angle >> ~/Rlags_project/scripts/communication/build/serial_input
