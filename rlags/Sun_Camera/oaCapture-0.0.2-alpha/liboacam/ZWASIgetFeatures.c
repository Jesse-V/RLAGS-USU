/*****************************************************************************
 *
 * ZWASIgetFeatures.c -- feature enumeration for ZW ASI cameras
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

#include "oacam.h"
#include "ZWASIoacam.h"

#define	cameraState		camera->_zwasi

/**
 * Check if a camera supports 16 bit display
 */

int
oaZWASICameraHas16Bit ( oaCamera* camera )
{
  return cameraState.videoGrey16;
}


/**
 * Check if a camera supports binning
 */

int
oaZWASICameraHasBinning ( oaCamera* camera, int factor )
{
  return isBinSupported ( factor );
}


/**
 * Check if a camera has fixed frame sizes
 */

int
oaZWASICameraHasFixedFrameSizes ( oaCamera* camera )
{
  return 1;
}


/**
 * Check if a camera supports frame rates
 */

int
oaZWASICameraHasFrameRateSupport ( oaCamera* camera )
{
  return 0;
}


/**
 * Check if a camera has fixed frame rates
 */

int
oaZWASICameraHasFixedFrameRates ( oaCamera* camera, int resX, int resY )
{
  return 0;
}


int
oaZWASICameraStartRequiresROI ( oaCamera* camera )
{
  return 1;
}


int
oaZWASICameraIsColour ( oaCamera* camera )
{
  return isColorCam();
}


int
oaZWASICameraHasTemperature ( oaCamera* camera )
{
  return 1;
}


