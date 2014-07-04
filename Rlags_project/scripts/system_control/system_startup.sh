cd ~/control_scripts
echo -e "\nSystem startup at "$(date)"\n"
sleep 3
dmesg | tail -10
echo -e "\n"

echo "Startup: initializing system..."
./initialize_system.sh
echo "Startup: system initialization complete"

echo "Startup: beginning sanity checks"

#sanity check on sun/star cameras
nSunCameras=$(dmesg | grep -c 'ZWOptical company')
if [ $nSunCameras -ge 4 ]; then
	echo "Startup: notice: Counted "$nSunCameras" sun cameras"
else
	echo "Startup: ERR: Not enough sun cameras (counted "$nSunCameras")"
fi

#sanity check on SSDs
if [ -d /media/ssd_0/sun_cameras ]; then
	echo "Startup: notice: ssd_0 is mounted, sun_camera directory exists"

	file=/media/ssd_0/sun_cameras/temp.txt
	touch $file
	if [ -f $file ]; then
		echo "Startup: notice: SSD write capability functioning normally"
	else
		echo "Startup: ERR: Error writing to SSD"
	fi
	rm $file
else
	echo "Startup: ERR: ssd_0 improperly mounted or sun_camera directory is missing"
fi

#sanity check on SEDI camera
if [ $(dmesg | grep -c 'Starlight Xpress') -ge 1 ]; then
	echo "Startup: notice: SEDI camera is ready"
else
	echo "Startup: ERR: SEDI camera is not found!"
fi

#sanity check on USB-serial communication cables
if [ $(dmesg | grep -c 'USB Serial Device converter now attached') -ge 2 ]; then
        echo "Startup: notice: USB-serial converter is ready"
else
        echo "Startup: ERR: USB-serial converters are not found!"
fi

echo "Startup: end of sanity checks"
sleep 5 #ensure that everything is fully ready
echo "Startup: beginning scientific capture"

#./capture_sedi_loop.sh &	#SEDI camera capturing loop
#./capture_cameras_loop.sh &	#Sun and star capturing loop
#./capture_imu_loop.sh &	#IMU querying and archiving
#./pull_command_loop.sh &	#Wait for commands through the uplink

#wait until star/sun images come in, then we have something
while [ ! -f ~/latestData/star_cam.jpg ];
do
        sleep 0.25
done

#now that we have some data to transmit, start sending
#./push_bundle_loop.sh &		#begin downlink transmission loop

echo "Startup: startup complete, all systems activated. "$(date)
