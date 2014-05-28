/*****************************************************************************
 *
 * getState.c -- interface for camera state queries
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

// FIX ME -- this should perhaps return a status value?

void
oaCameraGetControlRange ( oaCamera* camera, int control, int* min, int* max,
    int* step, int* def )
{
  // FIX ME -- This flag should perhaps be pulled out of the camera-specific
  // data and into the generic area

  switch ( camera->interface ) {
    case OA_IF_V4L2:
      oaV4L2CameraGetControlRange ( camera, control, min, max, step, def );
      break;
    case OA_IF_ZWASI:
      oaZWASICameraGetControlRange ( camera, control, min, max, step, def );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetControlRange: camera %d not yet supported\n", camera->interface );
      break;
  }

  return;
}


FRAMESIZE*
oaCameraGetFrameSizes ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetFrameSizes ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraGetFrameSizes ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetFrameSizes: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


FRAMERATE*
oaCameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetFrameRates ( camera, resX, resY );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
    case OA_IF_ZWASI:
      fprintf ( stderr, "oaCameraGetFrameRates: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


const char*
oaCameraGetName ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetName ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraGetName ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetName: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraGetFramePixelFormat ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetFramePixelFormat ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraGetFramePixelFormat ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetFramePixelFormat: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


float
oaCameraGetTemperature ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetTemperature ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraGetTemperature ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetTemperature: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}


int
oaCameraGetPixelFormatForBitDepth ( oaCamera* camera, int depth )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraGetPixelFormatForBitDepth ( camera, depth );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraGetPixelFormatForBitDepth ( camera, depth );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraGetPixelFormatForBitDepth: camera %d not yet supported\n", camera->interface );
      break;
  }

  return 0;
}
