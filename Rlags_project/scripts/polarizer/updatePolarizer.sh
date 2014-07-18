#!/bin/bash

echo "getting latest gps data"

gps=$(cat ../../gps/latestGps)

cd ../../imu/build/
ls
./get_imu_data.sh
imu=$(cat new_cc_data.txt | python parseCC.py)

cd ../../polarizer/build/s

echo ${gps}
echo ${imu}