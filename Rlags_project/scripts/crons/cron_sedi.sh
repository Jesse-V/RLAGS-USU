#!/bin/bash
inits=$(ps aux | grep initialize_system | grep -v grep | grep -v cron_initialize_system)
if echo $inits | grep initialize_system > /dev/null
        then
                sedi=$(ps aux | grep sedi_execution | grep -v grep)
                echo system is initialized
        if ! echo $sedi | grep sedi_execution > /dev/null
                then
			echo sedi starting
			sudo /home/linaro/Rlags_project/scripts/system_control/sedi_execution.sh &
	fi
fi
