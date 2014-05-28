/*****************************************************************************
 *
 * V4L2getFeatures.c -- feature enumeration for V4L2 cameras
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
#include <errno.h>
#include <string.h>

#include "oacam.h"
#include "V4L2oacam.h"
#include "v4l2ioctl.h"

#define	cameraState		camera->_v4l2
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))

/**
 * Check if a camera supports 16 bit display
 */

int
oaV4L2CameraHas16Bit ( oaCamera* camera )
{
  return cameraState.videoGrey16;
}


/**
 * Check if a camera supports binning
 */

int
oaV4L2CameraHasBinning ( oaCamera* camera, int factor )
{
  // Don't know of any V4L2 cameras that do support binning right now
  return 0;
}


/**
 * Check if a camera has fixed frame sizes
 */

int
oaV4L2CameraHasFixedFrameSizes ( oaCamera* camera )
{
  return ( OA_FRAMESIZES_DISCRETE == camera->_v4l2.frameSizing );
}


/**
 * Check if a camera supports frame rates
 */

int
oaV4L2CameraHasFrameRateSupport ( oaCamera* camera )
{
  return cameraState.frameRateSupport;
}


/**
 * Check if a camera has fixed frame rates
 */

int
oaV4L2CameraHasFixedFrameRates ( oaCamera* camera, int resX, int resY )
{
  struct v4l2_frmivalenum fint;
  int k;

  // This may depend on frame size, so it can't be a generic flag

  for ( k = 0;; k++ ) {
    CLEAR( fint );
    fint.index = k;
    fint.pixel_format = cameraState.videoCurrent;
    fint.width = resX;
    fint.height = resY;
    if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_ENUM_FRAMEINTERVALS,
        &fint )) {
      if ( EINVAL == errno) {
        break;
      } else {
        perror("VIDIOC_ENUM_FRAMEINTERVALS");
        return 0;
      }
    }
    if ( V4L2_FRMIVAL_TYPE_DISCRETE == fint.type ) {
      return 1;
    }
  }

  return 0;
}


/**
 * Some V4L2 cameras require the resolution to be set when started
 * whilst others don't.  Could be just UVC cameras, or could be just
 * the Lifecams.  My Lifecam Cinema gives an error on subsequent ioctls
 * if the resolution isn't set first, anyhow.
 * We'll assume they always do to make things simple
 */

int
oaV4L2CameraStartRequiresROI ( oaCamera* camera )
{
  return 1;
}


int
oaV4L2CameraIsColour ( oaCamera* camera )
{
  return cameraState.colour;
}


int
oaV4L2CameraHasTemperature ( oaCamera* camera )
{
  return 0;
}
