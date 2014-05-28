/*****************************************************************************
 *
 * ZWASIgetState.c -- state querying for ZW ASI cameras
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

#include <ASICamera.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "oacam.h"
#include "ZWASIoacam.h"

#define	cameraState		camera->_zwasi
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))


void
oaZWASICameraGetControlRange ( oaCamera* camera, int control, int* min,
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
oaZWASICameraGetFrameSizes ( oaCamera* camera )
{
  switch ( cameraState.binMode ) {
    case OA_BIN_MODE_NONE:
      return cameraState.resolutions;
      break;
    case OA_BIN_MODE_2x2:
      return cameraState.binMode2Resolutions;
      break;
    case OA_BIN_MODE_3x3:
      return cameraState.binMode3Resolutions;
      break;
    case OA_BIN_MODE_4x4:
      return cameraState.binMode4Resolutions;
      break;
  }
  fprintf ( stderr, "oaZWASICameraGetFrameSizes: unknown bin mode %d\n",
      cameraState.binMode );
}


const char*
oaZWASICameraGetName ( oaCamera* camera )
{
  return camera->deviceName;
}

int
oaZWASICameraGetFramePixelFormat ( oaCamera* camera )
{
  switch ( cameraState.videoCurrent ) {
    case IMG_RGB24:
      return OA_PIX_FMT_BGR24;
      break;
    case IMG_RAW8:
      return OA_PIX_FMT_GREY8;
      break;
    case IMG_RAW16:
      return OA_PIX_FMT_GREY16LE;
      break;
    default:
      // FIX ME -- need to sort this out
      fprintf ( stderr, "no configured return value for pixel format\n" );
      break;
  }
  // default.  not a great choice, but there you go
  return OA_PIX_FMT_BGR24;
}


float
oaZWASICameraGetTemperature ( oaCamera* camera )
{
  return getSensorTemp();
}


int
oaZWASICameraGetPixelFormatForBitDepth ( oaCamera *camera, int depth )
{
  if ( !depth ) {
    switch ( cameraState.videoCurrent ) {
      case IMG_RGB24:
        return OA_PIX_FMT_RGB24;
        break;
      case IMG_RAW16:
        // FIX ME -- handle colour camera
        return OA_PIX_FMT_GREY16LE;
        break;
      case IMG_RAW8:
        // FIX ME -- handle colour camera
        return OA_PIX_FMT_GREY8;
        break;
      case IMG_Y8:
        // FIX ME -- handle colour camera
        return OA_PIX_FMT_GREY8;
        break;
    }
  } else {
    if ( 12 == depth || 16 == depth ) {
      // FIX ME -- handle colour camera
      return OA_PIX_FMT_GREY16LE;
    }
    if ( 8 == depth ) {
      if ( isColorCam()) {
        return OA_PIX_FMT_RGB24;
      }
      return OA_PIX_FMT_GREY8;
    }
  }

  // default
  return OA_PIX_FMT_RGB24;
}
