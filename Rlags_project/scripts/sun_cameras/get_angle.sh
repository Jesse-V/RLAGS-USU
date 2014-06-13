#!/bin/bash

cd /home/linaro/Rlags_project/scripts/filter/

for j in 0 1 2
do
	cp /home/linaro/latestData/sun_cam_$j.jpg image.jpg
	echo "Angle for sun_cam_$j.jpg:"
	python sun_center.py
done
