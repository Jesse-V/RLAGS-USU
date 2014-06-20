/******************************************************************************/
/*                         USB INTERFACE ROUTINES                             */
/*                                                                            */
/* All the routines for interfacing with the USB subsystem are contained in   */
/* this module.                                                               */
/*                                                                            */
/* Copyright (C) 2012 - 2014  Edward Simonson                                 */
/*                                                                            */
/* This file is part of GoQat.                                                */
/*                                                                            */
/* GoQat is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 3 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, see <http://www.gnu.org/licenses/> .      */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBUSB

#include <pthread.h>

#include "gqusb.h"

#define TRUE  1
#define FALSE 0

#define GQUSB_BULK_TIMEOUT 1000

static struct libusb_context *gqctx;
static struct libusb_device **devices;

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void gqusb_init (void);
void gqusb_exit (void);
int gqusb_list_devices (int vendor, int maxdev, int pids[], 
                        struct libusb_device *devs[]);
int gqusb_open_device (struct usbdevice *u_dev);
void gqusb_close_device (struct usbdevice *u_dev);
int gqusb_bulk_read (struct usbdevice *u_dev, unsigned char *data, int length);
int gqusb_bulk_write (struct usbdevice *u_dev, unsigned char *data, int length);
int gqusb_bulk_write_lock (struct usbdevice *u_dev, unsigned char *data, 
						   int length);
int gqusb_bulk_write_unlock (struct usbdevice *u_dev, unsigned char *data, 
							 int length);
int gqusb_bulk_io (struct usbdevice *u_dev, unsigned char *wdata, int wlength,
										    unsigned char *rdata, int rlength);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void gqusb_init (void)
{
	/* Initialise libusb */
	
	libusb_init (&gqctx);
    libusb_set_debug (gqctx, 3);
}

void gqusb_exit (void)
{
	/* Close libusb */
	
	libusb_exit (gqctx);
}

int gqusb_list_devices (int vendor, int maxdev, int pids[],
                        struct libusb_device *devs[])
{
	/* Create a list of product id's for all devices that match the given
	 * vendor id and return the number of such devices found, up to a maximum of
	 * 'maxdev' devices.  Also store pointers to each device.
	 */
	 
	struct libusb_device_descriptor dev_desc;
	ssize_t num;
	int i, j = 0;
	
	num = libusb_get_device_list (gqctx, &devices);
	if (num > 0) {
		for (i = 0; i < num; i++) {
			libusb_get_device_descriptor (devices[i], &dev_desc);
			if (dev_desc.idVendor == vendor) {
				pids[j] = dev_desc.idProduct;
				devs[j++] = devices[i];
				if (j == maxdev)
					break;
			}
		}
	} else
		j = -1;
		
	return j;
}

