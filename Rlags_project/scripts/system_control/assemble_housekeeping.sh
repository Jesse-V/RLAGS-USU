#!/bin/bash

echo "Housekeeping: assembling latest data"
(cd ~/Rlags_project/scripts; ./getDataLock.sh) #request mutex on latestData/

cd ~/latestData/
mkdir housekeeping/

#gemeral assembly, grab last bits of logs
echo $date > housekeeping/timestamp.txt
tail -75 status.log > housekeeping/latestEvents.txt
echo "**************************" >> housekeeping/latestEvents.txt
sudo tail -50 /root/GoQat/log.txt >> housekeeping/latestEvents.txt

#if they exist, truncate SEDI images to save space
if [ -f ~/latestData/sedi/capture.fit ]; then
	echo "Housekeeping: preparing SEDI .fit images"
	cp -r sedi/*.txt housekeeping/ #move all housekeeping data files

	cp sedi/*.fit housekeeping/
fi

echo "Housekeeping: compressing sun/star images"
convert star_cam.jpg  -resize 640x480 housekeeping/star_cam.jpg
convert sun_cam_0.jpg -resize 640x480 housekeeping/sun_cam_0.jpg
convert sun_cam_1.jpg -resize 640x480 housekeeping/sun_cam_1.jpg
convert sun_cam_2.jpg -resize 640x480 housekeeping/sun_cam_2.jpg

echo "Housekeeping: compressing bundle"

cd housekeeping/
tar -cjf ../bundle.tar.bz2 *
cd ..

rm -r housekeeping/

(cd ~/Rlags_project/scripts; ./releaseDataLock.sh) #release mutex on latestData/
echo "Housekeeping: bundling complete"
