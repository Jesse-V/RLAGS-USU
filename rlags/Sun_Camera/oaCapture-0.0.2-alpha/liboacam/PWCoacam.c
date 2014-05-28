/*****************************************************************************
 *
 * PWCoacam.c -- main entrypoint for non V4L2 PWC camera
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
#include "PWCoacam.h"

#define	VENDOR_ID	0x0471
#define PRODUCT_ID	0x0329

/**
 * Cycle through the sys filesystem looking for USBdevices with the
 * appropriate vendor ID and product ID
 */

int
oaPWCGetCameras ( oaDevice** deviceList )
{
  unsigned int				numFound = 0, current = 0;
  unsigned int         		 	numUSBDevices, i, r;
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

  for ( i = 0; i < numUSBDevices; i++ ) {
    device = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( device, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( ctx );
      return -1;
    }
    if ( VENDOR_ID == desc.idVendor && PRODUCT_ID == desc.idProduct ) {
      /*
       * This doesn't work without root access
       *
      if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
        libusb_free_device_list ( devlist, 1 );
        libusb_exit ( ctx );
        return -1;
      }
      if ( desc.iManufacturer > 0 ) {
        if ( LIBUSB_SUCCESS != libusb_get_string_descriptor_ascii (
            handle, desc.iManufacturer, manufacturer, OA_MAX_NAME_LEN )) {
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -1;
        }
      } else {
        *manufacturer = 0;
      }
      if ( desc.iProduct > 0 ) {
        if ( LIBUSB_SUCCESS != libusb_get_string_descriptor_ascii (
            handle, desc.iProduct, product, OA_MAX_NAME_LEN )) {
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -1;
        }
      } else {
        *product = 0;
      }
      libusb_close ( handle );
       */
      ( void ) strcpy ( manufacturer, "Philips" );
      ( void ) strcpy ( product, "SPC900NC" );
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
      dev->interface = OA_IF_PWC;
      ( void ) snprintf ( fullname, OA_MAX_NAME_LEN, "%s %s", manufacturer,
          product );
      ( void ) strcpy ( dev->deviceName, fullname );
      dev->_devIndex = index;
      deviceList[ current++ ] = dev;
      numFound++;
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}


void
oaPWCCloseCamera ( oaCamera* camera )
{
  // FIX ME -- implement
}
