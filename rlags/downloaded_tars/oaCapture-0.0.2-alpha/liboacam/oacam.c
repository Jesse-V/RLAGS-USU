/*****************************************************************************
 *
 * oacam.c -- main camera library entrypoint
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

#include <sys/types.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "oacam.h"
#include "V4L2oacam.h"
#include "PGRoacam.h"
#include "PWCoacam.h"
#include "ZWASIoacam.h"
#include "QHYoacam.h"

static int	_devicesFound = 0;

static int      (*getFunctions[])() = {
#ifdef	OA_PWC_DRIVER_SUPPORT
   oaPWCGetCameras,
#endif
   oaZWASIGetCameras,
   oaQHYGetCameras,
#ifdef	OA_PGR_DRIVER_SUPPORT
   oaPGRGetCameras,
#endif
   oaV4L2GetCameras,
   0
};
  

int
oaGetCameras( oaDevice** deviceList )
{
  int           i = 0, n, numDevices = 0;

  while ( getFunctions[i] ) {
    // FIX ME
    // error on found < 0 ?
    n = getFunctions[i]( deviceList );
    numDevices += n;
    i++;
  }

  return numDevices;
}


oaCamera*
oaInitCamera ( oaDevice* device )
{
  switch ( device->interface ) {
    case OA_IF_V4L2:
      return oaV4L2InitCamera ( device );
      break;
    case OA_IF_ZWASI:
      return oaZWASIInitCamera ( device );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaInitCamera: camera %d not yet supported\n", device->interface );
      break;
    default:
      fprintf ( stderr, "Unrecognised device interface '%d'\n",
          device->interface );
  }
  return 0;
}


void
oaCloseCamera ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      oaV4L2CloseCamera ( camera );
      break;
    case OA_IF_ZWASI:
      oaZWASICloseCamera ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCloseCamera: camera %d not yet supported\n", camera->interface );
      break;
    default:
      fprintf ( stderr, "Unrecognised camera interface '%d'\n",
          camera->interface );
  }
  return;
}
