#!/usr/bin/python
# Markham Thomas  2/27/2013
#
# SFR PWM demo program
# This program demonstrates using the SFR registers to 
# setup and control PWM output from the USB IO board
#
# NOTES:   carefully look at the pinout of the HK USB IO board and 
# locate the P1B, P1C, P1D outputs and what pins the are on
# example:  P1B is RD5
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
pwm = Bunch(pic_registers)
bit = Bunch(pic_bits)

#------------- PWM init code ------------
#		//- Duty Cycle Ratio = CCPR1L:CCP1CON<5:4> / 4 * (PR2 + 1)
#		//- 50 %   =        400          * 1/48,000,000 * 16
#		CCPR1L = 0x64; //- 50%
#		T2CON |= 0x06;   prescaler x16
def my_pwm_init(duty_cycle):
	sfr_set_regbit(usb, pwm.ANSELD, 5, 0)	# RD5 digital (P1B)
	sfr_set_regbit(usb, pwm.TRISD,  5, dir_input)	# RD5 is input for now
	sfr_set_reg(   usb, pwm.CCPTMRS, 0x00)	# select timer resource 
	#- PWM Period = [PR2 + 1] * 4 * TOSC * TMR2 Prescale Value
	#- 3,750 Hz   = (199 + 1) * 4 * 1/48,000,000 * 16
	#PR2 = 199;
	sfr_set_reg(   usb, pwm.PR2, 199)	# load with PWM period value
	# enable pwm mode bit(3-0), bit(5-4) LSB PWM duty cycle
	sfr_set_reg(   usb, pwm.CCP1CON, 0b00001100)
	sfr_set_reg(   usb, pwm.CCPR1L, 0x00)	# eight high bits of duty cycle
	# Timer2 on, 1:1 Post, 16x prescale
	sfr_set_reg(   usb, pwm.T2CON, 0b00000110)
	sfr_set_regbit(usb, pwm.PSTR1CON, 1, 1)	# enable STR1B to assign PxB
	sfr_set_regbit(usb, pwm.TRISD,  5, dir_output)	# RD5 enabled for output
	# LSB of PWM duty cycle
	#- Duty Cycle Ratio = CCPR1L:CCP1CON<5:4> / 4 * (PR2 + 1)
	#- 50 %   =        400          * 1/48,000,000 * 16
	#CCPR1L = 0x64; //- 50%
	sfr_set_reg(usb, pwm.CCPR1L, duty_cycle)

# initialize the registers needed for PWM output
# RD5 pin should measure 50% of 3.3 volts
my_pwm_init(0x64)		# 50% duty cycle @3.750khz
