/*****************************************************************************
 *
 * ZWASIinit.c -- Initialise ZW ASI cameras
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "oacam.h"
#include "ZWASIoacam.h"

#define	cameraState		camera->_zwasi
#define CLEAR(x)		memset ( &(x), 0, sizeof ( x ))

static void _clearCameraData ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaZWASIInitCamera ( oaDevice* device )
{
  oaCamera*			camera;
  int                   	e, i, j, n, ret, multiplier;
  __u32                 	id;

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    // FIX ME -- set errno?
    return 0;
  }
  _clearCameraData ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraState.initialised = 0;
  cameraState.index = -1;

  if ( !openCamera ( device->_devIndex )) {
    fprintf ( stderr, "open of camera %ld failed\n", device->_devIndex );
    return 0;
  }

  camera->interface = device->interface;
  cameraState.index = device->_devIndex;
  cameraState.cameraType = device->_devType;

  // FIX ME -- should check from scratch for the genuine default values
  // as the current code isn't reliable for warm-started cameras

  CLEAR ( camera->controls );

  if ( isAvailable ( CONTROL_GAIN )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_GAIN ] = 1;
    cameraState.minGain = getMin ( CONTROL_GAIN );
    cameraState.maxGain = getMax ( CONTROL_GAIN );
    cameraState.stepGain = 1;
    cameraState.defaultGain = getValue ( CONTROL_GAIN, &autoEnabled );
    if ( cameraState.defaultGain < cameraState.minGain ) {
      cameraState.defaultGain = cameraState.minGain;
    }
    if ( cameraState.defaultGain > cameraState.maxGain ) {
      cameraState.defaultGain = cameraState.maxGain;
    }
    cameraState.currentGain = cameraState.defaultGain;
    if ( isAutoSupported ( CONTROL_GAIN )) {
      camera->controls [ OA_CTRL_AUTOGAIN ] = 1;
      cameraState.defaultAutoGain = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_EXPOSURE )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_EXPOSURE_ABSOLUTE ] = 1;
    cameraState.minAbsoluteExposure = getMin ( CONTROL_EXPOSURE ) / 1000;
    cameraState.maxAbsoluteExposure = getMax ( CONTROL_EXPOSURE ) / 1000;
    cameraState.stepAbsoluteExposure = 1;
    cameraState.defaultAbsoluteExposure = getValue ( CONTROL_EXPOSURE,
        &autoEnabled ) / 1000;
    if ( cameraState.defaultAbsoluteExposure <
        cameraState.minAbsoluteExposure ) {
      cameraState.defaultAbsoluteExposure = cameraState.minAbsoluteExposure;
    }
    if ( cameraState.defaultAbsoluteExposure >
        cameraState.maxAbsoluteExposure ) {
      cameraState.defaultAbsoluteExposure = cameraState.maxAbsoluteExposure;
    }
    cameraState.currentAbsoluteExposure = cameraState.defaultAbsoluteExposure;
    if ( isAutoSupported ( CONTROL_EXPOSURE )) {
      camera->controls [ OA_CTRL_AUTO_EXPOSURE ] = 1;
      cameraState.defaultAutoExposure = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_GAMMA )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_GAMMA ] = 1;
    cameraState.minGamma = getMin ( CONTROL_GAMMA );
    cameraState.maxGamma = getMax ( CONTROL_GAMMA );
    cameraState.stepGamma = 1;
    cameraState.defaultGamma = getValue ( CONTROL_GAMMA, &autoEnabled );
    if ( cameraState.defaultGamma < cameraState.minGamma ) {
      cameraState.defaultGamma = cameraState.minGamma;
    }
    if ( cameraState.defaultGamma > cameraState.maxGamma ) {
      cameraState.defaultGamma = cameraState.maxGamma;
    }
    cameraState.currentGamma = cameraState.defaultGamma;
    if ( isAutoSupported ( CONTROL_GAMMA )) {
      camera->controls [ OA_CTRL_AUTO_GAMMA ] = 1;
      cameraState.defaultAutoGamma = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_WB_R )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_RED_BALANCE ] = 1;
    cameraState.minRedBalance = getMin ( CONTROL_WB_R );
    cameraState.maxRedBalance = getMax ( CONTROL_WB_R );
    cameraState.stepRedBalance = 1;
    cameraState.defaultRedBalance = getValue ( CONTROL_WB_R, &autoEnabled );
    if ( cameraState.defaultRedBalance < cameraState.minRedBalance ) {
      cameraState.defaultRedBalance = cameraState.minRedBalance;
    }
    if ( cameraState.defaultRedBalance > cameraState.maxRedBalance ) {
      cameraState.defaultRedBalance = cameraState.maxRedBalance;
    }
    cameraState.currentRedBalance = cameraState.defaultRedBalance;
    if ( isAutoSupported ( CONTROL_WB_R )) {
      camera->controls [ OA_CTRL_AUTO_RED_BALANCE ] = 1;
      cameraState.defaultAutoRedBalance = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_WB_B )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_BLUE_BALANCE ] = 1;
    cameraState.minBlueBalance = getMin ( CONTROL_WB_B );
    cameraState.maxBlueBalance = getMax ( CONTROL_WB_B );
    cameraState.stepBlueBalance = 1;
    cameraState.defaultBlueBalance = getValue ( CONTROL_WB_B, &autoEnabled );
    // The blue balance for the ASI120MC seems way too high at a default
    // of 95 vs. 52 for red, so set them both here.  70 for red and 85 for
    // blue look about right to me.
    if ( ZWO_ASI120MC == cameraState.cameraType ) {
      cameraState.defaultRedBalance = cameraState.currentRedBalance = 50;
      cameraState.defaultBlueBalance = 70;
    }
    if ( cameraState.defaultBlueBalance < cameraState.minBlueBalance ) {
      cameraState.defaultBlueBalance = cameraState.minBlueBalance;
    }
    if ( cameraState.defaultBlueBalance > cameraState.maxBlueBalance ) {
      cameraState.defaultBlueBalance = cameraState.maxBlueBalance;
    }
    cameraState.currentBlueBalance = cameraState.defaultBlueBalance;
    if ( isAutoSupported ( CONTROL_WB_B )) {
      camera->controls [ OA_CTRL_AUTO_BLUE_BALANCE ] = 1;
      cameraState.defaultAutoBlueBalance = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_BRIGHTNESS )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_BRIGHTNESS ] = 1;
    cameraState.minBrightness = getMin ( CONTROL_BRIGHTNESS );
    cameraState.maxBrightness = getMax ( CONTROL_BRIGHTNESS );
    cameraState.stepBrightness = 1;
    cameraState.defaultBrightness = getValue ( CONTROL_BRIGHTNESS,
        &autoEnabled );
    if ( cameraState.defaultBrightness < cameraState.minBrightness ) {
      cameraState.defaultBrightness = cameraState.minBrightness;
    }
    if ( cameraState.defaultBrightness > cameraState.maxBrightness ) {
      cameraState.defaultBrightness = cameraState.maxBrightness;
    }
    cameraState.currentBrightness = cameraState.defaultBrightness;
    if ( isAutoSupported ( CONTROL_BRIGHTNESS )) {
      camera->controls [ OA_CTRL_AUTO_BRIGHTNESS ] = 1;
      cameraState.defaultAutoBrightness = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_BANDWIDTHOVERLOAD )) {
    int autoEnabled;
    camera->controls [ OA_CTRL_USBTRAFFIC ] = 1;
    cameraState.minUSBTraffic = getMin ( CONTROL_BANDWIDTHOVERLOAD );
    cameraState.maxUSBTraffic = getMax ( CONTROL_BANDWIDTHOVERLOAD );
    cameraState.stepUSBTraffic = 1;
    cameraState.defaultUSBTraffic = getValue ( CONTROL_BANDWIDTHOVERLOAD,
        &autoEnabled );
    // Setting this to at most 45 seems to be the only way to get the
    // camera to work reliably, at least for the ASI120MC and ASI120MM in
    // 8-bit mode, but that's still not low enough for 16-bit.
    if ( ZWO_ASI120MC == cameraState.cameraType ||
        ZWO_ASI120MM == cameraState.cameraType ) {
      cameraState.defaultUSBTraffic = 40;
    }
    if ( cameraState.defaultUSBTraffic < cameraState.minUSBTraffic ) {
      cameraState.defaultUSBTraffic = cameraState.minUSBTraffic;
    }
    if ( cameraState.defaultUSBTraffic > cameraState.maxUSBTraffic ) {
      cameraState.defaultUSBTraffic = cameraState.maxUSBTraffic;
    }
    cameraState.currentUSBTraffic = cameraState.defaultUSBTraffic;
    if ( isAutoSupported ( CONTROL_BANDWIDTHOVERLOAD )) {
      camera->controls [ OA_CTRL_AUTO_USBTRAFFIC ] = 1;
      cameraState.defaultAutoUSBTraffic = autoEnabled;
    }
  }

  cameraState.maxResolutionX = getMaxWidth();
  cameraState.maxResolutionY = getMaxHeight();

  if ( isBinSupported ( 2 ) || isBinSupported ( 3 ) || isBinSupported ( 4 )) {
    camera->controls [ OA_CTRL_BINNING ] = 1;
  }

  // These appear to be supported by all cameras

  camera->controls [ OA_CTRL_HFLIP ] = 1;
  cameraState.defaultHFlip = cameraState.currentHFlip = 0;
  camera->controls [ OA_CTRL_VFLIP ] = 1;
  cameraState.defaultVFlip = cameraState.currentVFlip = 0;
  camera->controls [ OA_CTRL_ROI ] = 1;

  // Ok, now we need to find out what frame formats are supported and
  // which one we want to use

  cameraState.videoRGB24 = cameraState.videoGrey16 = cameraState.videoGrey = 0;
  cameraState.videoCurrent = -1;
  // The mono ASI120MM will do RGB24 as a greyscale RGB image if we ask it
  // to, but that's rather wasteful, so we only support this for colour
  // cameras
  if ( isColorCam()) {
    if ( isImgTypeSupported ( IMG_RGB24 )) {
      cameraState.videoCurrent = IMG_RGB24;
      cameraState.videoRGB24 = 1;
    }
  } else {
    if ( isImgTypeSupported ( IMG_RAW8 )) { // prefer 8-bit to 16-bit initially
      cameraState.videoCurrent = IMG_RAW8;
      cameraState.videoGrey = 1;
    }
    if ( isImgTypeSupported ( IMG_RAW16 )) {
      if ( cameraState.videoCurrent < 0 ) {
        cameraState.videoCurrent = IMG_RAW16;
      }
      cameraState.videoGrey16 = 1;
    }
    if ( isImgTypeSupported ( IMG_Y8 )) {
      if ( cameraState.videoCurrent < 0 ) {
        cameraState.videoCurrent = IMG_Y8;
      }
      cameraState.videoGrey = 1;
    }
  }

  if ( -1 == cameraState.videoCurrent ) {
    fprintf ( stderr, "No suitable video format found on camera %d",
        cameraState.index );
    free (( void* ) camera );
    return 0;
  }

  cameraState.binMode = OA_BIN_MODE_NONE;

  // These resolutions are only valid without binning

  n = 0;
  switch ( cameraState.cameraType ) {
    case ZWO_ASI130MM:
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 1024;
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 600;
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 600;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 640;
      cameraState.resolutions[n++].y = 480;
      cameraState.resolutions[n].x = 600;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 400;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 480;
      cameraState.resolutions[n++].y = 320;
      cameraState.resolutions[n].x = 320;
      cameraState.resolutions[n++].y = 240;
      cameraState.resolutions[n].x = 0;
      cameraState.resolutions[n++].y = 0;

      cameraState.binMode2Resolutions[0].x = 640;
      cameraState.binMode2Resolutions[0].y = 512;
      cameraState.binMode2Resolutions[1].x = 0;
      cameraState.binMode2Resolutions[1].y = 0;

      cameraState.binMode3Resolutions[0].x = 0;
      cameraState.binMode3Resolutions[0].y = 0;

      cameraState.binMode4Resolutions[0].x = 320;
      cameraState.binMode4Resolutions[0].y = 256;
      cameraState.binMode4Resolutions[1].x = 0;
      cameraState.binMode4Resolutions[1].y = 0;

      break;

    case ZWO_ASI120MM:
    case ZWO_ASI120MC:
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 960;
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 720;
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 600;
      cameraState.resolutions[n].x = 1280;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 960;
      cameraState.resolutions[n++].y = 960;
      cameraState.resolutions[n].x = 1024;
      cameraState.resolutions[n++].y = 768;
      cameraState.resolutions[n].x = 1024;
      cameraState.resolutions[n++].y = 600;
      cameraState.resolutions[n].x = 1024;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 800;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 640;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 512;
      cameraState.resolutions[n].x = 800;
      cameraState.resolutions[n++].y = 320;
      cameraState.resolutions[n].x = 640;
      cameraState.resolutions[n++].y = 560;
      cameraState.resolutions[n].x = 640;
      cameraState.resolutions[n++].y = 480;
      cameraState.resolutions[n].x = 512;
      cameraState.resolutions[n++].y = 440;
      cameraState.resolutions[n].x = 512;
      cameraState.resolutions[n++].y = 400;
      cameraState.resolutions[n].x = 480;
      cameraState.resolutions[n++].y = 320;
      cameraState.resolutions[n].x = 320;
      cameraState.resolutions[n++].y = 240;
      cameraState.resolutions[n].x = 0;
      cameraState.resolutions[n++].y = 0;

      if ( ZWO_ASI120MM == cameraState.cameraType ) {
        cameraState.binMode2Resolutions[0].x = 640;
        cameraState.binMode2Resolutions[0].y = 480;
        cameraState.binMode2Resolutions[1].x = 0;
        cameraState.binMode2Resolutions[1].y = 0;
      } else {
        cameraState.binMode2Resolutions[0].x = 0;
        cameraState.binMode2Resolutions[0].y = 0;
      }

      cameraState.binMode3Resolutions[0].x = 0;
      cameraState.binMode3Resolutions[0].y = 0;

      cameraState.binMode4Resolutions[0].x = 0;
      cameraState.binMode4Resolutions[0].y = 0;

      break;

    case ZWO_ASI035MM:
    case ZWO_ASI035MC:
      cameraState.resolutions[0].x = 752;
      cameraState.resolutions[0].y = 480;
      cameraState.resolutions[1].x = 640;
      cameraState.resolutions[1].y = 480;
      cameraState.resolutions[2].x = 600;
      cameraState.resolutions[2].y = 400;
      cameraState.resolutions[3].x = 400;
      cameraState.resolutions[3].y = 400;
      cameraState.resolutions[4].x = 320;
      cameraState.resolutions[4].y = 240;
      cameraState.resolutions[5].x = 0;
      cameraState.resolutions[6].y = 0;

      if ( ZWO_ASI035MM == cameraState.cameraType ) {
        cameraState.binMode2Resolutions[0].x = 376;
        cameraState.binMode2Resolutions[0].y = 240;
        cameraState.binMode2Resolutions[1].x = 0;
        cameraState.binMode2Resolutions[1].y = 0;
      } else {
        cameraState.binMode2Resolutions[0].x = 0;
        cameraState.binMode2Resolutions[0].y = 0;
      }

      cameraState.binMode3Resolutions[0].x = 0;
      cameraState.binMode3Resolutions[0].y = 0;

      cameraState.binMode4Resolutions[0].x = 0;
      cameraState.binMode4Resolutions[0].y = 0;

      break;

    case ZWO_ASI030MC:
      cameraState.resolutions[0].x = 640;
      cameraState.resolutions[0].y = 480;
      cameraState.resolutions[1].x = 480;
      cameraState.resolutions[1].y = 320;
      cameraState.resolutions[2].x = 320;
      cameraState.resolutions[2].y = 240;
      cameraState.resolutions[3].x = 0;
      cameraState.resolutions[3].y = 0;

      cameraState.binMode2Resolutions[0].x = 0;
      cameraState.binMode2Resolutions[0].y = 0;

      cameraState.binMode3Resolutions[0].x = 0;
      cameraState.binMode3Resolutions[0].y = 0;

      cameraState.binMode4Resolutions[0].x = 0;
      cameraState.binMode4Resolutions[0].y = 0;

      break;

    default:
      fprintf ( stderr, "invalid camera type in resolution selection\n",
          cameraState.cameraType );
      return 0;
      break;
  }

  cameraState.xSize = cameraState.maxResolutionX;
  cameraState.ySize = cameraState.maxResolutionY;
  cameraState.buffers = 0;
  cameraState.configuredBuffers = 0;
  cameraState.frameRateSupport = 0;

  // FIX ME -- check return code?
  initCamera();

  setImageFormat ( cameraState.xSize, cameraState.ySize, cameraState.binMode,
      cameraState.videoCurrent );

  // The largest buffer size we should need
  // colour is 3 bytes per pixel, mono one for 8-bit, two for 16-bit
  multiplier = 1;
  if ( isColorCam()) {
    multiplier = 3;
  } else {
    if ( IMG_RAW16 == cameraState.videoCurrent ) {
      multiplier = 2; 
    }
  }
  cameraState.imageBufferLength = cameraState.maxResolutionX *
      cameraState.maxResolutionY * multiplier;
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
      return 0;
    }
  }

  return camera;
}


static void
_clearCameraData ( oaCamera* camera )
{
  cameraState.initialised = 0;
  cameraState.index = 0;
  cameraState.cameraType = 0;
  cameraState.minBrightness = 0;
  cameraState.maxBrightness = 0;
  cameraState.stepBrightness = 0;
  cameraState.defaultBrightness = 0;
  cameraState.currentBrightness = 0;
  cameraState.minGain = 0;
  cameraState.maxGain = 0;
  cameraState.stepGain = 0;
  cameraState.defaultGain = 0;
  cameraState.currentGain = 0;
  cameraState.minAbsoluteExposure = 0;
  cameraState.maxAbsoluteExposure = 0;
  cameraState.stepAbsoluteExposure = 0;
  cameraState.defaultAbsoluteExposure = 0;
  cameraState.currentAbsoluteExposure = 0;
  cameraState.minGamma = 0;
  cameraState.maxGamma = 0;
  cameraState.stepGamma = 0;
  cameraState.defaultGamma = 0;
  cameraState.currentGamma = 0;
  cameraState.minRedBalance = 0;
  cameraState.maxRedBalance = 0;
  cameraState.stepRedBalance = 0;
  cameraState.defaultRedBalance = 0;
  cameraState.currentRedBalance = 0;
  cameraState.minBlueBalance = 0;
  cameraState.maxBlueBalance = 0;
  cameraState.stepBlueBalance = 0;
  cameraState.defaultBlueBalance = 0;
  cameraState.currentBlueBalance = 0;
  cameraState.minUSBTraffic = 0;
  cameraState.maxUSBTraffic = 0;
  cameraState.stepUSBTraffic = 0;
  cameraState.defaultUSBTraffic = 0;
  cameraState.currentUSBTraffic = 0;
  cameraState.defaultAutoGain = 0;
  cameraState.defaultAutoBrightness = 0;
  cameraState.defaultAutoExposure = 0;
  cameraState.defaultAutoGamma = 0;
  cameraState.defaultAutoBlueBalance = 0;
  cameraState.defaultAutoRedBalance = 0;
  cameraState.defaultAutoUSBTraffic = 0;
  cameraState.defaultHFlip = 0;
  cameraState.currentHFlip = 0;
  cameraState.defaultVFlip = 0;
  cameraState.currentVFlip = 0;
  cameraState.videoRGB24 = 0;
  cameraState.videoGrey16 = 0;
  cameraState.videoGrey = 0;
  cameraState.videoCurrent = 0;
  cameraState.frameRateSupport = 0;
  cameraState.binMode = 0;
  cameraState.xSize = 0;
  cameraState.ySize = 0;
  cameraState.buffers = 0;
  cameraState.configuredBuffers = 0;
  cameraState.lastUsedBuffer = 0;
  cameraState.bufferInUse = 0;
  cameraState.frameTime = 0;
  cameraState.imageBufferLength = 0;
}
