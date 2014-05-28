#!/usr/bin/python
# Markham Thomas  3/07/2013
#
# SFR PWM output driving a RGB LED
# RGB LED:  Radio Shack 276-0028
# LED info:  flat side to your left: (led on top, pins down)
#  - RED cathode   = medium pin       (leftmost pin)
#  + Common Anode  = longest pin      (next from left)
#  - BLUE cathode  = inside short pin (3rd from left)
#  - GREEN cathode = outer short pin  (4th from left)
#
# Notes:  A resistor sized for each RGB lead should be used
# instead just one is used.  PWM driving each RGB pin should 
# also be used, instead just one is used.
# ----- wiring
# RE0 - RED,  RE1 - BLUE,  RE2 - GREEN,  RD5 - Common Anode
# PWM output:  P1B is RD5    RD5 <-- 120ohm resistor --> Common Anode
# 
# common anode means LED is on when output is LOW on RE0,RE1, or RE2
#
# NOTE: without separate tuned resistors for each color RED will dominate 
# More PWM outputs would be needed to drive each leg separately
from hk_usb_io import *
from sfr_constants import *
import sys
import time
import random

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
def my_gpio_init():
	sfr_set_regbit(usb, pwm.ANSELE, 0, 0)	# RE0 is digital
	sfr_set_regbit(usb, pwm.ANSELE, 1, 0)	# RE1 is digital
	sfr_set_regbit(usb, pwm.ANSELE, 2, 0)	# RE2 is digital
	sfr_set_regbit(usb, pwm.TRISE,  0, 0)	# RED   output
	sfr_set_regbit(usb, pwm.TRISE,  1, 0)	# BLUE  output
	sfr_set_regbit(usb, pwm.TRISE,  2, 0)	# GREEN output
def random_toggle():
	x = random.random()	# random float  0.0 <= x < 1.0	
	if (x >= 0.8):		# 20% of the time be a 1
		return 1
	else:
		return 0
def random_str():
	x = random.randint(0x40, 0xc0)
	return x

# initialize the registers needed for PWM output
my_pwm_init(0xc0)		# 100% duty cycle @3.750khz
my_gpio_init()
random.seed()
for n in range (0, 255):
	r = random_toggle()
	b = random_toggle()
	g = random_toggle()
	print "r-b-g: ", r,b,g
	sfr_set_regbit(usb, pwm.LATE, 0, r)	# red
	sfr_set_regbit(usb, pwm.LATE, 1, b)	# blue
	sfr_set_regbit(usb, pwm.LATE, 2, g)	# green
	# random brightness
	sfr_set_reg(usb, pwm.CCPR1L, random_str())
	time.sleep(1)
