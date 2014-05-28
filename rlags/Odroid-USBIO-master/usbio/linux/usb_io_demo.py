#!/usr/bin/python
# Markham Thomas  2/24/2013
#
# Demo some of the HK_USB_IO ROM functions
#
#   Refer to the other python programs for more
# in depth examples
from hk_usb_io import *		# import interface module
import sys
import time

usb = init()			# init the USB IO board
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "---------- output -----------"

toggle_led(usb)			# toggle the LED

#-- pushbutton on board status --
a = read_switch(usb)		# read the switch status
print a

#-- gpio in --
gpio_init(usb,rd7,dir_input)	# configure gpio RD7 as input
a = gpio_in(usb,rd7)		# read the GPIO pin RD7
print a

#-- analog in --
a = adc_ra1(usb)		# do ADC conversion on pin RA1
print a

#------------- serial port (UART) functionality ----------
# LCD on serial port (UART IO to a serial attached LCD)
ser_putc(usb,chr(0xfe))		# clear LCD screen
ser_putc(usb,chr(0x01))	
ser_putc(usb,chr(0xfe))		# block cursor
ser_putc(usb,chr(0x0d))
ser_puts(usb,"Hello World")
ser_puts(usb,chr(0xfe) + chr(192))	# move to next line
ser_puts(usb,"From Odroid-x2")

if (ser_test(usb)):		# check if incoming char on UART
	a = ser_getc(usb)
	print a
else:
	print "no"

#------------- Special Function Registers ---------------
# SFR register access:  refer to page 88-95 of SPEC doc
# where ANSELB register is: 0xf5c  (only use lower byte to access)
ANSELB = 0x5c
sfr_set_regbit(usb, ANSELB, 1, 0)	# RB1 is digital

# where 0xfc9 is SS1BUF serial port buffer
a = sfr_get_reg(usb, 0xc9)	# SS1BUF register on PIC18
print a
