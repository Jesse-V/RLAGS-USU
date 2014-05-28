/*****************************************************************************
 *
 * ZWASIstate.h -- ZW ASI camera state header
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

#ifndef OA_ZWASI_STATE_H
#define OA_ZWASI_STATE_H

#define ZWASI_BUFFERS 1
  
struct ZWASIbuffer {  
  void   *start; 
  size_t length; 
};


typedef struct {
  char		devicePath[ PATH_MAX+1];
  int           initialised;
  int           index;
  int		cameraType;
  __u32          minBrightness;
  __u32          maxBrightness;
  __u32          stepBrightness;
  __u32          defaultBrightness;
  __u32          currentBrightness;
  __u32          minGain;
  __u32          maxGain;
  __u32          stepGain;
  __u32          defaultGain;
  __u32          currentGain;
  __u32          minAbsoluteExposure;
  __u32          maxAbsoluteExposure;
  __u32          stepAbsoluteExposure;
  __u32          defaultAbsoluteExposure;
  __u32          currentAbsoluteExposure;
  __u32          minGamma;
  __u32          maxGamma;
  __u32          stepGamma;
  __u32          defaultGamma;
  __u32          currentGamma;
  __u32          minRedBalance;
  __u32          maxRedBalance;
  __u32          stepRedBalance;
  __u32          defaultRedBalance;
  __u32          currentRedBalance;
  __u32          minBlueBalance;
  __u32          maxBlueBalance;
  __u32          stepBlueBalance;
  __u32          defaultBlueBalance;
  __u32          currentBlueBalance;
  __u32          minUSBTraffic;
  __u32          maxUSBTraffic;
  __u32          stepUSBTraffic;
  __u32          defaultUSBTraffic;
  __u32          currentUSBTraffic;
  __u32          defaultAutoGain;
  __u32          defaultAutoBrightness;
  __u32          defaultAutoExposure;
  __u32          defaultAutoGamma;
  __u32          defaultAutoBlueBalance;
  __u32          defaultAutoRedBalance;
  __u32          defaultAutoUSBTraffic;
  __u32          defaultHFlip;
  __u32          currentHFlip;
  __u32          defaultVFlip;
  __u32          currentVFlip;
  int            videoRGB24;
  int            videoGrey16;
  int            videoGrey;
  __u32          videoCurrent;
  // FIX ME -- ought to do this dynamically
  FRAMESIZE      resolutions[OA_MAX_RESOLUTIONS+1];
  // FIX ME -- I'm going to use a magic number here because there are so few
  FRAMESIZE      binMode2Resolutions[4];
  FRAMESIZE      binMode3Resolutions[4];
  FRAMESIZE      binMode4Resolutions[4];
  int            frameRateSupport;
  int            binMode;
  int            xSize;
  int            ySize;
  struct ZWASIbuffer* buffers;
  int            configuredBuffers;
  int            lastUsedBuffer;
  int            bufferInUse;
  int            frameTime;
  int            imageBufferLength;
  __u32          maxResolutionX;
  __u32          maxResolutionY;

} ZWASI_STATE;

#endif	/* OA_ZWASI_STATE_H */
