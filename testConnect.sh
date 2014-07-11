#!/bin/bash
for i in {1..100000}
do	
	sleep 0.2
	clear
	echo $i 
	cat /sys/devices/virtual/thermal/thermal_zone0/temp|sed 's/000//g'
done
