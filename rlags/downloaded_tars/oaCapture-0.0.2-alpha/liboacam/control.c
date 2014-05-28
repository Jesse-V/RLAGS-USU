/*****************************************************************************
 *
 * control.c -- interface for camera control functions
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
#include <stdlib.h>
#include <stdio.h>

#include "oacam.h"
#include "V4L2oacam.h"
#include "PGRoacam.h"
#include "PWCoacam.h"
#include "ZWASIoacam.h"
#include "QHYoacam.h"


int
oaCameraSetControl ( oaCamera* camera, int control, int value )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraSetControl ( camera, control, value );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraSetControl ( camera, control, value );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraSetControl: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraSetROI ( oaCamera* camera, int x, int y )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraSetROI ( camera, x, y );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraSetROI ( camera, x, y );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraSetROI: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraSetFrameInterval ( oaCamera* camera, int numerator, int denominator )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraSetFrameInterval ( camera, numerator, denominator );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
    case OA_IF_ZWASI:   
      fprintf ( stderr, "oaCameraSetFrameInterval: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraStart ( oaCamera* camera, START_PARMS* parms )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraStart ( camera, parms );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraStart ( camera, parms );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraStart: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraStop ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraStop ( camera );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraStop ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraStop: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


int
oaCameraReset ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraReset ( camera );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraReset ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraReset: camera %d not yet supported\n", camera->interface );
      break;
  }
  return 0;
}


int
oaCameraSetBitDepth ( oaCamera* camera, int depth )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:
      return oaV4L2CameraSetBitDepth ( camera, depth );
      break;
    case OA_IF_ZWASI:
      return oaZWASICameraSetBitDepth ( camera, depth );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraSetControl: camera %d not yet supported\n", camera->interface );
      break;
  }
  return 0;
}

