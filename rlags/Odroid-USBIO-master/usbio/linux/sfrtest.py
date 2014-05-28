#!/usr/bin/python
# Markham Thomas  2/25/2013
# SFR test program
#   Updated to use the constants file:  sfr_constants.py
# and the class Bunch()
from hk_usb_io import *
from sfr_constants import *
import sys
import time

usb = init()			# init the USB IO board

print module_version()		# print python module version

print rom_version(usb)		# print rom version
print "---------- output -----------"

# instantiate your class of pic_registers
ior = Bunch(pic_registers)		# uses the sfr_constants.py file

a = sfr_get_reg(usb, ior.SSP1BUF)	# easy reference to the PIC SFR's
print hex(a)
