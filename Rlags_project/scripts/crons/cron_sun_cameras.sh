inits=$(ps aux | grep initialize_system | grep -v grep | grep -v cron_initialize_system)
if echo $inits | grep initialize_system > /dev/null
	then
		capts=$(ps aux | grep capture_sun_cameras | grep -v grep)
		echo system is initialized
	if ! echo $capts | grep capture_sun_cameras > /dev/null
		then
		sudo /home/linaro/Rlags_project/scripts/system_control/capture_sun_cameras.sh &
		echo starting sun captures...
	fi
fi
