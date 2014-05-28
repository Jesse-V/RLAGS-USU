#!/bin/bash
sudo mv /home/linaro/Rlags_project/scripts/sun_camera_0/sun_cam_0.* /home/linaro/Rlags_project/scripts/filter/
sudo python /home/linaro/Rlags_project/scripts/filter/sun_center.py >> /home/linaro/Rlags_project/scripts/filter/angles.txt
echo "" >> /home/linaro/Rlags_project/scripts/filter/angles.txt
sudo mv /home/linaro/Rlags_project/scripts/filter/sun_cam_0.* /home/linaro/Rlags_project/scripts/sun_camera_0/
