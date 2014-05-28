/*****************************************************************************
 *
 * capture.c -- interface for capture functions
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
oaCameraReadFrame ( oaCamera* camera, void** buffer  )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      return oaV4L2CameraReadFrame ( camera, buffer );
      break;
    case OA_IF_ZWASI:   
      return oaZWASICameraReadFrame ( camera, buffer );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraReadFrame: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return 0;
}


void
oaCameraStartReadFrame ( oaCamera* camera, int frameTime )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      oaV4L2CameraStartReadFrame ( camera, frameTime );
      break;
    case OA_IF_ZWASI:   
      oaZWASICameraStartReadFrame ( camera, frameTime );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraStartReadFrame: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return;
}


void
oaCameraFinishReadFrame ( oaCamera* camera )
{
  switch ( camera->interface ) {
    case OA_IF_V4L2:    
      oaV4L2CameraFinishReadFrame ( camera );
      break;
    case OA_IF_ZWASI:   
      oaZWASICameraFinishReadFrame ( camera );
      break;
    case OA_IF_PGR:
    case OA_IF_PWC:
    case OA_IF_QHY:
      fprintf ( stderr, "oaCameraFinishReadFrame: camera %d not yet supported\n", camera->interface );
      break;
  } 
  return;
}
