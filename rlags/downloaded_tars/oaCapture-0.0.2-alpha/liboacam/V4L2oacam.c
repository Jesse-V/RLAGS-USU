/*****************************************************************************
 *
 * V4L2oacam.c -- main entrypoint for V4L2 Cameras
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
#include <linux/limits.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "oacam.h"
#include "V4L2oacam.h"

/**
 * Cycle through the sys filesystem looking for V4L devices and grab
 * their names
 */

int
oaV4L2GetCameras ( oaDevice** deviceList )
{
  DIR*			dirp;
  struct dirent*	entry;
  char			nameFile[ PATH_MAX ];
  FILE*			fp;
  char			name[ OA_MAX_NAME_LEN+1 ];
  unsigned int		numFound = 0, current = 0;
  oaDevice*             dev;
  unsigned int          index;

  while ( deviceList[ current ] ) {
    current++;
  }
  if ( current >= OA_MAX_DEVICES ) {
    return 0;
  }

  if ( access ( SYS_V4L_PATH, X_OK )) {
    return 0;
  }

  if ( 0 == ( dirp = opendir ( SYS_V4L_PATH ))) {
    return -1;
  }

  while ( entry = readdir ( dirp )) {
    if ( !strncmp ( entry->d_name, "video", 5 )) {
      // we need a numeric portion for the index
      if ( !isdigit ( entry->d_name[5] )) {
        closedir ( dirp );
        return -1;
      }
      index = atoi ( entry->d_name+5 );
      
      // FIX ME
      // check for error?
      ( void ) snprintf ( nameFile, PATH_MAX-1, "%s/%s/name", SYS_V4L_PATH,
          entry->d_name );
      if (!( fp = fopen ( nameFile, "r" ))) {
        closedir ( dirp );
        // FiX ME
        // set oaErrno
        return -1;
      }
      if ( !fgets ( name, OA_MAX_NAME_LEN, fp )) {
        closedir ( dirp );
        fclose ( fp );
        // FiX ME
        // set oaErrno
        return -1;
      }
      // remove terminating LF
      fclose ( fp );
      name[ strlen ( name ) - 1] = 0;

      // now we can drop the data into the list
      if (!( dev = malloc ( sizeof ( oaDevice )))) {
        closedir ( dirp );
        // FiX ME
        // set oaErrno
        return -1;
      }
      dev->interface = OA_IF_V4L2;
      ( void ) strcpy ( dev->deviceName, name );
      dev->_devIndex = index;
      deviceList[ current++ ] = dev;
      numFound++;
    }
  }
  closedir ( dirp );

  return numFound;
}


void
oaV4L2CloseCamera ( oaCamera* camera )
{
  if ( camera ) {
    if ( camera->_v4l2.fd >= 0 ) {
      v4l2_close ( camera->_v4l2.fd );
    }
    free ( camera );
  }
  return;
}
