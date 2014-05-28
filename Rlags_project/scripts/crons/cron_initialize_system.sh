inits=$(ps aux | grep initialize_system | grep -v grep | grep -v cron_initialize_system)
if ! echo $inits | grep initialize_system > /dev/null
	then
		sudo /home/linaro/Rlags_project/scripts/system_control/initialize_system.sh & > /dev/null
		echo starting initialization...
fi
