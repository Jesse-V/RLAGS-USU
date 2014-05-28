#!/bin/bash
sudo mv /media/ssd_0/imu/cc_data.txt /media/ssd_0/imu/cc_data.`date +"%d.%H.%M.%S"`.txt
sudo mv /media/ssd_0/imu/d2_data.txt /media/ssd_0/imu/d2_data.`date +"%d.%H.%M.%S"`.txt

sudo mv /media/ssd_1/imu/cc_data.txt /media/ssd_1/imu/cc_data.`date +"%d.%H.%M.%S"`.txt
sudo mv /media/ssd_1/imu/d2_data.txt /media/ssd_1/imu/d2_data.`date +"%d.%H.%M.%S"`.txt
