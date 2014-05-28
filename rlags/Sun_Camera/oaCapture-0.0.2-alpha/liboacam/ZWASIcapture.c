/*****************************************************************************
 *
 * ZWASIcapture.c -- capture functions for ZW ASI cameras
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "oacam.h"
#include "ZWASIoacam.h"

#define cameraState             camera->_zwasi
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))

int
oaZWASICameraReadFrame ( oaCamera* camera, void** bufferPtr )
{
  int			ret;

  cameraState.bufferInUse = 0;
  if ( cameraState.lastUsedBuffer < 0 ) {
    cameraState.lastUsedBuffer = 0;
  }
  *bufferPtr = 0;
  if (( ret = getImageData (
      cameraState.buffers[ cameraState.lastUsedBuffer].start,
//    cameraState.imageBufferLength, -1 ))) {
      cameraState.imageBufferLength, cameraState.frameTime ))) {
    *bufferPtr = cameraState.buffers[ cameraState.lastUsedBuffer].start;
    cameraState.bufferInUse = 1;
    return cameraState.imageBufferLength;
  }
  return 0;
}


void
oaZWASICameraStartReadFrame ( oaCamera* camera, int frameTime )
{
  cameraState.frameTime = frameTime;
  return;
}


void
oaZWASICameraFinishReadFrame ( oaCamera* camera )
{
  if ( cameraState.bufferInUse ) {
    cameraState.bufferInUse = 0;
    cameraState.lastUsedBuffer = ( cameraState.lastUsedBuffer + 1 ) %
        cameraState.configuredBuffers;
  }
}
