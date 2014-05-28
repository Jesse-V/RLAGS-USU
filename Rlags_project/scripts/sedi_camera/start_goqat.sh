#!/bin/bash
sudo ps aux | grep Xvfb | grep -v grep | awk {'print $2'} | xargs sudo kill
sudo xvfb-run GoQat &
