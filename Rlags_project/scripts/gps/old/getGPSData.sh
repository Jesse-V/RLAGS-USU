#!/bin/bash

gpspipe -r > gpsData.txt & tail -f gpsData.txt | grep --line-buffered -E "GPRMC" | python parse.py
