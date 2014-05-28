#!/usr/bin/python
#
# Markham Thomas  2/17/2013
#
# todo:  apply correction coefficents
#   and compute proper temperature value

from hk_usb_io import *
import sys
import time

usb = init()			# connect to the USB IO board

#==== print version info ====
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "---------- output -----------"

#====== MPL115A2 pressure/temp sensor
MPL115A2_ADDRESS = 0x60
MPL115A2_REGISTER_PRESSURE_MSB = 0x00
MPL115A2_REGISTER_PRESSURE_LSB = 0x01
MPL115A2_REGISTER_TEMP_MSB = 0x02
MPL115A2_REGISTER_TEMP_LSB = 0x03
MPL115A2_REGISTER_A0_COEFF_MSB = 0x04
MPL115A2_REGISTER_A0_COEFF_LSB = 0x05
MPL115A2_REGISTER_B1_COEFF_MSB = 0x06
MPL115A2_REGISTER_B1_COEFF_LSB = 0x07
MPL115A2_REGISTER_B2_COEFF_MSB = 0x08
MPL115A2_REGISTER_B2_COEFF_LSB = 0x09
MPL115A2_REGISTER_C12_COEFF_MSB = 0x0A
MPL115A2_REGISTER_C12_COEFF_LSB = 0x0B
MPL115A2_REGISTER_STARTCONVERSION = 0x12
#=== sample sequence of I2C read
# read coeff: [0xC0], [0x04], [0xC1], [0x3E], [0xCE],
#     [0xB3], [0xF9], [0xC5], [0x17], [0x33], [0xC8]
#=== start i2c
i2c_init(usb)
# start i2c transmission
i2c_start(usb, I2C_START_CMD)
# write MPL115A2 control byte
i2c_write(usb, (MPL115A2_ADDRESS << 1) | I2C_WRITE_CMD)
# coeff request
i2c_write(usb, MPL115A2_REGISTER_A0_COEFF_MSB)
# restart i2c_transmission
i2c_start(usb, I2C_REP_START_CMD)
# get some data
i2c_write(usb, (MPL115A2_ADDRESS << 1) |I2C_READ_CMD)
coeff = array('i',[0,0,0,0,0,0,0,0,0,0,0,0,0,0])
for x in range (1,5):
	dataMSB = i2c_read(usb)
	i2c_master_ack(usb, I2C_DATA_ACK)
	dataLSB = i2c_read(usb)
	i2c_master_ack(usb, I2C_DATA_ACK)
	coeff[x] = dataMSB << 8 | dataLSB
test = i2c_read(usb)	# PIC usb stops if we don't read one more byte
print "Hex raw coeff: " 
print hex(coeff[1]), hex(coeff[2]), hex(coeff[3]), hex(coeff[4])
mpl_a0 = float(coeff[1]) / 8
mpl_b1 = float(coeff[2]) / 8182
mpl_b2 = float(coeff[3]) / 16384
mpl_c12 = float(coeff[4])
mpl_c12 /= 4194304.0
if (coeff[1] > 0x7fff):
	mpl_a0 *= -1
if (coeff[2] > 0x7fff):
	mpl_b1 *= -1
if (coeff[3] > 0x7fff):
	mpl_b2 *= -1
if (coeff[4] > 0x7fff):
	mpl_c12 *= -1
print "processed coefficients are:"
print mpl_a0, mpl_b1, mpl_b2, mpl_c12
i2c_start(usb, I2C_START_CMD)
i2c_write(usb, (MPL115A2_ADDRESS << 1) | I2C_WRITE_CMD)
i2c_write(usb, MPL115A2_REGISTER_STARTCONVERSION) 
i2c_write(usb, MPL115A2_REGISTER_PRESSURE_MSB) 
time.sleep(1)	# delay (only needs 5ms but this will do for now)
i2c_start(usb, I2C_START_CMD)
i2c_write(usb, (MPL115A2_ADDRESS << 1) | I2C_WRITE_CMD)
i2c_write(usb, MPL115A2_REGISTER_PRESSURE_MSB) 
i2c_start(usb, I2C_REP_START_CMD)
i2c_write(usb, (MPL115A2_ADDRESS << 1) |I2C_READ_CMD)

pressMSB = i2c_read(usb) 
i2c_master_ack(usb, I2C_DATA_ACK)
pressLSB = i2c_read(usb) 
i2c_master_ack(usb, I2C_DATA_ACK)
tempMSB  = i2c_read(usb) 
i2c_master_ack(usb, I2C_DATA_ACK)
tempLSB  = i2c_read(usb) 

press = pressMSB << 8 | pressLSB >> 6
temp  = tempMSB << 8 | tempLSB >> 6
print "temperature: %X, %X, MSB|LSB: %X" %(tempMSB, tempLSB, temp)
print "   pressure: %X, %X, MSB|LSB: %X" %(pressMSB, pressLSB, press)
# Below is from arduino sketch, not working yet
rtemp = (float(temp) - 498.0) / -5.35 + 25.0
print "Bogus temp value for now: %f" % (rtemp)

i2c_master_ack(usb, I2C_DATA_NOACK)
i2c_stop(usb)


close(usb)
