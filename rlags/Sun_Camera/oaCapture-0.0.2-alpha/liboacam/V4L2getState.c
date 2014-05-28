/*****************************************************************************
 *
 * V4L2getState.c -- state querying for V4L2 cameras
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

void
oaV4L2CameraGetControlRange ( oaCamera* camera, int control, int* min,
    int* max, int* step, int* def )
{
  switch ( control ) {

    case OA_CTRL_GAIN:
      *min = cameraState.minGain;
      *max = cameraState.maxGain;
      *step = cameraState.stepGain;
      *def = cameraState.defaultGain;
      break;

    case OA_CTRL_GAMMA:
      *min = cameraState.minGamma;
      *max = cameraState.maxGamma;
      *step = cameraState.stepGamma;
      *def = cameraState.defaultGamma;
      break;

    case OA_CTRL_BRIGHTNESS:
      *min = cameraState.minBrightness;
      *max = cameraState.maxBrightness;
      *step = cameraState.stepBrightness;
      *def = cameraState.defaultBrightness;
      break;

    case OA_CTRL_EXPOSURE:
      *min = cameraState.minExposure;
      *max = cameraState.maxExposure;
      *step = cameraState.stepExposure;
      *def = cameraState.defaultExposure;
      break;

    case OA_CTRL_EXPOSURE_ABSOLUTE:
      *min = cameraState.minAbsoluteExposure;
      *max = cameraState.maxAbsoluteExposure;
      *step = cameraState.stepAbsoluteExposure;
      *def = cameraState.defaultAbsoluteExposure;
      break;

    default:
      fprintf ( stderr, "getControlRange not yet implemented for control %d\n",
          control );
      break;
  }
}


FRAMESIZE*
oaV4L2CameraGetFrameSizes ( oaCamera* camera )
{
  return cameraState.resolutions;
}


FRAMERATE*
oaV4L2CameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  struct v4l2_frmivalenum fint;
  int j = 0, k;

  CLEAR ( cameraState.framerates );
  for ( k = 0; j < OA_MAX_FRAMERATES; k++ ) { 
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
      } 
    } 
    if ( V4L2_FRMIVAL_TYPE_DISCRETE == fint.type ) { 
      cameraState.framerates[j].numerator = fint.discrete.numerator;
      cameraState.framerates[j].denominator = fint.discrete.denominator;
      j++;
    } 
  } 

  cameraState.framerates[j].numerator = 0;
  cameraState.framerates[j].denominator = 0;

  return cameraState.framerates;
}


const char*
oaV4L2CameraGetName ( oaCamera* camera )
{
  return camera->deviceName;
}


int
oaV4L2CameraGetFramePixelFormat ( oaCamera* camera )
{
  return OA_PIX_FMT_RGB24;
}


float
oaV4L2CameraGetTemperature ( oaCamera* camera )
{
  return 0;
}


int   
oaV4L2CameraGetPixelFormatForBitDepth ( oaCamera *camera, int depth )
{
  fprintf ( stderr, "oaV4L2CameraGetPixelFormatForBitDepth always returns RGB24\n" );
  return OA_PIX_FMT_RGB24;
}

