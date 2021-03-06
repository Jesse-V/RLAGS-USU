#!/bin/bash

#old_IFS=$IFS  # save the field separator
#IFS=$'\n'     # new field separator, the end of line

#kill all existing GoQat instances
echo "SEDI: killing any existing GoQat processes..."
sudo rm -f /root/GoQat/ccd_display.fit
goInstances=$(ps aux | grep -E "xvfb-run|GoQat" | grep -v grep | awk {'print $2 NL'})
for instance in $goInstances
do
	sudo kill $instance
done

#IFS=$old_IFS     # restore default field separator

#launch a GoQat instance in the background
echo "SEDI: starting fresh GoQat instance"
su -c "xvfb-run GoQat &"
