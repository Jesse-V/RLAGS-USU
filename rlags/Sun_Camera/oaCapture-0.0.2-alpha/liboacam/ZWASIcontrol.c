/*****************************************************************************
 *
 * ZWASIcontrol.c -- control functions for ZW ASI cameras
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "oacam.h"
#include "ZWASIoacam.h"

#define	cameraState		camera->_zwasi
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))


int
oaZWASICameraSetControl ( oaCamera* camera, int control, int arg1 )
{
  switch ( control ) {
    case OA_CTRL_BRIGHTNESS:
      setValue ( CONTROL_BRIGHTNESS, arg1, 0 );
      cameraState.currentBrightness = arg1;
      break;
    case OA_CTRL_BLUE_BALANCE:
      setValue ( CONTROL_WB_B, arg1, 0 );
      cameraState.currentBlueBalance = arg1;
      break;
    case OA_CTRL_RED_BALANCE:
      setValue ( CONTROL_WB_R, arg1, 0 );
      cameraState.currentRedBalance = arg1;
      break;
    case OA_CTRL_GAMMA:
      setValue ( CONTROL_GAMMA, arg1, 0 );
      cameraState.currentGamma = arg1;
      break;
    case OA_CTRL_GAIN:
      setValue ( CONTROL_GAIN, arg1, 0 );
      cameraState.currentGain = arg1;
      break;
    case OA_CTRL_EXPOSURE_ABSOLUTE:
      setValue ( CONTROL_EXPOSURE, arg1 * 1000, 0 );
      cameraState.currentAbsoluteExposure = arg1 * 1000;
      break;
    case OA_CTRL_BINNING:
      // This will take effect when the resolution is switched
      cameraState.binMode = arg1;
      break;
    case OA_CTRL_HFLIP:
      cameraState.currentHFlip = arg1;
      SetMisc ( cameraState.currentHFlip, cameraState.currentVFlip );
      break;
    case OA_CTRL_VFLIP:
      cameraState.currentVFlip = arg1;
      SetMisc ( cameraState.currentHFlip, cameraState.currentVFlip );
      break;
    case OA_CTRL_AUTOGAIN:
      // FIX ME -- implement this
      break;
    case OA_CTRL_AUTO_BRIGHTNESS:
      // FIX ME -- implement this
      break;
    case OA_CTRL_AUTO_EXPOSURE:
      // FIX ME -- implement this
      break;     
    case OA_CTRL_AUTO_RED_BALANCE:
      // FIX ME -- implement this
      break;     
    case OA_CTRL_AUTO_BLUE_BALANCE:
      // FIX ME -- implement this
      break;     
    case OA_CTRL_ROI:
      fprintf ( stderr, "oaZWASICameraSetControl called with OA_CTRL_ROI\n" );
      fprintf ( stderr, "use oaZWASICameraSetROI\n" );
      return 0;
      break;
    default:
      fprintf ( stderr, "Unrecognised control %d in oaZWASICameraSetControl\n",
          control );
      return 0;
      break;
  }
  return -1;
}


int
oaZWASICameraSetROI ( oaCamera* camera, int x, int y )
{
  cameraState.xSize = x;
  cameraState.ySize = y;
  return 0;
}


int
oaZWASICameraStart ( oaCamera* camera, START_PARMS* parms )
{
  int i, j, multiplier = 1;

  cameraState.xSize = parms->size.x;
  cameraState.ySize = parms->size.y;
  // colour is 3 bytes per pixel, mono one for 8-bit, two for 16-bit
  if ( isColorCam()) {
    multiplier = 3;
  } else {
    if ( IMG_RAW16 == cameraState.videoCurrent ) {
      multiplier = 2;
    }
  }
  cameraState.imageBufferLength = parms->size.x * parms->size.y * multiplier;
  cameraState.lastUsedBuffer = -1;

  /*
   * This is now done by assigning the largest possible buffers when
   * the camera is initialised, but could be changed back to here to
   * use meory more efficiently.
   *
  cameraState.configuredBuffers = 0;
  cameraState.buffers = calloc ( ZWASI_BUFFERS, sizeof ( struct ZWASIbuffer ));
  for ( i = 0; i < ZWASI_BUFFERS; i++ ) {
    void* m = malloc ( cameraState.imageBufferLength );
    if ( m ) {
      cameraState.buffers[i].start = m;
      cameraState.configuredBuffers++;
    } else {
      fprintf ( stderr, "oaZWASICameraStart malloc failed\n" );
      if ( i ) {
        for ( j = 0; i < i; j++ ) {
          free (( void* ) cameraState.buffers[j].start );
        }
      }
      return -1;
    }
  }
  */

  setImageFormat ( cameraState.xSize, cameraState.ySize, cameraState.binMode,
      cameraState.videoCurrent );
  if ( 1 == cameraState.binMode &&
      ( cameraState.xSize != cameraState.maxResolutionX ||
      cameraState.ySize != cameraState.maxResolutionY )) {
    setStartPos (( cameraState.maxResolutionX - cameraState.xSize ) / 2,
        ( cameraState.maxResolutionY - cameraState.ySize ) / 2 );
  }
  setValue ( CONTROL_BRIGHTNESS, cameraState.currentBrightness, 0 );
  setValue ( CONTROL_WB_B, cameraState.currentBlueBalance, 0 );
  setValue ( CONTROL_WB_R, cameraState.currentRedBalance, 0 );
  setValue ( CONTROL_GAMMA, cameraState.currentGamma, 0 );
  setValue ( CONTROL_GAIN, cameraState.currentGain, 0 );
  setValue ( CONTROL_EXPOSURE, cameraState.currentAbsoluteExposure, 0 );
  setValue ( CONTROL_BANDWIDTHOVERLOAD, cameraState.currentUSBTraffic, 0 );

  SetMisc ( cameraState.currentHFlip, cameraState.currentVFlip );

  startCapture();
  return 0;
}


void
oaZWASICameraStop ( oaCamera* camera )
{
  int i;

  stopCapture();

  /*
   * This will be needed if the buffer assignment is restored in
   * CameraStart()
   *
  for ( i = 0; i < cameraState.configuredBuffers; i++ ) {
    free (( void* ) cameraState.buffers[i].start );
  }
  cameraState.configuredBuffers = 0;
  cameraState.lastUsedBuffer = -1;
  free (( void* ) cameraState.buffers );
  cameraState.buffers = 0;
   */
}


// FIX ME -- should this return value be the other way around?
int
oaZWASICameraReset ( oaCamera* camera )
{
  stopCapture();
  closeCamera();
  if ( !openCamera ( cameraState.index )) {
    fprintf ( stderr, "cannot open video device %d\n", cameraState.index );
    return -1;
  }
  // FIX ME -- check return code
  initCamera();
  return 0;
}


int
oaZWASICameraSetBitDepth ( oaCamera *camera, int depth )
{
  // 12 is the actual resolution of the ADC, so we allow it here
  if ( 16 == depth || 12 == depth ) {
    cameraState.videoCurrent = IMG_RAW16;
    return 0;
  } else {
    if ( 8 == depth ) {
      cameraState.videoCurrent = IMG_RAW8;
      return 0;
    }
  }

  return -1;
}
