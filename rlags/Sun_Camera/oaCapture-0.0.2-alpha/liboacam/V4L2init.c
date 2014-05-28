/*****************************************************************************
 *
 * V4L2init.c -- Initialise V4L2 cameras
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
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/sysctl.h>
#include <linux/limits.h>


#include "oacam.h"
#include "V4L2oacam.h"
#include "v4l2ioctl.h"

#define	DEF_EXPOSURE_MODE	V4L2_EXPOSURE_AUTO
#define	DEF_EXPOSURE_LEN	100
#define DEF_MAX_EXPOSURE	0x7fffffff
#define	DEF_BUFFERS		5

#define	cameraState		camera->_v4l2
#define CLEAR(x)		memset ( &(x), 0, sizeof ( x ))

/**
 * Initialise a given camera device
 */

oaCamera*
oaV4L2InitCamera ( oaDevice* device )
{
  char				deviceName[ PATH_MAX ];
  oaCamera*			camera;
  struct v4l2_queryctrl 	ctrl;
  struct v4l2_capability	cap;
  struct v4l2_fmtdesc		formatDesc;
  struct v4l2_format		format;
  struct v4l2_frmsizeenum	fsize;
  struct v4l2_streamparm        parm;
  int                   	e, i, j, ret;
  __u32                 	id;


  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    // FIX ME -- set errno?
    return 0;
  }

  camera->interface = device->interface;
  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraState.initialised = 0;
  cameraState.fd = -1;
  cameraState.exposureMode = DEF_EXPOSURE_MODE;

  cameraState.colourDxK21 = 0;
  if ( !strncmp ( camera->deviceName, "DFK 21", 6 ) ||
      !strncmp ( camera->deviceName, "DBK 21", 6 )) {
//fprintf(stderr,"is colour DxK21\n");
    cameraState.colourDxK21 = 1;
  }

  // path name for device is /dev/video<device._devIndex>
  ( void ) snprintf ( cameraState.devicePath, PATH_MAX, "/dev/video%ld",
      device->_devIndex );
  if (( cameraState.fd = v4l2_open ( cameraState.devicePath,
      O_RDWR | O_NONBLOCK, 0 )) < 0 ) {
    fprintf ( stderr, "oaV4L2InitCamera: cannot open video device %s\n",
        cameraState.devicePath );
    // errno should be set?
    free (( void * ) camera );
    return 0;
  }

  // Now we can get the capabilites and make sure this is a capture device

  CLEAR ( cap );
  if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_QUERYCAP, &cap )) {
    if ( EINVAL == errno ) {
      fprintf ( stderr, "%s is not a V4L2 device\n", deviceName );
    } else {
      perror ( "VIDIOC_QUERYCAP" );
    }
    free (( void* ) camera );
    return 0;
  }

  if (!( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE )) {
    fprintf ( stderr, "%s does not support video capture", deviceName );
    free (( void* ) camera );
    return 0;
  }

  // FIX ME -- check for streaming?  V4L2_CAP_STREAMING

  // And now what controls the device supports

  // the "get next" ioctl doesn't seem to work reliably sometimes, so do
  // this the hard way

  // FIX ME -- think about doing this the way it is all done in gf.c

  CLEAR ( camera->controls );

  for ( id = V4L2_CID_USER_BASE; id < V4L2_CID_LASTP1; id++ ) {
    CLEAR ( ctrl );
    ctrl.id = id;

    if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_QUERYCTRL, &ctrl )) {
      // EINVAL means we don't have this one
      if ( EINVAL != errno ) {
        fprintf ( stderr, "VIDIOC_QUERYCTRL( %x ) failed, errno %d\n", id,
            errno );
        continue;
      }
    }

    // FIX ME -- it's a bit of a pain to work through these one at a
    // time, but without doing so we could end up with a big mess with
    // the gui controls

    switch ( id ) {

      case V4L2_CID_BRIGHTNESS:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_BRIGHTNESS ] = 1;
          cameraState.minBrightness = ctrl.minimum;
          cameraState.maxBrightness = ctrl.maximum;
          cameraState.stepBrightness = ctrl.step;
          cameraState.defaultBrightness = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "brightness is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_CONTRAST:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_CONTRAST ] = 1;
          cameraState.minContrast = ctrl.minimum;
          cameraState.maxContrast = ctrl.maximum;
          cameraState.stepContrast = ctrl.step;
          cameraState.defaultContrast = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "contrast is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_SATURATION:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_SATURATION ] = 1;
          cameraState.minSaturation = ctrl.minimum;
          cameraState.maxSaturation = ctrl.maximum;
          cameraState.stepSaturation = ctrl.step;
          cameraState.defaultSaturation = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "saturation is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_HUE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_HUE ] = 1;
          cameraState.minHue = ctrl.minimum;
          cameraState.maxHue = ctrl.maximum;
          cameraState.stepHue = ctrl.step;
          cameraState.defaultHue = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "hue is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_AUDIO_VOLUME:
      case V4L2_CID_AUDIO_BALANCE:
      case V4L2_CID_AUDIO_BASS:
      case V4L2_CID_AUDIO_TREBLE:
      case V4L2_CID_AUDIO_MUTE:
      case V4L2_CID_AUDIO_LOUDNESS:
      case V4L2_CID_BLACK_LEVEL:
        break;

      case V4L2_CID_AUTO_WHITE_BALANCE:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_AUTO_WHITE_BALANCE ] = 1;
          cameraState.defaultAutoWhiteBalance = ctrl.default_value;
          cameraState.autoWhiteBalanceOff = 0;
        } else {
          int foundManual = 0;
          int manualValue = 0;
          if ( V4L2_CTRL_TYPE_MENU == ctrl.type ) {
            int m;
            struct v4l2_querymenu menuItem;
            for ( m = ctrl.minimum; m <= ctrl.maximum && !foundManual; m++ ) {
              CLEAR( menuItem );
              menuItem.id = V4L2_CID_AUTO_WHITE_BALANCE;
              menuItem.index = m;
              if ( !v4l2ioctl ( cameraState.fd, VIDIOC_QUERYMENU, &menuItem )) {
                if ( strcasestr ( menuItem.name, "manual" )) {
                  foundManual++;
                  manualValue = m;
                }
              }
            }
          }
          if ( foundManual ) {
            camera->controls[ OA_CTRL_AUTO_WHITE_BALANCE ] = 1;
            cameraState.defaultAutoWhiteBalance = ctrl.default_value;
            cameraState.autoWhiteBalanceOff = manualValue;
          } else {
            fprintf ( stderr, "AWB control type not handled (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_DO_WHITE_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_WHITE_BALANCE ] = 1;
          cameraState.minWhiteBalance = ctrl.minimum;
          cameraState.maxWhiteBalance = ctrl.maximum;
          cameraState.stepWhiteBalance = ctrl.step;
          cameraState.defaultWhiteBalance = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "white balance is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_RED_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_RED_BALANCE ] = 1;
          cameraState.minRedBalance = ctrl.minimum;
          cameraState.maxRedBalance = ctrl.maximum;
          cameraState.stepRedBalance = ctrl.step;
          cameraState.defaultRedBalance = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "red balance is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BLUE_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_BLUE_BALANCE ] = 1;
          cameraState.minBlueBalance = ctrl.minimum;
          cameraState.maxBlueBalance = ctrl.maximum;
          cameraState.stepBlueBalance = ctrl.step;
          cameraState.defaultBlueBalance = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "blue balance is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_GAMMA:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_GAMMA ] = 1;
          cameraState.minGamma = ctrl.minimum;
          cameraState.maxGamma = ctrl.maximum;
          cameraState.stepGamma = ctrl.step;
          cameraState.defaultGamma = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "gamma is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_EXPOSURE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_EXPOSURE ] = 1;
          cameraState.minExposure = ctrl.minimum;
          cameraState.maxExposure = ctrl.maximum;
          cameraState.stepExposure = ctrl.step;
          cameraState.defaultExposure = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "exposure is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_AUTOGAIN:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_AUTOGAIN ] = 1;
          cameraState.defaultAutoGain = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "Auto Gain is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_GAIN:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_GAIN ] = 1;
          cameraState.minGain = ctrl.minimum;
          cameraState.maxGain = ctrl.maximum;
          cameraState.stepGain = ctrl.step;
          cameraState.defaultGain = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "exposure is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_HFLIP:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_HFLIP ] = 1;
          cameraState.defaultHFlip = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HFLIP is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_VFLIP:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_VFLIP ] = 1;
          cameraState.defaultVFlip = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "VFLIP is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_HCENTER:
      case V4L2_CID_VCENTER:
      case V4L2_CID_POWER_LINE_FREQUENCY:
        break;

      case V4L2_CID_HUE_AUTO:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_HUE_AUTO ] = 1;
          cameraState.defaultAutoHue = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HUE_AUTO is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_WHITE_BALANCE_TEMP ] = 1;
          cameraState.minWhiteBalanceTemp = ctrl.minimum;
          cameraState.maxWhiteBalanceTemp = ctrl.maximum;
          cameraState.stepWhiteBalanceTemp = ctrl.step;
          cameraState.defaultWhiteBalanceTemp = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "white bal temp is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_SHARPNESS:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_SHARPNESS ] = 1;
          cameraState.minSharpness = ctrl.minimum;
          cameraState.maxSharpness = ctrl.maximum;
          cameraState.stepSharpness = ctrl.step;
          cameraState.defaultSharpness = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "sharpness is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BACKLIGHT_COMPENSATION:
      case V4L2_CID_CHROMA_AGC:
      case V4L2_CID_COLOR_KILLER:
      case V4L2_CID_COLORFX:
        break;

      case V4L2_CID_AUTOBRIGHTNESS:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_AUTO_BRIGHTNESS ] = 1;
          cameraState.defaultAutoBrightness = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HUE_AUTO is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BAND_STOP_FILTER:
      case V4L2_CID_ROTATE:
      case V4L2_CID_BG_COLOR:
      case V4L2_CID_CHROMA_GAIN:
      case V4L2_CID_ILLUMINATORS_1:
      case V4L2_CID_ILLUMINATORS_2:
      case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE: // FIX ME -- use this one?
      case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
      case V4L2_CID_ALPHA_COMPONENT:
      case V4L2_CID_COLORFX_CBCR:
        break;
    }
  }

  for ( id = V4L2_CID_CAMERA_CLASS_BASE; id <= V4L2_CID_AUTO_FOCUS_RANGE;
      id++ ) {
    CLEAR ( ctrl );
    ctrl.id = id;

    if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_QUERYCTRL, &ctrl )) {
      if ( EINVAL != errno ) {
        fprintf ( stderr, "VIDIOC_QUERYCTRL( %x ) failed, errno %d\n", id,
            errno );
        continue;
      }
    }

    // returning 0 as the type here is not helpful
    if ( !ctrl.type ) {
      continue;
    }

    // FIX ME -- it's a bit of a pain to work through these one at a
    // time, but without doing so we could end up with a big mess with
    // the gui controls

    switch ( id ) {

      case V4L2_CID_EXPOSURE_AUTO:
        // This one should always be a menu for V4L2
        if ( V4L2_CTRL_TYPE_MENU == ctrl.type ) {
          camera->controls[ OA_CTRL_AUTO_EXPOSURE ] = 1;
          cameraState.defaultAutoExposure = ctrl.default_value;
        } else {
          fprintf ( stderr, "AUTO_EXPOSURE control type is not MENU (%d)\n",
              ctrl.type );
        }
        break;

      case V4L2_CID_EXPOSURE_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_EXPOSURE_ABSOLUTE ] = 1;
          cameraState.minAbsoluteExposure = ctrl.minimum;
          cameraState.maxAbsoluteExposure = ctrl.maximum;
          cameraState.stepAbsoluteExposure = ctrl.step;
          cameraState.defaultAbsoluteExposure = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "absolute exposure is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_RELATIVE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_PAN_RELATIVE ] = 1;
          cameraState.minPanRelative = ctrl.minimum;
          cameraState.maxPanRelative = ctrl.maximum;
          cameraState.stepPanRelative = ctrl.step;
          cameraState.defaultPanRelative = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan relative is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_RELATIVE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_TILT_RELATIVE ] = 1;
          cameraState.minTiltRelative = ctrl.minimum;
          cameraState.maxTiltRelative = ctrl.maximum;
          cameraState.stepTiltRelative = ctrl.step;
          cameraState.defaultTiltRelative = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt relative is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_RESET:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_PAN_RESET ] = 1;
          cameraState.defaultPanReset = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan reset is not BOOLEAN (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_RESET:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CTRL_TILT_RESET ] = 1;
          cameraState.defaultTiltReset = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt reset is not BOOLEAN (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_PAN_ABSOLUTE ] = 1;
          cameraState.minPanAbsolute = ctrl.minimum;
          cameraState.maxPanAbsolute = ctrl.maximum;
          cameraState.stepPanAbsolute = ctrl.step;
          cameraState.defaultPanAbsolute = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_TILT_ABSOLUTE ] = 1;
          cameraState.minTiltAbsolute = ctrl.minimum;
          cameraState.maxTiltAbsolute = ctrl.maximum;
          cameraState.stepTiltAbsolute = ctrl.step;
          cameraState.defaultTiltAbsolute = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_FOCUS_ABSOLUTE:
      case V4L2_CID_FOCUS_RELATIVE:
      case V4L2_CID_FOCUS_AUTO:
        break;

      case V4L2_CID_ZOOM_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CTRL_ZOOM_ABSOLUTE ] = 1;
          cameraState.minZoomAbsolute = ctrl.minimum;
          cameraState.maxZoomAbsolute = ctrl.maximum;
          cameraState.stepZoomAbsolute = ctrl.step;
          cameraState.defaultZoomAbsolute = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "zoom absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_ZOOM_CONTINUOUS:
      case V4L2_CID_PRIVACY:
      case V4L2_CID_IRIS_ABSOLUTE:
      case V4L2_CID_IRIS_RELATIVE:
      case V4L2_CID_AUTO_EXPOSURE_BIAS:
      case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
      case V4L2_CID_WIDE_DYNAMIC_RANGE:
      case V4L2_CID_IMAGE_STABILIZATION:
      case V4L2_CID_ISO_SENSITIVITY:
      case V4L2_CID_ISO_SENSITIVITY_AUTO:
      case V4L2_CID_EXPOSURE_METERING:
      case V4L2_CID_SCENE_MODE:
      case V4L2_CID_3A_LOCK:
      case V4L2_CID_AUTO_FOCUS_START:
      case V4L2_CID_AUTO_FOCUS_STOP:
      case V4L2_CID_AUTO_FOCUS_STATUS:
      case V4L2_CID_AUTO_FOCUS_RANGE:
        break;
    }
  }

  // Ok, now we need to find out what frame formats are supported and
  // which one we want to use

  // FIX ME
  // support colour pixel formats V4L2_PIX_FMT_RGB24, V4L2_PIX_FMT_YUYV
  // support mono pixel formats V4L2_PIX_FMT_Y16, V4L2_PIX_FMT_GREY
  // in those preferred orders

  cameraState.videoRGB24 = cameraState.videoYUYV = cameraState.videoGrey16 =
      cameraState.videoYUV420 = cameraState.videoGrey = 0;
  cameraState.videoCurrent = 0;
  cameraState.colour = 0;
  int bestGreyscale = 0;
  int haveNonEmulatedColour = 0;
  for ( id = 0;; id++ ) {
    CLEAR ( formatDesc );
    formatDesc.index = id;
    formatDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_ENUM_FMT, &formatDesc )) {
      if ( EINVAL != errno) {
        perror("VIDIOC_ENUM_FORMAT");
        continue;
      }
      break;
    }

