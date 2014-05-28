#!/usr/bin/python
# Markham Thomas  2/28/2013
#
# SFR DAC demo program
# This program demonstrates using the SFR registers to 
# setup and control DAC output from the USB IO board
#
# NOTES:   RA2 is DACOUT
# FYI:  PWM can be used as a poor-mans DAC also
from hk_usb_io import *
from sfr_constants import *
import sys
import time

#-------------- USB init required --------
usb = init()			# init the USB IO board
#---------------
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "----------- output ------------"

#---- SPI SFR constants --------------
dac = Bunch(pic_registers)
bit = Bunch(pic_bits)

#------------- DAC init code ------------
def my_dac_init(output):
	sfr_set_regbit(usb, dac.ANSELA, 2, 1)
	# analog channel selection bits
	sfr_set_reg(   usb, dac.ADCON0, 0b01111000)
	# setup fixed voltage reference control register
	# 0x00 sets FVR disabled, FVR output off
	#sfr_set_reg(usb, dac.VREFCON0, 0b11010000)
	sfr_set_reg(usb, dac.VREFCON0, 0b00000000)
	# setting DAC output voltage to Vsrc+
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACEN, 0)
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACLPS, 1)
	# DAC voltage output enable bit
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACOE, 1)
	# set DACPSS bits to the proper positive source
	# (0,0) = VDD  (0,1) = Vref+  (1,0) = FVR BUF1 output
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACPSS1, 0)
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACPSS0, 0)
	# DACR<4:0>: DAC Voltage Output Select bits
	# VOUT = ((VSRC+) - (VSRC-))*(DACR<4:0>/(2^5)) + VSRC
	#sfr_set_reg(   usb, dac.VREFCON2, 0b00011111)  # clamp to max VSS
	# valid range is:  0x00 to 0x1F (a total of 32 values)
	sfr_set_reg(   usb, dac.VREFCON2, output)
	# now enable the dac
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACEN, 1)
def my_dac_out(output):
	sfr_set_reg(   usb, dac.VREFCON2, output)
def my_dac_stop():
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACEN, 0)
	sfr_set_regbit(usb, dac.ANSELA, 2, 0)
	sfr_set_regbit(usb, dac.VREFCON1, bit.VREFCON1_DACOE, 0)
	

# initialize the registers needed for DAC output
my_dac_init(0x00)		
# sweep range of output values from 0 to 0x1f
for n in range (0, 0x20):
	my_dac_out(n)
	time.sleep(0.5)
time.sleep(5)
my_dac_stop()
