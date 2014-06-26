cd ~/control_scripts
echo -e "\nSystem startup at "$(date)"\n"
dmesg | tail -10

echo "Initializing system..."
./initialize_system.sh
echo "System initialization complete"

echo "Beginning sanity checks"

nSunCameras=$(dmesg | grep -c 'ZWOptical company')
if [ $nSunCameras -ge 3 ]; then
	echo "NOTICE: Counted "$nSunCameras" sun cameras"
else
	echo "ERR: Not enough sun cameras (counted "$nSunCameras")"
fi

if [ -d /media/ssd_0/sun_cameras ]; then
	echo "NOTICE: ssd_0 is mounted, sun_camera directory exists"

	file=/media/ssd_0/sun_cameras/temp.txt
	touch $file
	if [ -f $file ]; then
		echo "NOTICE: SSD write capability functioning normally"
	else
		echo "ERR: Error writing to SSD"
	fi
	rm $file
else
	echo "ERR: ssd_0 improperly mounted or sun_camera directory is missing"
fi

echo "End of sanity checks"
sleep 5 #ensure that everything is fully ready
echo "Begin scientific capture"



#sudo ps aux | grep GoQat | grep -v grep | awk {'print $2'} | xargs sudo kill
#sudo /home/linaro/Rlags_project/scripts/sedi_camera/start_goqat.sh &



#------------------------------- SCIENTIFIC CAPTURE ---------------------------
#nSunCameras=sudo -u linaro sleep 5; /home/linaro/Rlags_project/scripts/system_control/initialize_system.sh
#sudo -u linaro touch /home/linaro/tempDerp.temp
#sudo -u linaro touch /home/linaro/tempDerp.temp
