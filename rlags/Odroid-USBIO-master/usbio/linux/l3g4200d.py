#!/usr/bin/python
# Markham Thomas  2/18/2013
# L3G4200D -- Gyroscope module 3-axis 
from hk_usb_io import *
from ctypes import *
import sys
import time
# note: output now in signed decimal instead of 2's complement
usb = init()			# init the USB IO board
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "---------- output -----------"
#====== L3G4200D digital gyro
L3G4200D_ADDRESS = 0x69		# i2c address of gyro if SDO not high
#L3G4200D_ADDRESS = 0x69	# i2c address of gyro if SDO to VDD
L3G4200D_CTRL_REG1 = 0x20
L3G4200D_CTRL_REG2 = 0x21
L3G4200D_CTRL_REG3 = 0x22
L3G4200D_CTRL_REG4 = 0x23
L3G4200D_CTRL_REG5 = 0x24
def writeRegister(dev_addr, addr, value):
	i2c_start(usb, I2C_START_CMD)
	i2c_write(usb, (dev_addr << 1 | I2C_WRITE_CMD))
	while (i2c_slave_ack(usb)):
		time.sleep(0.1)
	# select the register to write to 
	i2c_write(usb, addr)
	while (i2c_slave_ack(usb)):
		time.sleep(0.1)
	# write the value to the previously selected reg
	i2c_write(usb, value)
	while (i2c_slave_ack(usb)):
		time.sleep(0.1)
	i2c_stop(usb)
def readRegister(dev_addr, addr):
	i2c_start(usb, I2C_START_CMD)
	i2c_write(usb, (dev_addr << 1 | I2C_WRITE_CMD))
	while (i2c_slave_ack(usb)):	# wait for slave ack
		time.sleep(0.1)
	# select the register address to read
	i2c_write(usb, addr)
	while (i2c_slave_ack(usb)):	# wait for slave ack
		time.sleep(0.1)
	i2c_start(usb, I2C_REP_START_CMD)
	i2c_write(usb, (dev_addr << 1 | I2C_READ_CMD))
	# read the value from the register
	while (i2c_slave_ack(usb)):	# wait for slave ack
		time.sleep(0.1)
	a = i2c_read(usb)
	# ack the byte
	i2c_master_ack(usb, I2C_DATA_NOACK)
	i2c_stop(usb)
	return a
def setupL3G4200D(scale):
	# enable x,y,z and turn off power down
	writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG1, 0b00001111)
	# to adjust/use HPF, edit next line to configure CTRL_REG2
	writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG2, 0b00000000)
	# consult datasheet to determine if INT on INT1,INT2, or INT3
	writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG3, 0b00001000)
	# REG4 also controls full-scale range
	# orig: 0b00000000 change to bit7=1 to wait on reading
	# appears to fix a random 0xff returned (BDU 8th bit 1, block data update)
	# enabled BDU on all 4 speed settings (bit7=1)
	if   (scale == 250):
		writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG4, 0b10000000)
	elif (scale == 500):
		writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG4, 0b10010000)
	else:
		writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG4, 0b10110000)
	# REG5 controls high-pass filtering of outputs, enable if needed
	writeRegister(L3G4200D_ADDRESS, L3G4200D_CTRL_REG5, 0b00000000)
def get_gyro():
        a = array('i',[0,0,0])
	xLSB = readRegister(L3G4200D_ADDRESS, 0x28)
	xMSB = readRegister(L3G4200D_ADDRESS, 0x29)
	#print hex(xMSB), hex(xLSB)
	x = c_short(( xMSB << 8 ) | xLSB)
	yLSB = readRegister(L3G4200D_ADDRESS, 0x2A)
	yMSB = readRegister(L3G4200D_ADDRESS, 0x2B)
	#print hex(yMSB), hex(yLSB)
	y = c_short(( yMSB << 8 ) | yLSB)
	zLSB = readRegister(L3G4200D_ADDRESS, 0x2C)
	zMSB = readRegister(L3G4200D_ADDRESS, 0x2D)
	#print hex(zMSB), hex(zLSB)
	z = c_short(( zMSB << 8 ) | zLSB)
	#print x.value,y.value,z.value
	a[0] = x.value
	a[1] = y.value
	a[2] = z.value
	return a

# start i2c
i2c_init(usb)
# configure sensor - 250, 500 or 2000 deg/sec
setupL3G4200D(2000)	# setup the sensor
time.sleep(1.5)		# wait for sensor to become ready
# delay for sensor ready
for n in range (1, 60):
	time.sleep(0.2)
	a = get_gyro()
	print "x=%6d, y=%6d, z=%6d" % (a[0], a[1], a[2])
