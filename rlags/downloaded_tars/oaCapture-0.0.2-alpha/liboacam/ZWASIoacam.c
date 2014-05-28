/*****************************************************************************
 *
 * ZWASIoacam.c -- main entrypoint for ZW ASI Cameras
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
#include <ASICamera.h>

#include "oacam.h"
#include "ZWASIoacam.h"

const char *cameraNames[] = {
  "ZWO ASI130MM", "ZWO ASI120MM", "ZWO ASI120MC", "ZWO ASI035MM",
  "ZWO ASI035MC", "ZWO ASI030MC"
};

/**
 * Cycle through the cameras reported by the ASI library
 */

int
oaZWASIGetCameras ( oaDevice** deviceList )
{
  unsigned int				numFound = 0, current = 0;
  unsigned int         		 	i;
  const char*				currName;
  oaDevice*				dev;

  while ( deviceList[ current ] ) {
    current++;
  }
  if ( current >= OA_MAX_DEVICES ) {
    return 0;
  }

  if (( numFound = getNumberOfConnectedCameras()) < 1 ) {
    return 0;
  }

  for ( i = 0; i < numFound; i++ ) {
    if (!( currName = getCameraModel ( i ))) {
      fprintf ( stderr, "ZW name[%d] = 0\n", i );
    } else {
      int j, cameraType, found = 0;
      for ( j = 0; !found && i < ZWO_NUM_CAMERAS; j++ ) {
        if ( !strcmp ( currName, cameraNames[j] )) {
          found = 1;
          cameraType = j;
        }
      }
      if ( !found ) {
        fprintf ( stderr, "Unrecognised camera '%s'\n", currName );
      } else {
        // FIX ME check number with the same name and add index to name
        if (!( dev = malloc ( sizeof ( oaDevice )))) {
          return -1;
        }
        dev->interface = OA_IF_ZWASI;
        ( void ) strcpy ( dev->deviceName, currName );
        dev->_devType = cameraType;
        dev->_devIndex = i;
        deviceList[ current++ ] = dev;
      }
    }
  }

  return numFound;
}


void
oaZWASICloseCamera ( oaCamera* camera )
{
  // FIX ME -- implement
}