//fprintf(stderr,"pixelformat = %s\n", formatDesc.description );

    if ( cameraState.colourDxK21 ) {
      switch ( formatDesc.pixelformat ) {
        case V4L2_PIX_FMT_SBGGR8:
        cameraState.videoCurrent = V4L2_PIX_FMT_SBGGR8;
        cameraState.videoBGGR8 = 1;
        cameraState.colour = 1;
        cameraState.raw = 1;
        if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
          haveNonEmulatedColour = 1;
        }
        break;
      }

    } else {

      switch ( formatDesc.pixelformat ) {

        case V4L2_PIX_FMT_RGB24:
          cameraState.videoCurrent = V4L2_PIX_FMT_RGB24;
          cameraState.videoRGB24 = 1;
          cameraState.colour = 1;
          if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
            haveNonEmulatedColour = 1;
          }
          break;

        case V4L2_PIX_FMT_YUV420:
          if ( cameraState.videoCurrent != V4L2_PIX_FMT_RGB24 ) {
            cameraState.videoCurrent = V4L2_PIX_FMT_YUV420;
            cameraState.videoYUV420 = 1;
            cameraState.colour = 1;
            if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
              haveNonEmulatedColour = 1;
            }
          }
          break;

        case V4L2_PIX_FMT_YUYV:
          if ( cameraState.videoCurrent != V4L2_PIX_FMT_YUV420 &&
              cameraState.videoCurrent != V4L2_PIX_FMT_RGB24 ) {
            cameraState.videoCurrent = V4L2_PIX_FMT_YUYV;
            cameraState.videoYUYV = 1;
            cameraState.colour = 1;
            if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
              haveNonEmulatedColour = 1;
            }
          }
          break;

        case V4L2_PIX_FMT_Y16:
          bestGreyscale = V4L2_PIX_FMT_Y16;
          if ( cameraState.videoCurrent != V4L2_PIX_FMT_RGB24 &&
              cameraState.videoCurrent != V4L2_PIX_FMT_YUYV &&
              cameraState.videoCurrent != V4L2_PIX_FMT_YUV420 ) {
            cameraState.videoCurrent = V4L2_PIX_FMT_Y16;
            cameraState.videoGrey16 = 1;
          }
          break;

        case V4L2_PIX_FMT_GREY:
          if ( !bestGreyscale ) {
            bestGreyscale = V4L2_PIX_FMT_GREY;
          }
          if ( !cameraState.videoCurrent ) {
            cameraState.videoCurrent = V4L2_PIX_FMT_GREY;
            cameraState.videoGrey = 1;
          }
          break;
      }
    }
  }

  // if the only colour modes are emulated then we look for a suitable
  // greyscale mode

  if ( !haveNonEmulatedColour ) {
//fprintf(stderr,"have no native colour frame formats\n");
    switch ( bestGreyscale ) {
      case V4L2_PIX_FMT_Y16:
        cameraState.videoCurrent = V4L2_PIX_FMT_Y16;
        cameraState.videoGrey16 = 1;
        cameraState.videoGrey = 0;
        cameraState.videoRGB24 = cameraState.videoYUYV = 0;
        cameraState.videoYUV420 = cameraState.colour = 0;
      case V4L2_PIX_FMT_GREY:
        cameraState.videoCurrent = V4L2_PIX_FMT_GREY;
        cameraState.videoGrey = 1;
        cameraState.videoGrey16 = 0;
        cameraState.videoRGB24 = cameraState.videoYUYV = 0;
        cameraState.videoYUV420 = cameraState.colour = 0;
    }
  }

  if ( !cameraState.videoCurrent ) {
    fprintf ( stderr, "No suitable video format found on %s", deviceName );
    free (( void* ) camera );
    return 0;
  }

  // Put the camera into the current video mode.  Ignore the frame size
  // for now.  That will have to be sorted later by the caller

  CLEAR ( format );
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = 1;
  format.fmt.pix.height = 1;
  format.fmt.pix.pixelformat = cameraState.videoCurrent;
  format.fmt.pix.field = V4L2_FIELD_NONE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_S_FMT, &format )) {
    perror ( "VIDIOC_S_FMT xioctl failed" );
    free (( void* ) camera );
    return 0;
  }

  if ( format.fmt.pix.pixelformat != cameraState.videoCurrent ) {
    fprintf ( stderr, "Can't set required video format in %s.\n", __func__);
    free (( void* ) camera );
    return 0;
  }

  for ( j = 0; j < OA_MAX_RESOLUTIONS; j++ ) {
    CLEAR ( fsize );
    fsize.index = j;
    fsize.pixel_format = cameraState.videoCurrent;
    if ( -1 == v4l2ioctl ( cameraState.fd, VIDIOC_ENUM_FRAMESIZES, &fsize )) {
      if ( EINVAL == errno) {
        break;
      } else {
        perror("VIDIOC_ENUM_FRAMESIZES failed");
      }
    }
    // FIX ME -- we can't handle mixed frame types here
    if ( V4L2_FRMSIZE_TYPE_DISCRETE == fsize.type ) {
      cameraState.frameSizing = OA_FRAMESIZES_DISCRETE;
      cameraState.resolutions[j].x = fsize.discrete.width;
      cameraState.resolutions[j].y = fsize.discrete.height;
    } else {
      fprintf ( stderr, "Can't handle framesizing type %d\n", fsize.type );
    }
  }

  if ( OA_FRAMESIZES_DISCRETE == cameraState.frameSizing ) {
    cameraState.resolutions[j].x = 0;
    cameraState.resolutions[j].y = 0;
  }

  cameraState.buffers = 0;
  cameraState.configuredBuffers = 0;
  cameraState.frameRateSupport = 0;

  CLEAR( parm );
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_G_PARM, &parm )) {
    if ( errno != EINVAL ) {
      perror ( "VIDIOC_G_PARM v4l2ioctl failed" );
      return 0;
    }
  }
  if ( V4L2_CAP_TIMEPERFRAME == parm.parm.capture.capability ) {
    CLEAR( parm );
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = 1;
    cameraState.frameRateSupport = 1;
    if ( v4l2ioctl ( cameraState.fd, VIDIOC_S_PARM, &parm )) {
      perror ( "VIDIOC_S_PARM v4l2ioctl failed" );
      return 0;
    }
  } else {
    cameraState.frameRateSupport = 0;
  }

  return camera;
}
