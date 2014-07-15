#!/bin/bash

#echo "Mutex requested on ~/latestData/"

#wait for current lock to disappear
while [ -f ~/latestData/lock ]
do
        sleep 0.05
done

#create new lock
touch ~/latestData/lock
