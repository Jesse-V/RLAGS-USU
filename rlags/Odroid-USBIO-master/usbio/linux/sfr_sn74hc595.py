#!/usr/bin/python
# Markham Thomas  3/05/2013
#
# SFR control of GPIO to drive a shift register
# SN74HC595 8-bit shift register demo output to 7-segment display
#  - only supports 8-bits, modify to chain 595's together
#  - 1st shift register pin 9 (QH') goes to 2nd shift reg pin 14 (SER)
#------- wiring
# RD4 <--> SER,  RD5 <--> RCLK,  RD6 <--> SRCLK,  RD7 <--> OE
# (74HC595) = VCC(16), SER(14), OE(13), RCLK(12), SRCLK(11), GND(8)
#    "" SRCLR(10) = tied high unless need to clear all registers
#
#-------- 7-segment
# A(7)=T, B(6)=TR, C(4)=BR, D(2)=B, E(1)=BL, F(9)=TL, DP(5), G(10)=middle
# pin order from TOP: 10,9,8,7,6  BOTTOM: 1,2,3,4,5
# Common Anodes are: 3 and 8   (tied to +VDD )
# 1k resistors in series on each output of 74HC595
#-------- mapping
# 74HC595 15->A(7), 1->B(6), 2->C(4), 3->D(2), 4->E(1), 5->F(9), 6->G(10) 7->DP(5)
#    A
#    -
#  F| |B 		bit: 7654 3210 
#    -   G		MSB: HGFE DCBA
#  E| |C
#    -   . H
#    D
from hk_usb_io import *
from sfr_constants import *
import sys
import time

#-------------- USB init required --------
usb = init()			# init the USB IO board
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "----------- output ------------"

io = Bunch(pic_registers)	# define the class GPIO containing our constants

DINP = 1
DOUT = 0
LOW  = 0
HIGH = 1
TRUE = 1
FALSE = 0

# array to hold shift register pin states
pin_states = array('B',[0,0,0,0,0,0,0,0])
# this defines the segments to turn on for each number from 0 to F
# for B D the decimal point is turned on also
hextable = array('B',[0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
		      0x7f,0x67,0x77,0xff,0x39,0xbf,0x79,0x71])

def setup_pins():		# configure initial pin functions and states
	sfr_set_regbit(usb, io.ANSELD, 4, DOUT)	# RD4 is digital
	sfr_set_regbit(usb, io.TRISD,  4, DOUT)
	sfr_set_regbit(usb, io.ANSELD, 5, DOUT)	# RD5 is digital
	sfr_set_regbit(usb, io.TRISD,  5, DOUT)
	sfr_set_regbit(usb, io.ANSELD, 6, DOUT)	# RD6 is digital
	sfr_set_regbit(usb, io.TRISD,  6, DOUT)
	sfr_set_regbit(usb, io.ANSELD, 7, DOUT)	# RD7 is digital
	sfr_set_regbit(usb, io.TRISD,  7, DOUT)	# OE (low enabled)
	sfr_set_regbit(usb, io.LATD,   4, 0)	# outputs low
	sfr_set_regbit(usb, io.LATD,   5, 0)	#   ""
	sfr_set_regbit(usb, io.LATD,   6, 0)	#   "" 
	sfr_set_regbit(usb, io.LATD,   7, 1)	# disable output 
def output_enable(enable):
	if (enable):
		sfr_set_regbit(usb, io.LATD,   7, 0)	# enable output 
	else:
		sfr_set_regbit(usb, io.LATD,   7, 1)	# disable output 
def set_pin_state(arr, pin, state):
	arr[pin] = state
def pin_srclk(enable):
	if (enable):
		sfr_set_regbit(usb, io.LATD,   6, 1)	# RD6
	else:
		sfr_set_regbit(usb, io.LATD,   6, 0)	# RD6
def pin_rclk(enable):
	if (enable):
		sfr_set_regbit(usb, io.LATD,   5, 1)	# RD5
	else:
		sfr_set_regbit(usb, io.LATD,   5, 0)	# RD5
def pin_ser(enable):
	if (enable):	# Note: common Anode so logic is reversed
		sfr_set_regbit(usb, io.LATD,   4, 0)	# RD4
	else:		#  if common Cathode switch logic back
		sfr_set_regbit(usb, io.LATD,   4, 1)	# RD4
def clear_pins():
	for n in range (0,8):
		pin_states[n] = 0
def write_states(state_array):		# write the array out
	pin_rclk(LOW)
	for n in range (7,-1,-1):	# reverse it so A=A, B=B
		pin_srclk(LOW)
		pin_ser(state_array[n])	# set the values
		pin_srclk(HIGH)	# values shifted to the right
	pin_rclk(HIGH)		# update output pins with new data
def write_hex(value):		# determine bits to set on for char
	a = 1			# will be shifting this bit for tests
	for n in range (0,8):	# set bits in array if bit is 1
		if (hextable[value] & a):
			set_pin_state(pin_states, n, 1)
		else:	
			set_pin_state(pin_states, n, 0)
		a = a << 1	# test next bit
	write_states(pin_states)# write bits to the shift-register

#----------------------
setup_pins()			# configure our GPIO pins
output_enable(TRUE)		# enable shift register output
for n in range (0,16):		# step through each character 0-F
	write_hex(n)		# write each char
	time.sleep(1)