int gqusb_open_device (struct usbdevice *u_dev)
{
	/* Open the usb device.  Assumes one configuration, interface and altsetting
	 * as is appropriate for Starlight Xpress cameras and throws an error if
	 * more are found.
	 */
	
	struct libusb_device_descriptor dev_desc;
	struct libusb_config_descriptor *conf_desc;
	const struct libusb_interface *intf;
	int i, err;
	unsigned char end_addr;
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	libusb_get_device_descriptor (u_dev->dev, &dev_desc);
	if (dev_desc.bNumConfigurations > 1) {
		u_dev->err = "Found more than one configuration for this device";
		return FALSE;
	}
	libusb_get_config_descriptor (u_dev->dev, 0, &conf_desc);
	if (conf_desc[0].bNumInterfaces > 1) {
		u_dev->err = "Found more than one interface for this configuration";
		return FALSE;
	}
	intf = &conf_desc[0].interface[0];
	u_dev->intf_num = intf->altsetting[0].bInterfaceNumber;
	if (intf->num_altsetting > 1) {
		u_dev->err = "Found more than one altsetting for this interface";
		return FALSE;
	}
	if (intf->altsetting[0].bNumEndpoints > 2) {
		u_dev->err = "Found more than two endpoints for this interface";
		return FALSE;
	}

	/* Get endpoints associated with this interface */
	
	u_dev->input.num = 0;
	u_dev->output.num = 0;
	for (i = 0; i < intf->altsetting[0].bNumEndpoints; i++) {
		end_addr = intf->altsetting[0].endpoint[i].bEndpointAddress;
		if ((end_addr & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
			u_dev->input.address = end_addr;
			u_dev->input.num++;
		}
		if ((end_addr & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
			u_dev->output.address = end_addr;
			u_dev->output.num++;
		}
	}
	
	/* Now open the device */
	
	if ((err = libusb_open (u_dev->dev, &u_dev->handle)) == 0) {
		
		/* If the kernel has automatically attached a driver to this interface,
		 * detach it so that we can claim the interface.
		 */
		 
		if (libusb_kernel_driver_active (u_dev->handle, u_dev->intf_num) == 1)
			if (!libusb_detach_kernel_driver (u_dev->handle, u_dev->intf_num)) {
				libusb_close (u_dev->handle);
				u_dev->err = "Unable to claim USB device interface";
				return FALSE;
			}
			
		 /* Finally, claim the interface */
		 
		if (libusb_claim_interface (u_dev->handle, u_dev->intf_num)) {
			libusb_close (u_dev->handle);
			u_dev->err = "Unable to claim USB device interface";
			return FALSE;
		}
	} else {
		if (err == LIBUSB_ERROR_ACCESS) {
			u_dev->err = "Insufficient user permissions to open device";
			return FALSE;
		} else if (err == LIBUSB_ERROR_NO_DEVICE) {
			u_dev->err = "Device has been disconnected";
			return FALSE;
		} else {
			u_dev->err = "Error opening device";
			return FALSE;
		}
	}
	
	libusb_free_config_descriptor (conf_desc);
	libusb_free_device_list (devices, 1);
	
	/* Initialise IO mutex */
	
	if (pthread_mutex_init (&u_dev->io_mutex, NULL)) {
		u_dev->err = "Unable to initialise IO mutex";
		return FALSE;
	}

	return TRUE;
}

void gqusb_close_device (struct usbdevice *u_dev)
{
	/* Close the usb device */
	
	libusb_release_interface (u_dev->handle, u_dev->intf_num);
	libusb_close (u_dev->handle);
	
	pthread_mutex_destroy (&u_dev->io_mutex);
}

int gqusb_bulk_read (struct usbdevice *u_dev, unsigned char *data, int length)
{
	/* Do a bulk read from the usb device.  libusb_close can hang if a bulk
	 * read is cancelled part way through, so prevent the calling thread from
	 * doing that.
	 */
	
	int err, tr, bytes, last_state;
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	if (!u_dev->input.num) {
		u_dev->err = "No input endpoint!";
		return FALSE;
	}
	
	bytes = 0;
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &last_state);
	while ((err = libusb_bulk_transfer (u_dev->handle,
										u_dev->input.address,
										data+bytes,
										length - bytes, 
										&tr,
										GQUSB_BULK_TIMEOUT)) ==
										LIBUSB_ERROR_TIMEOUT) {
		bytes+= tr;
	}
	pthread_setcancelstate (last_state, NULL);
	
	if (err != 0) {
		u_dev->err = "Error reading from device";
		return FALSE;
	}
	if (tr < length) {
		u_dev->err = "Insufficient data returned from device";
		return FALSE;
	}
	
	return TRUE;	
}

int gqusb_bulk_write (struct usbdevice *u_dev, unsigned char *data, int length)
{
	/* Do a bulk write to the usb device.  libusb_close can hang if a bulk
	 * write is cancelled part way through, so prevent the calling thread from
	 * doing that.
	 */
	
	int err, tr, last_state;
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	if (!u_dev->output.num) {
		u_dev->err = "No output endpoint!";
		return FALSE;
	}
	
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &last_state);
	err = libusb_bulk_transfer (u_dev->handle,
								u_dev->output.address,
								data,
								length, 
								&tr,
								GQUSB_BULK_TIMEOUT);
	pthread_setcancelstate (last_state, NULL);
	if (err != 0) {
		u_dev->err = "Error writing to device";
		return FALSE;
	}
	if (tr < length) {
		u_dev->err = "Insufficient data written to device";
		return FALSE;
	}
	
	return TRUE;	
}

