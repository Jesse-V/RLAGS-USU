#!/bin/bash
echo "killing"
ps aux | grep -v grep | grep accel | awk {'print $2'} | xargs kill
