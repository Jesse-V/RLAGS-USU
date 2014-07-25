#!/bin/bash
sudo ./cc > temp.txt
sudo echo "" >> temp.txt
mv temp.txt new_cc_data.txt

sudo ./d2 > temp.txt
sudo echo "" >> temp.txt
mv temp.txt new_d2_data.txt
