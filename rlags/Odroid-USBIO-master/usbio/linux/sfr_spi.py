#!/usr/bin/python
# Markham Thomas  2/25/2013
#
# SFR SPI test program
# This program demonstrates using the SFR registers to 
# setup and communicate via SPI as a MASTER
#
# 	Note:  The demo ROM does implement its own SPI, 
# but this demo shows SPI control just through the SFR registers
# It also uses sfr_constants to get the PIC constants to use

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
spi = Bunch(pic_registers)
bit = Bunch(pic_bits)

SPI_BAUD  = 0b00000010		# 750khz
SPI_SMP_E = 0b10000000		# SMP sample at end
SPI_BF	  = 0			# bit 0 of SSP1STAT is BF flag
SPI_CS	  = 5			# RA5 is going to be CS (chip select)
SS_LOW    = 0
SS_HIGH	  = 1
#------------- SPI init code ------------
def my_spi_init():
	sfr_set_regbit(usb, spi.ANSELB, 1, 0)	# RB1 is digital
	sfr_set_regbit(usb, spi.ANSELB, 0, 0)	# RB0 is digital
	sfr_set_regbit(usb, spi.ANSELA, 5, 0)	# RA5 is digital
	sfr_set_reg(   usb, spi.SSP1CON1, 0x00)	# reset serial
	sfr_set_regbit(usb, spi.TRISC,  7, 0)	# RC7=SDO (set as output)
	sfr_set_regbit(usb, spi.TRISA,  5, 0)	# RA5=SS  (set as output)
	sfr_set_regbit(usb, spi.TRISB,  0, 1)	# RB0=SDI (set as input)
	sfr_set_regbit(usb, spi.TRISB,  1, 0)	# RB1=SCL (set as output)
	sfr_set_reg(   usb, spi.SSP1STAT, 0x00)	# mode3 CKE=0
	sfr_set_reg(   usb, spi.SSP1CON1, 0x10)	# mode3 CKP=1
	sfr_set_regbit(usb, spi.SSP1CON1, 1, 1)	# baud is 750khz (ref: SPI_BAUD)
	sfr_set_regbit(usb, spi.SSP1STAT, 7, 1)	# Sample at end of clock (ref: SPI_SMP_E)
	sfr_set_regbit(usb, spi.LATA, 5, 1)	# set RA5 to high (SS=disable)
	sfr_set_regbit(usb, spi.SSP1CON1, 5, 1)	# enable serial sync (bit5: SSPEN=1)
def my_spi_transfer(value):
	sfr_set_reg(   usb, spi.SSP1BUF, value)	# write byte to SPI shift reg
	while not (sfr_get_regbit(usb, spi.SSP1STAT, bit.SSP1STAT_BF)):
		time.sleep(0.05)
	return (sfr_get_reg(usb, spi.SSP1BUF))
def my_spi_cs(value):
	if (value == SS_LOW):
		sfr_set_regbit(usb, spi.LATA, 5, 0)	# SS goes low (SS=enable)
	else:
		sfr_set_regbit(usb, spi.LATA, 5, 1)	# SS goes high (SS=disable)
	time.sleep(0.05)	# modest sleep to give SS time to respond

# using my l3g4200d gyroscope to test the WHOAMI register
my_spi_init()

my_spi_cs(SS_LOW)				# enable chip select

# select the register from the gyroscope
sfr_set_reg(usb, spi.SSP1BUF, 0x0f | 0x80)	# 0x0f is whoami,  0x80 says spi read

# wait for the bits to finish being sent
while not (sfr_get_regbit(usb, spi.SSP1STAT, bit.SSP1STAT_BF)):
	time.sleep(0.05)

# dummy write to shift data in from the gyro
sfr_set_reg(usb, spi.SSP1BUF, 0x00)		# dummy write to shift in response
while not (sfr_get_regbit(usb, spi.SSP1STAT, bit.SSP1STAT_BF)):
	time.sleep(0.05)

# get the data from the gyro's register selected earlier
a = sfr_get_reg(usb, spi.SSP1BUF)		# get value from gyroscope

my_spi_cs(SS_HIGH)				# disable CHIP (SS=HIGH)

print "Reponse should be 0xd3, value is: ", hex(a)
