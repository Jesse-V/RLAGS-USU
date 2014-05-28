/*****************************************************************************
 *
 * QHYoacam.c -- main entrypoint for QHY Cameras
 *
 * Copyright 2013 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "oacam.h"
#include "QHYoacam.h"

struct qhycam {
  unsigned int	vendorId;
  unsigned int	productId;
  const char*	name;
};

static struct qhycam cameraList[] =
{
  { 0x16c0, 0x081e, "QHY2" },
  { 0x16c0, 0x081d, "QHY6" },
  { 0x16c0, 0x296d, "QHY5" },
  { 0x16c0, 0x2972, "QHY8" },
  { 0x16c0, 0x2981, "QHY6Pro" },
  { 0x1618, 0x025a, "QHY6" },
  { 0x1618, 0x0910, "QHY5T" },
  { 0x1618, 0x0921, "QHY5II" },
  { 0x1618, 0x0931, "QHY5V" },
  { 0x1618, 0xa285, "IMG2S" },
  { 0x1618, 0x1001, "QHY10" },
  { 0x1618, 0x1111, "QHY11" },
  { 0x1618, 0x1601, "QHY16" },
  { 0x1618, 0x2851, "IMG2Pro" },
  { 0x1618, 0x2859, "IMG2E" },
  { 0x1618, 0x4023, "QHY7" },
  { 0x1618, 0x6005, "QHY8L" },
  { 0x1618, 0x6007, "QHY8M" },
  { 0x1618, 0x6741, "QHY21" },
  { 0x1618, 0x6669, "QHY50" },
  { 0x1618, 0x666A, "IMG132E" },
  { 0x1618, 0x6941, "QHY22" },
  { 0x1618, 0x8051, "QHY20" },
  { 0x1618, 0x8301, "QHY9" },
  { 0x1618, 0x8311, "QHY9L" }
// Still missing:
// IMG0S, IMG0H, IMG0T, IMG0L, IMG0X, IMG1S, IMG3S, IMG5S, IMG1E, IMG3T
// QHY6S, QHY9S, QHY5L-II, QHY5T-II, QHY5R-II, QHY5R, QHY5P-II, QHY5V-II
// Q16000, QHY23
};
static unsigned int numCameras = 25;


/**
 * Cycle through the sys filesystem looking for USBdevices with one
 * of the appropriate vendor ID and product ID
 */

int
oaQHYGetCameras ( oaDevice** deviceList )
{
  unsigned int				numFound = 0, current = 0;
  unsigned int         		 	numUSBDevices, i, j, r;
  unsigned int  		        index;
  libusb_context*      		 	ctx = 0;
  libusb_device**      		 	devlist;
  libusb_device*			device;
  libusb_device_handle*			handle;
  struct libusb_device_descriptor	desc;
  char					manufacturer[ OA_MAX_NAME_LEN+1 ];
  char					product[ OA_MAX_NAME_LEN+1 ];
  char					fullname[ OA_MAX_NAME_LEN+1 ];
  oaDevice*				dev;
  unsigned int				matched;

  while ( deviceList[ current ] ) {
    current++;
  }
  if ( current >= OA_MAX_DEVICES ) {
    return 0;
  }

  libusb_init ( &ctx );
  // libusb_set_debug ( ctx, LIBUSB_LOG_LEVEL_DEBUG );
  numUSBDevices = libusb_get_device_list ( ctx, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    if ( numUSBDevices ) {
      return -1;
    }
    return 0;
  }

  matched = 0;
  for ( i = 0; i < numUSBDevices; i++ ) {
    device = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( device, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( ctx );
      return -1;
    }
    for ( j = 0; j < numCameras && !matched; j++ ) {
      if ( desc.idVendor == cameraList[j].vendorId &&
          desc.idProduct == cameraList[j].productId ) {
        index = libusb_get_bus_number ( device ) << 8;
        index |= libusb_get_device_address ( device );

        // now we can drop the data into the list
        if (!( dev = malloc ( sizeof ( oaDevice )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          // FiX ME
          // set oaErrno
          return -1;
        }
        dev->interface = OA_IF_QHY;
        ( void ) strcpy ( dev->deviceName, cameraList[j].name );
        dev->_devIndex = index;
        deviceList[ current++ ] = dev;
        numFound++;
        matched = 1;
      }
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}


void
oaQHYCloseCamera ( oaCamera* camera )
{
  // FIX ME -- implement
}
