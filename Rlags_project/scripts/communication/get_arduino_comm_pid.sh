#!/bin/bash

ps aux | grep /seria[l] | awk '{print $2}'

#note: to send messages to the process: echo hey > /proc/$PID/fd/0
