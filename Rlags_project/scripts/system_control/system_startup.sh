cd ~/control_scripts
echo -e "\nSystem startup at "$(date)"\n"
sleep 3
dmesg | tail -10
echo -e "\n"

echo "Startup: initializing system..."
./initialize_system.sh
echo "Startup: system initialization complete"

echo "Startup: beginning sanity checks"

nSunCameras=$(dmesg | grep -c 'ZWOptical company')
if [ $nSunCameras -ge 3 ]; then
	echo "Startup: notice: Counted "$nSunCameras" sun cameras"
else
	echo "Startup: ERR: Not enough sun cameras (counted "$nSunCameras")"
fi

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

echo "Startup: end of sanity checks"
sleep 5 #ensure that everything is fully ready
echo "Startup: beginning scientific capture"

#./capture_sedi_loop.sh &
#./capture_cameras_loop.sh &
#./capture_imu_loop.sh &

#wait until star/sun images come in, then we have something
while [ ! -f ~/latestData/star_cam.jpg ];
do
        sleep 0.25
done

#now that we have some data to transmit, start sending
./stream_data_bundle.sh &
