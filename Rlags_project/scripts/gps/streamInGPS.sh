#!/bin/bash

rm -f gpsStream.txt
cat < /dev/ttyUSB0 > gpsStream.txt
