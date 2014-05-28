/*****************************************************************************
 *
 * V4L2state.h -- V4L2 camera state header
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

#ifndef OA_V4L2_STATE_H
#define OA_V4L2_STATE_H

#include <sys/types.h>
#include <linux/videodev2.h>
#include <linux/limits.h>

#define V4L2_BUFFERS 4

struct buffer {
  void   *start;
  size_t length;
};

typedef struct {
  char		devicePath[ PATH_MAX+1];
  int            initialised;
  int            fd;
  int            exposureMode;
  float          exposureLength;
  __u32          exposureControl;
  int            haveAutoExposure;
  __u32          minBrightness;
  __u32          maxBrightness;
  __u32          stepBrightness;
  __u32          defaultBrightness;
  __u32          minContrast;
  __u32          maxContrast;
  __u32          stepContrast;
  __u32          defaultContrast;
  __u32          minSaturation;
  __u32          maxSaturation;
  __u32          stepSaturation;
  __u32          defaultSaturation;
  __u32          minHue;
  __u32          maxHue;
  __u32          stepHue;
  __u32          defaultHue;
  __u32          defaultAutoWhiteBalance;
  __u32          autoWhiteBalanceOff;
  __u32          minWhiteBalance;
  __u32          maxWhiteBalance;
  __u32          stepWhiteBalance;
  __u32          defaultWhiteBalance;
  __u32          minRedBalance;
  __u32          maxRedBalance;
  __u32          stepRedBalance;
  __u32          defaultRedBalance;
  __u32          minBlueBalance;
  __u32          maxBlueBalance;
  __u32          stepBlueBalance;
  __u32          defaultBlueBalance;
  __u32          minGamma;
  __u32          maxGamma;
  __u32          stepGamma;
  __u32          defaultGamma;
  __u32          minExposure;
  __u32          maxExposure;
  __u32          stepExposure;
  __u32          defaultExposure;
  __u32          defaultAutoGain;
  __u32          minGain;
  __u32          maxGain;
  __u32          stepGain;
  __u32          defaultGain;
  __u32          defaultHFlip;
  __u32          defaultVFlip;
  __u32          defaultAutoHue;
  __u32          minWhiteBalanceTemp;
  __u32          maxWhiteBalanceTemp;
  __u32          stepWhiteBalanceTemp;
  __u32          defaultWhiteBalanceTemp;
  __u32          minSharpness;
  __u32          maxSharpness;
  __u32          stepSharpness;
  __u32          defaultSharpness;
  __u32          defaultAutoBrightness;
  __u32          defaultAutoExposure;
  __u32          minAbsoluteExposure;
  __u32          maxAbsoluteExposure;
  __u32          stepAbsoluteExposure;
  __u32          defaultAbsoluteExposure;
  __u32          minPanRelative;
  __u32          maxPanRelative;
  __u32          stepPanRelative;
  __u32          defaultPanRelative;
  __u32          minTiltRelative;
  __u32          maxTiltRelative;
  __u32          stepTiltRelative;
  __u32          defaultTiltRelative;
  __u32          defaultPanReset;
  __u32          defaultTiltReset;
  __u32          minPanAbsolute;
  __u32          maxPanAbsolute;
  __u32          stepPanAbsolute;
  __u32          defaultPanAbsolute;
  __u32          minTiltAbsolute;
  __u32          maxTiltAbsolute;
  __u32          stepTiltAbsolute;
  __u32          defaultTiltAbsolute;
  __u32          minZoomAbsolute;
  __u32          maxZoomAbsolute;
  __u32          stepZoomAbsolute;
  __u32          defaultZoomAbsolute;
  int            videoYUV420;
  int            videoRGB24;
  int            videoYUYV;
  int            videoGrey16;
  int            videoGrey;
  int            videoBGGR8;
  __u32          videoCurrent;
  int            frameSizing;
  // FIX ME -- ought to do this dynamically
  FRAMESIZE      resolutions[OA_MAX_RESOLUTIONS+1];
  // FIX ME -- only works for discrete rates?
  FRAMERATE      framerates[OA_MAX_FRAMERATES+1];
  struct buffer* buffers;
  int            configuredBuffers;
  int            lastUsedBuffer;
  int            bufferInUse;
  int            frameTime;
  int            frameRateSupport;
  int            colour;
  int            raw;
  int            colourDxK21;

} V4L2_STATE;

#endif	/* OA_V4L2_STATE_H */
