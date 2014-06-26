#!/bin/bash

echo "Housekeeping: assembling latest data"

cd ~/latestData/
mkdir housekeeping/

echo $date > housekeeping/timestamp.txt
cat status.log | tail -75 > housekeeping/latestEvents.txt
cp -r sedi/ housekeeping/

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

echo "Housekeeping: bundling complete"
