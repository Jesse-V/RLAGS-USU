#!/bin/bash

queueUp()
{
	while true
	do
		read line
		echo $line >> serialFeed

		sleep 4
	done
}

tail -f rawSerialInput | grep --line-buffered -E "*" | queueUp
