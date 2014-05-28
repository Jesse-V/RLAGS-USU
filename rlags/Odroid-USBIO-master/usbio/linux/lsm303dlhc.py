#!/usr/bin/python
# Markham Thomas  2/26/2013
# LSM303 -- 3d Compass and Accelerometer module 3-axis 
# I2C communication to LSM303DLHC - from Pololu
from hk_usb_io import *
from ctypes import *
import sys
import time
usb = init()			# init the USB IO board
print module_version()		# print python module version
print rom_version(usb)		# print rom version
print "---------- output -----------"
#====== LSM303 digital accel and compass
lsm = { 
'LSM303_ADDR_MAG':0x1e,		# i2c address of mag sensor
'LSM303_ADDR_ACC':0x19,		# i2c address of accel sensor
'LSM303_CTRL_REG1':0x20,'LSM303_CTRL_REG2':0x21,'LSM303_CTRL_REG3':0x22,'LSM303_CTRL_REG4':0x23,
'LSM303_CTRL_REG5':0x24,'LSM303_CTRL_REG6':0x25,'REFERENCE_A':0x26,'STATUS_REG_A':0x27,
'OUT_X_L_A':0x28,'OUT_X_H_A':0x29,'OUT_Y_L_A':0x2a,'OUT_Y_H_A':0x2b,'OUT_Z_L_A':0x2c,
'OUT_Z_H_A':0x2d,'OUT_X_H_M':0x03,'OUT_X_L_M':0x04,'OUT_Z_H_M':0x05,'OUT_Z_L_M':0x06,
'OUT_Y_H_M':0x07,'OUT_Y_L_M':0x08,'TEMP_OUT_H_M':0x31,'TEMP_OUT_L_M':0x32,
'CRA_REG_M':0x00,'CRB_REG_M':0x01,'MR_REG_M':0x02,}

acc = Bunch(lsm)

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
def setupLSM303():
	# enable x,y,z and turn off power down, 10Hz data rate
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG1, 0b00100111)
	# to adjust/use HPF, edit next line to configure CTRL_REG2
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG2, 0b00000000)
	# consult datasheet to determine if INT on INT1,INT2, or INT3
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG3, 0b00000000)
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG4, 0b00000000)
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG5, 0b00000000)
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG6, 0b00000000)
	writeRegister(acc.LSM303_ADDR_ACC, acc.LSM303_CTRL_REG6, 0b00000000)
	# enable temp and 3Hz data output rate for Mag sensor
	writeRegister(acc.LSM303_ADDR_MAG, acc.CRA_REG_M, 0b10001000)
	# mag gain config bits
	writeRegister(acc.LSM303_ADDR_MAG, acc.CRB_REG_M, 0b00000000)
	# continous conversion mode
	writeRegister(acc.LSM303_ADDR_MAG, acc.MR_REG_M,  0b00000000)
def get_data_acc():
        a = array('i',[0,0,0])
	xLSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_X_L_A)
	xMSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_X_H_A)
	#print hex(xMSB), hex(xLSB)
	x = c_short(( xMSB << 8 ) | xLSB)
	yLSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_Y_L_A)
	yMSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_Y_H_A)
	#print hex(yMSB), hex(yLSB)
	y = c_short(( yMSB << 8 ) | yLSB)
	zLSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_Z_L_A)
	zMSB = readRegister(acc.LSM303_ADDR_ACC, acc.OUT_Z_H_A)
	#print hex(zMSB), hex(zLSB)
	z = c_short(( zMSB << 8 ) | zLSB)
	#print x.value,y.value,z.value
	a[0] = x.value
	a[1] = y.value
	a[2] = z.value
	return a
def get_data_mag():
        a = array('i',[0,0,0])
	xLSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_X_L_M)
	xMSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_X_H_M)
	x = c_short(( xMSB << 8 ) | xLSB)
	zLSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_Z_L_M)
	zMSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_Z_H_M)
	z = c_short(( zMSB << 8 ) | zLSB)
	yLSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_Y_L_M)
	yMSB = readRegister(acc.LSM303_ADDR_MAG, acc.OUT_Y_H_M)
	y = c_short(( yMSB << 8 ) | yLSB)
	#print x.value,y.value,z.value
	a[0] = x.value
	a[1] = y.value
	a[2] = z.value
	return a

# start i2c
i2c_init(usb)
setupLSM303()		# setup the sensor
time.sleep(1.5)		# wait for sensor to become ready
# delay for sensor ready
for n in range (1, 60):
	time.sleep(1)
	a = get_data_acc()
	b = get_data_mag()
	print "Accel: x=%6d, y=%6d, z=%6d -- Mag: x=%6d, y=%6d, z=%6d" % (a[0], a[1], a[2], b[0], b[1], b[2])
