#!/usr/bin/python
# Markham Thomas  2/26/2013
# SPI mode for L3GD20-- 3-axis Gyroscope module Pololu
# Notes:  RC7=SDO, RB0=SDI, RB1=SCLK, RB5=CS
#  you can ignore return from spi_transfer until you expect data
# Notes:  L3GD20 is an update over the L3G4200D
from hk_usb_io import *
from ctypes import *
import sys
import time
usb = init()			# init the USB IO board
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "---------- output -----------"
#====== SPI mode for L3GD20 digital gyro
L3GD20_CTRL_REG1 = 0x20
L3GD20_CTRL_REG2 = 0x21
L3GD20_CTRL_REG3 = 0x22
L3GD20_CTRL_REG4 = 0x23
L3GD20_CTRL_REG5 = 0x24
L3GD20_WHOAMIREG = 0x0f
L3GD20_REFERENCE = 0x25
L3GD20_OUTTEMP   = 0x26
L3GD20_STATUSREG = 0x27
L3GD20_OUTXL	 = 0x28
L3GD20_OUTXH	 = 0x29
L3GD20_OUTYL	 = 0x2A
L3GD20_OUTYH	 = 0x2B
L3GD20_OUTZL	 = 0x2C
L3GD20_OUTZH	 = 0x2D

def writeRegister(addr, value):
	spi_cs(usb, SPI_CS_ENABLE)	# RB5 goes low
	time.sleep(0.05)		# sleep time for CS seems critical
	spi_transfer(usb, addr & 0x7f )
	spi_transfer(usb, value)
	spi_cs(usb, SPI_CS_DISABLE)
	time.sleep(0.05)		# sleep time for CS to settle
def readRegister(addr):
	spi_cs(usb, SPI_CS_ENABLE)
	time.sleep(0.05)
	spi_transfer(usb, addr | 0x80 ) # requesting a read from address
	a = spi_transfer(usb, 0x00)	# dummy write new data shifts in
	spi_cs(usb, SPI_CS_DISABLE)	# CS off (RB5 goes high)
	time.sleep(0.05)
	return a
def setupL3GD20(scale):
	spi_init(usb, SPI_MODE3, SPI_LOW_BAUD, SPI_SAMP_END)
	time.sleep(0.1)
	# WHOAMI reg should read 0xd4 on L3GD20
	if (readRegister(L3GD20_WHOAMIREG) != 0xd4):
		print "No whoami response from L3GD20 gyroscope!"
		sys.exit()
	# get value from outtemp register (note: chip temp)
	a = readRegister(L3GD20_OUTTEMP)
	print "Chip temp", hex(a)
	# enable x,y,z and turn off power down
	#writeRegister(L3GD20_CTRL_REG1, 0b00001111)
	# testing ODR(hz)=760 cut-off=100
	writeRegister(L3GD20_CTRL_REG1, 0b01111111)
	# to adjust/use HPF, edit next line to configure CTRL_REG2
	writeRegister(L3GD20_CTRL_REG2, 0b00000000)
	# consult datasheet to determine if INT on INT1,INT2, or INT3
	writeRegister(L3GD20_CTRL_REG3, 0b00001000)
	# REG4 also controls full-scale range
	# orig: 0b00000000 change to bit7=1 to wait on reading
	# appears to fix a random 0xff returned (BDU 8th bit 1, block data update)
	# enabled BDU on all 4 speed settings (bit7=1)
	if   (scale == 250):
		writeRegister(L3GD20_CTRL_REG4, 0b10000000)
	elif (scale == 500):
		writeRegister(L3GD20_CTRL_REG4, 0b10010000)
	else:
		writeRegister(L3GD20_CTRL_REG4, 0b10110000)
	# REG5 controls high-pass filtering of outputs, enable if needed
	writeRegister(L3GD20_CTRL_REG5, 0b00000000)
def get_gyro():
        a = array('i',[0,0,0])
	xLSB = readRegister(L3GD20_OUTXL)
	xMSB = readRegister(L3GD20_OUTXH)
	#print hex(xMSB), hex(xLSB)
	x = c_short(( xMSB << 8 ) | xLSB)
	yLSB = readRegister(L3GD20_OUTYL)
	yMSB = readRegister(L3GD20_OUTYH)
	#print hex(yMSB), hex(yLSB)
	y = c_short(( yMSB << 8 ) | yLSB)
	zLSB = readRegister(L3GD20_OUTZL)
	zMSB = readRegister(L3GD20_OUTZH)
	#print hex(zMSB), hex(zLSB)
	z = c_short(( zMSB << 8 ) | zLSB)
	#print x.value,y.value,z.value
	a[0] = x.value
	a[1] = y.value
	a[2] = z.value
	return a

# configure sensor - 250, 500 or 2000 deg/sec
setupL3GD20(250)	# setup the sensor
time.sleep(1.5)		# wait for sensor to become ready
# delay for sensor ready
for n in range (1, 60):
	time.sleep(0.1)
	a = get_gyro()
	print "x=%6d, y=%6d, z=%6d" % (a[0], a[1], a[2])
