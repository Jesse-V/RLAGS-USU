/******************************************************************************/
/*                  HEADER FILE FOR USB INTERFACE ROUTINES                    */
/*                                                                            */
/* Header file for USB interface routines.                                    */
/*                                                                            */
/* Copyright (C) 2012 - 2013  Edward Simonson                                 */
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

#ifndef GOQAT_USB
#define GOQAT_USB

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBUSB
#include <libusb-1.0/libusb.h>

struct uendpoint {                        /* USB endpoint                     */
	unsigned char address;
	int num;
};

struct udevice {
	struct libusb_device *dev;            /* USB device                       */
	struct libusb_device_handle *handle;  /* Handle to open device            */
	struct uendpoint input;               /* Input endpoint                   */
	struct uendpoint output;              /* Output endpoint                  */
	pthread_mutex_t io_mutex;             /* Mutex for write/read operations  */
	uint8_t intf_num;                     /* Interface number                 */
	int id;                           /* Sub-device ID; main or autog. camera */
	int idx;                          /* Index in array of device descriptions*/
	const char *func;                     /* Name of current gqusb function   */
	char *err;                            /* Error message                    */
};

extern int gqusb_list_devices (int vendor, int pids[], 
							   struct libusb_device *devs[]);
extern int gqusb_open_device (struct udevice *u_dev);
extern void gqusb_close_device (struct udevice *u_dev);
extern int gqusb_bulk_read (struct udevice *u_dev, unsigned char *data, 
							int length);
extern int gqusb_bulk_write (struct udevice *u_dev, unsigned char *data, 
							 int length);
extern int gqusb_bulk_write_lock (struct udevice *u_dev, unsigned char *data, 
								  int length);
extern int gqusb_bulk_write_unlock (struct udevice *u_dev, unsigned char *data, 
							 int length);
extern int gqusb_bulk_io (struct udevice *u_dev, 
						  unsigned char *wdata, int wlength,
						  unsigned char *rdata, int rlength);
								
#endif /* HAVE_LIBUSB */
#endif /* GOQAT_USB */

