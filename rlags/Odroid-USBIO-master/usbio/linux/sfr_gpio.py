#!/usr/bin/python
# Markham Thomas  2/25/2013
# SFR GPIO test program
# This program demonstrates using the SFR registers to 
# setup and control the GPIO pins
#
#  The new method of defining our PIC18f SFR registers will
# make for more readable code and take fewer characters
from hk_usb_io import *
import sys
import time

#-------------- USB init required --------
usb = init()			# init the USB IO board
#---------------
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "----------- output ------------"

#-------------- One method of GPIO constants -----------
ANSELA = 0x5b
TRISA  = 0x92
LATA   = 0x89

#---- Optionally using a dictionary to hold the constants ----
ior = {   'ANSELA': 0x5b, 'ANSELB' : 0x5c ,'ANSELC' : 0x5d, 'ANSELD' : 0x5e, 'ANSELE' : 0x5f,
	  'PORTA' : 0x80, 'PORTB' :  0x81, 'PORTC'  : 0x82, 'PORTD'  : 0x83, 'PORTE'  : 0x84,
	  'TRISA' : 0x92, 'TRISB' :  0x93, 'TRISC'  : 0x94, 'TRISD'  : 0x95, 'TRISE'  : 0x96,
	  'LATA'  : 0x89, 'LATB'  :  0x8a, 'LATC'   : 0x8b, 'LATD'   : 0x8c, 'LATE'   : 0x8d }

#------------- GPIO code using constants -----------------
#sfr_set_regbit(usb, ANSELA, 6, 0)	# program RA6 for digital
#sfr_set_regbit(usb, TRISA,  6, 0)	# set IO direction as output
#sfr_set_regbit(usb, LATA,   6, 1)	# set RA6 to high

#time.sleep(5)
#------------- GPIO code using dictionary -----------------
#sfr_set_regbit(usb, ior['ANSELA'], 6, 0)
#sfr_set_regbit(usb, ior['TRISA'],  6, 0)
#sfr_set_regbit(usb, ior['LATA'],   6, 0)# set RA6 to low

#------------- GPIO setup input pin and get data from PORTA ----------
sfr_set_regbit(     usb, ior['ANSELA'], 6, 0)	# RA6 is digital
sfr_set_regbit(     usb, ior['TRISA'],  6, 1)	# RA6 is input
a = sfr_get_regbit( usb, ior['PORTA'],  6)	# get the value
print "Pin value:", a

#------------- New version demoing constants -------------------------
gpio = Bunch(ior)		# define the class GPIO containing our constants

sfr_set_regbit(     usb, gpio.ANSELA, 6, 0)	# RA6 is digital
sfr_set_regbit(     usb, gpio.TRISA,  6, 1)	# RA6 is input
a = sfr_get_regbit( usb, gpio.PORTA,  6)	# get the value
print "Pin value:", a

