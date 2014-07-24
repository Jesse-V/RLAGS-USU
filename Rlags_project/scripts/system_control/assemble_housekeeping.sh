#!/bin/bash

echo "Housekeeping: assembling latest data"
(cd ~/Rlags_project/scripts; ./getDataLock.sh) #request mutex on latestData/

cd ~/latestData/
mkdir housekeeping/
touch housekeeping/bundle.txt

#timestamp, log file + temperature + IMU + GPS
echo "****BEGIN****"		>> housekeeping/bundle.txt
echo $(date) 			>> housekeeping/bundle.txt
echo $(uptime) 			>> housekeeping/bundle.txt
echo -e "\n****EVENTS***" 	>> housekeeping/bundle.txt
tail -100 status.log		>> housekeeping/bundle.txt
echo -e "\n****GOQAT****" 	>> housekeeping/bundle.txt
sudo tail -40 /root/GoQat/log.txt >> housekeeping/bundle.txt
echo -e "\n****TEMPS****" 	>> housekeeping/bundle.txt
cat odroidTemperature.txt 	>> housekeeping/bundle.txt
echo ""				>> housekeeping/bundle.txt
cat thermal_sensors.txt 	>> housekeeping/bundle.txt
echo -e "\n*****IMU*****"	>> housekeeping/bundle.txt
cat cc_imu.txt			>> housekeeping/bundle.txt
cat d2_imu.txt			>> housekeeping/bundle.txt
echo -e "\n*****GPS*****"	>> housekeeping/bundle.txt
cat gpsData.txt			>> housekeeping/bundle.txt
echo -e "\n*****END*****"	>> housekeeping/bundle.txt

#cp polarizerInfo.txt housekeeping/polarizerInfo.txt

cp housekeeping/bundle.txt bundle.txt
rm -r housekeeping/

(cd ~/Rlags_project/scripts; ./releaseDataLock.sh) #release mutex on latestData/
echo "Housekeeping: bundling complete"
