#!/bin/bash
inits=$(ps aux | grep initialize_system | grep -v grep | grep -v cron_initialize_system)
if echo $inits | grep initialize_system > /dev/null
        then
                imus=$(ps aux | grep imu_execution.sh | grep -v grep)
                echo system is initialized
        if ! echo $imus | grep imu_execution.sh > /dev/null
                then
			echo "Started imu"
			sudo /home/linaro/Rlags_project/scripts/system_control/imu_execution.sh &
	fi
fi