int gqusb_bulk_write_lock (struct usbdevice *u_dev, unsigned char *data, 
						   int length)
{
	/* Do a bulk write to the usb device, locking the mutex and keeping it 
	 * locked.  libusb_close can hang if a bulk write is cancelled part way
	 * through so the call to pthread_setcancelstate prevents this thread from
	 * being cancelled whilst accessing the camera and while it holds the
	 * mutex lock.  This function should be followed by a call to 
	 * gqusb_bulk_write_unlock to unlock the mutex and reset the cancel state.
	 */
	
	int err, tr;
	
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
	pthread_mutex_lock (&u_dev->io_mutex);
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	if (!u_dev->output.num) {
		u_dev->err = "No output endpoint!";
		goto write_error;
	}
	
	err = libusb_bulk_transfer (u_dev->handle,
								u_dev->output.address,
								data,
								length, 
								&tr,
								GQUSB_BULK_TIMEOUT);
	if (err != 0) {
		u_dev->err = "Error writing to device";
		goto write_error;
	}
	if (tr < length) {
		u_dev->err = "Insufficient data written to device";
		goto write_error;
	}
	
	return TRUE;
	
write_error:
	pthread_mutex_unlock (&u_dev->io_mutex);
	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	return FALSE;
}

int gqusb_bulk_write_unlock (struct usbdevice *u_dev, unsigned char *data, 
							 int length)
{
	/* Do a bulk write to the usb device, then unlock the mutex and reset the
	 * thread cancel state.  It is intended that this function is called 
	 * following a call to gqusb_bulk_write_lock from the same thread.
	 */
	
	int err, tr;
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	if (!u_dev->output.num) {
		u_dev->err = "No output endpoint!";
		goto write_error;
	}
	
	err = libusb_bulk_transfer (u_dev->handle,
								u_dev->output.address,
								data,
								length, 
								&tr,
								GQUSB_BULK_TIMEOUT);
	if (err != 0) {
		u_dev->err = "Error writing to device";
		goto write_error;
	}
	if (tr < length) {
		u_dev->err = "Insufficient data written to device";
		goto write_error;
	}
	
	pthread_mutex_unlock (&u_dev->io_mutex);
	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	return TRUE;
		
write_error:
	pthread_mutex_unlock (&u_dev->io_mutex);
	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	return FALSE;
}

int gqusb_bulk_io (struct usbdevice *u_dev, unsigned char *wdata, int wlength,
										    unsigned char *rdata, int rlength)
{
	/* Combined bulk write/read, protected by a mutex to prevent more than one
	 * thread attempting to read or write to the same camera at the same time.
	 * libusb_close can hang if a bulk read or write is cancelled part way 
	 * through, so prevent the calling thread from doing that.
	 */
	 
	int err, tr, bytes, last_state;
	
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_mutex_lock (&u_dev->io_mutex);
	
	u_dev->func = __func__;
	u_dev->err = NULL;
	
	if (wlength) {
		if (!u_dev->output.num) {
			u_dev->err = "No output endpoint!";
			goto io_error;
		}
		
		err = libusb_bulk_transfer (u_dev->handle,
									u_dev->output.address,
									wdata,
									wlength, 
									&tr,
									GQUSB_BULK_TIMEOUT);
		if (err != 0) {
			u_dev->err = "Error writing to device";
			goto io_error;
		}
		if (tr < wlength) {
			u_dev->err = "Insufficient data written to device";
			goto io_error;
		}
	}
	
	if (rlength) {
		if (!u_dev->input.num) {
			u_dev->err = "No input endpoint!";
			goto io_error;
		}
		
		bytes = 0;
		while ((err = libusb_bulk_transfer (u_dev->handle,
											u_dev->input.address,
											rdata+bytes,
											rlength - bytes, 
											&tr,
											GQUSB_BULK_TIMEOUT)) ==
											LIBUSB_ERROR_TIMEOUT) {
			bytes+= tr;
		}
		
		if (err != 0) {
			u_dev->err = "Error reading from device";
			goto io_error;
		}
		if (tr < rlength) {
			u_dev->err = "Insufficient data returned from device";
			goto io_error;
		}
	}
	
	pthread_mutex_unlock (&u_dev->io_mutex);
	pthread_setcancelstate (last_state, NULL);
	return TRUE;
	
io_error:
	pthread_mutex_unlock (&u_dev->io_mutex);
	pthread_setcancelstate (last_state, NULL);
	return FALSE;
}

#endif /* HAVE_LIBUSB */

