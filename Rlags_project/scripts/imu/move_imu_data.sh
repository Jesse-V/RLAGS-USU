#!/bin/bash
sudo chmod 777 /media/ssd_0/imu/*
sudo chmod 777 /media/ssd_1/imu/*

sudo cat ./new_cc_data.txt >> /media/ssd_0/imu/cc_data.txt
sudo cat ./new_cc_data.txt >> /media/ssd_1/imu/cc_data.txt

sudo cat ./new_d2_data.txt >> /media/ssd_0/imu/d2_data.txt
sudo cat ./new_d2_data.txt >> /media/ssd_1/imu/d2_data.txt
