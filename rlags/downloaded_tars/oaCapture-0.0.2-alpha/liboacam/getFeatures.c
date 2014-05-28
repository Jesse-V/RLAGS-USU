/*****************************************************************************
 *
 * getFeatures -- interface for camera feature enumeration functions
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

int
oaCameraHas16Bit ( oaCamera* camera )
{
  // We could perhaps implement this at this level by setting a flag
  // in osCamera from the init functions

  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraHas16Bit ( camera );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraHas16Bit ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraHas16Bit: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraHasBinning ( oaCamera* camera, int factor )
{
  // We could perhaps implement this at this level by setting some config
  // in osCamera from the init functions

  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraHasBinning ( camera, factor );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraHasBinning ( camera, factor );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraHasBinning: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraHasControl ( oaCamera* camera, int control )
{
  return camera->controls[ control ];
}


int
oaCameraHasFixedFrameSizes ( oaCamera* camera )
{
  // FIX ME -- This flag should perhaps be pulled out of the camera-specific
  // data and into the generic area

  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraHasFixedFrameSizes ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraHasFixedFrameSizes ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraHasFixedFrameSizes: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraHasFrameRateSupport ( oaCamera* camera )
{
  // FIX ME -- flag in config data?
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraHasFrameRateSupport ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraHasFrameRateSupport ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraHasFrameRateSupport: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraHasFixedFrameRates ( oaCamera* camera, int resX, int resY )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraHasFixedFrameRates ( camera, resX, resY );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
    case OA_IF_ZWASI:
      fprintf ( stderr, "oaCameraHasFixedFrameRates: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraStartRequiresROI ( oaCamera* camera )
{
  // FIX ME -- flag in config data?
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraStartRequiresROI ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
    case OA_IF_ZWASI:
      fprintf ( stderr, "oaCameraStartRequiresROI: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraIsColour ( oaCamera* camera )
{
  // We could perhaps implement this at this level by setting some config
  // in osCamera from the init functions

  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraIsColour ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraIsColour ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraIsColour: camera %d not yet supported\n", camera->interface );
      break;
  }
  return 0;
}


int
oaCameraHasTemperature ( oaCamera* camera )
{
  // We could perhaps implement this at this level by setting some config
  // in osCamera from the init functions

  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraHasTemperature ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraHasTemperature ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraHasTemperature: camera %d not yet supported\n", camera->interface );
      break;
  }
  return 0;
}
