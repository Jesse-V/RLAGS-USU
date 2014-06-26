#!/bin/bash

#this script must be run as root!
#arguments: <watch file name> <type of capture> <output directory name>

cd ~/Rlags_project/scripts/sedi_camera
if [ ! -d $3 ]; then
	mkdir $3
	echo "SEDI: made working directory "$3
fi

echo "SEDI: triggering GoQat camera for "$2", "$(date +"%H:%M:%S")
cp $1 /home/my_watch
startTime=$(date +"%d.%H.%M.%S")
startAtt=42 #TODO: get attitude

while [ ! -f /root/GoQat/ccd_display.fit ];
do
	sleep 0.2
done

endTime=$(date +"%d.%H.%M.%S")
endAtt=43 #TODO: get attitude
echo "SEDI: camera returned image, "$(date +"%H:%M:%S")

#move .fit file into directory
mv /root/GoQat/ccd_display.fit $(pwd)/$3/$2.fit

#create housekeeping data file
dataFile=$(pwd)/$3/$2_data.txt
echo "start time, end time, start attitude, end attitude" >> $dataFile
echo $startTime","$endTime","$startAtt","$endAtt >> $dataFile

chown -R linaro $3
