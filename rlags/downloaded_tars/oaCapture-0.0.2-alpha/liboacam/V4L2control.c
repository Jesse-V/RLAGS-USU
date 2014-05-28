/*****************************************************************************
 *
 * V4L2control.c -- control functions for V4L2 cameras
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libv4l2.h>

#include "oacam.h"
#include "V4L2oacam.h"
#include "v4l2ioctl.h"

#define	cameraState		camera->_v4l2
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))


int	_setUserControl ( int, int, int );
int	_setExtendedControl ( int, int, int );
int	_startCamera ( oaCamera*, int, int );

int
oaV4L2CameraSetControl ( oaCamera* camera, int control, int arg1 )
{
  switch ( control ) {
    case OA_CTRL_BRIGHTNESS:
      return _setUserControl ( cameraState.fd, V4L2_CID_BRIGHTNESS, arg1 );
      break;
    case OA_CTRL_CONTRAST:
      return _setUserControl ( cameraState.fd, V4L2_CID_CONTRAST, arg1 );
      break;
    case OA_CTRL_SATURATION:
      return _setUserControl ( cameraState.fd, V4L2_CID_SATURATION, arg1 );
      break;
    case OA_CTRL_HUE:
      return _setUserControl ( cameraState.fd, V4L2_CID_HUE, arg1 );
      break;
    case OA_CTRL_AUTO_WHITE_BALANCE:
      // We only care about turning white balance off here
      if ( !arg1 ) {
        arg1 = cameraState.autoWhiteBalanceOff;
      }
      return _setUserControl ( cameraState.fd, V4L2_CID_AUTO_WHITE_BALANCE,
          arg1 );
      break;
    case OA_CTRL_WHITE_BALANCE:
      return _setUserControl ( cameraState.fd, V4L2_CID_DO_WHITE_BALANCE,
          arg1 );
      break;
    case OA_CTRL_BLUE_BALANCE:
      return _setUserControl ( cameraState.fd, V4L2_CID_RED_BALANCE, arg1 );
      break;
    case OA_CTRL_RED_BALANCE:
      return _setUserControl ( cameraState.fd, V4L2_CID_BLUE_BALANCE, arg1 );
      break;
    case OA_CTRL_GAMMA:
      return _setUserControl ( cameraState.fd, V4L2_CID_GAMMA, arg1 );
      break;
    case OA_CTRL_EXPOSURE:
      return _setUserControl ( cameraState.fd, V4L2_CID_EXPOSURE, arg1 );
      break;
    case OA_CTRL_AUTOGAIN:
      return _setUserControl ( cameraState.fd, V4L2_CID_AUTOGAIN, arg1 );
      break;
    case OA_CTRL_GAIN:
      return _setUserControl ( cameraState.fd, V4L2_CID_GAIN, arg1 );
      break;
    case OA_CTRL_HFLIP:
      return _setUserControl ( cameraState.fd, V4L2_CID_HFLIP, arg1 );
      break;
    case OA_CTRL_VFLIP:
      return _setUserControl ( cameraState.fd, V4L2_CID_VFLIP, arg1 );
      break;
    case OA_CTRL_POWER_LINE_FREQ:
      return _setUserControl ( cameraState.fd, V4L2_CID_POWER_LINE_FREQUENCY,
          arg1 );
      break;
    case OA_CTRL_HUE_AUTO:
      return _setUserControl ( cameraState.fd, V4L2_CID_HUE_AUTO, arg1 );
      break;
    case OA_CTRL_WHITE_BALANCE_TEMP:
      return _setUserControl ( cameraState.fd,
          V4L2_CID_WHITE_BALANCE_TEMPERATURE, arg1 );
      break;
    case OA_CTRL_SHARPNESS:
      return _setUserControl ( cameraState.fd, V4L2_CID_SHARPNESS, arg1 );
      break;
    case OA_CTRL_BACKLIGHT_COMPENSATION:
      return _setUserControl ( cameraState.fd,
          V4L2_CID_BACKLIGHT_COMPENSATION, arg1 );
      break;
    case OA_CTRL_CHROMA_AGC:
      return _setUserControl ( cameraState.fd, V4L2_CID_CHROMA_AGC, arg1 );
      break;
    case OA_CTRL_COLOUR_KILLER:
      return _setUserControl ( cameraState.fd, V4L2_CID_COLOR_KILLER, arg1 );
      break;
    case OA_CTRL_COLOURFX:
      return _setUserControl ( cameraState.fd, V4L2_CID_COLORFX, arg1 );
      break;
    case OA_CTRL_AUTO_BRIGHTNESS:
      return _setUserControl ( cameraState.fd, V4L2_CID_BRIGHTNESS, arg1 );
      break;
    case OA_CTRL_BAND_STOP_FILTER:
      return _setUserControl ( cameraState.fd, V4L2_CID_AUTOBRIGHTNESS,
          arg1 );
      break;
    case OA_CTRL_ROTATE:
      return _setUserControl ( cameraState.fd, V4L2_CID_ROTATE, arg1 );
      break;
    case OA_CTRL_BG_COLOUR:
      return _setUserControl ( cameraState.fd, V4L2_CID_BG_COLOR, arg1 );
      break;
    case OA_CTRL_CHROMA_GAIN:
      return _setUserControl ( cameraState.fd, V4L2_CID_CHROMA_GAIN, arg1 );
      break;
    case OA_CTRL_MIN_BUFFERS_FOR_CAPTURE:
      return _setUserControl ( cameraState.fd,
          V4L2_CID_MIN_BUFFERS_FOR_CAPTURE, arg1 );
      break;
    case OA_CTRL_ALPHA_COMPONENT:
      return _setUserControl ( cameraState.fd, V4L2_CID_ALPHA_COMPONENT,
          arg1 );
      break;
    case OA_CTRL_COLOURFX_CBCR:
      return _setUserControl ( cameraState.fd, V4L2_CID_COLORFX_CBCR, arg1 );
      break;

    // end of the standard V4L2 controls.  Now the extended ones

    case OA_CTRL_AUTO_EXPOSURE:
      // We're not currently going to worry about shutter priority or
      // aperture priority here
      return _setExtendedControl ( cameraState.fd, V4L2_CID_EXPOSURE_AUTO,
          arg1 ? V4L2_EXPOSURE_AUTO : V4L2_EXPOSURE_MANUAL );
      break;     
    case OA_CTRL_EXPOSURE_ABSOLUTE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_EXPOSURE_ABSOLUTE,
        arg1 );
      break;
    case OA_CTRL_PAN_RELATIVE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_PAN_RELATIVE,
        arg1 );
      break;
    case OA_CTRL_TILT_RELATIVE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_TILT_RELATIVE,
        arg1 );
      break;
    case OA_CTRL_PAN_RESET:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_PAN_RESET,
        arg1 );
      break;
    case OA_CTRL_TILT_RESET:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_TILT_RESET,
        arg1 );
      break;
    case OA_CTRL_PAN_ABSOLUTE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_PAN_ABSOLUTE,
        arg1 );
      break;
    case OA_CTRL_TILT_ABSOLUTE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_TILT_ABSOLUTE,
        arg1 );
      break;
    case OA_CTRL_ZOOM_ABSOLUTE:
      return _setExtendedControl ( cameraState.fd, V4L2_CID_ZOOM_ABSOLUTE,
        arg1 );
      break;

    // end of supported extended controls.
    // the next lot are ones that aren't part of V4L2

    case OA_CTRL_FPS:
      fprintf ( stderr, "oaV4L2CameraSetControl called with OA_CTRL_FPS\n" );
      fprintf ( stderr, "use oaV4L2CameraSetFrameInterval\n" );
      break;

    case OA_CTRL_ROI:
      fprintf ( stderr, "oaV4L2CameraSetControl called with OA_CTRL_ROI\n" );
      fprintf ( stderr, "use oaV4L2CameraSetROI\n" );
      break;
  }
  return -1;
}


int
_setUserControl ( int fd, int id, __s32 value )
{
  struct v4l2_control        control;

  CLEAR ( control );
  control.id = id;
  control.value = value;
  if ( v4l2ioctl ( fd, VIDIOC_S_CTRL, &control )) {
    perror ("VIDIOC_S_CTRL");
    return -1;
  }
  return 0;
}


int
_setExtendedControl ( int fd, int id, __s32 value )
{
  struct v4l2_ext_control    extControl[1];
  struct v4l2_ext_controls   controls;

  CLEAR ( controls );
  CLEAR ( extControl );
  controls.ctrl_class = V4L2_CTRL_ID2CLASS ( id );
  controls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
  controls.count = 1;
  controls.controls = extControl;
  extControl[0].id = id;
  extControl[0].value = value;
  if ( v4l2ioctl ( fd, VIDIOC_S_EXT_CTRLS, &controls )) {
    perror ("setExtendedCOntrol: VIDIOC_S_EXT_CTRLS");
    fprintf ( stderr, "fd = %d, id = %x, value = %d\n", fd, id, value );
    /*
    if ( EINVAL == errno ) {
      // try the standard control if this is a user class control
      if ( V4L2_CTRL_CLASS_USER == V4L2_CTRL_ID2CLASS ( id )) {
        return _setUserControl ( fd, id, value );
      }
    }
    */
    return -1;
  }
  return 0;
}


int
oaV4L2CameraSetROI ( oaCamera* camera, int x, int y )
{
  return 0;
}


int
oaV4L2CameraSetFrameInterval ( oaCamera* camera, int numerator,
    int denominator )
{
  struct v4l2_streamparm     parm;
  enum v4l2_buf_type         type;

  return 0;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_STREAMOFF, &type ) < 0 )  {
    perror ( "VIDIOC_STREAMOFF" );
  } 
  CLEAR( parm );
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
  parm.parm.capture.timeperframe.numerator = numerator;
  parm.parm.capture.timeperframe.denominator = denominator;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_S_PARM, &parm )) {
    perror ( "VIDIOC_S_PARM xioctl failed" );
  }
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_STREAMON, &type ) < 0 )  {
    if ( -ENOSPC == errno ) {
      fprintf ( stderr, "Insufficient bandwidth for camera on the USB bus\n" );
      return -1;
    }
    perror ( "VIDIOC_STREAMON" );
  } 
  return 0;
}


int
oaV4L2CameraStart ( oaCamera* camera, START_PARMS* parms )
{
  struct v4l2_format         fmt;
  struct v4l2_buffer         buf;
  struct v4l2_requestbuffers req;
  struct v4l2_streamparm     parm;
  enum v4l2_buf_type         type;
  __s32                      value, range;
  int                        m, n;

  CLEAR ( fmt );
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = parms->size.x;
  fmt.fmt.pix.height = parms->size.y;
  fmt.fmt.pix.pixelformat = cameraState.videoCurrent;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_S_FMT, &fmt )) {
    perror ( "VIDIOC_S_FMT v4l2ioctl failed" );
    return -1;
  }

  if ( fmt.fmt.pix.pixelformat != cameraState.videoCurrent ) {
    fprintf ( stderr, "Can't get expected video format: %c%c%c%c\n",
        cameraState.videoCurrent & 0xff, cameraState.videoCurrent >> 8 & 0xff,
        cameraState.videoCurrent >> 16 & 0xff, cameraState.videoCurrent >> 24 );
    return -1;
  }

  if (( fmt.fmt.pix.width != parms->size.x ) || ( fmt.fmt.pix.height !=
      parms->size.y )) {
    fprintf ( stderr, "Requested image size %dx%d, offered as %dx%d\n",
        parms->size.x, parms->size.y, fmt.fmt.pix.width, fmt.fmt.pix.height );
  }

  CLEAR( parm );
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_G_PARM, &parm )) {
    if ( errno != EINVAL ) {
      perror ( "VIDIOC_G_PARM v4l2ioctl failed" );
      return -1;
    }
  }
  if ( V4L2_CAP_TIMEPERFRAME == parm.parm.capture.capability ) {
    CLEAR( parm );
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    parm.parm.capture.timeperframe.numerator = parms->rate.numerator;
    parm.parm.capture.timeperframe.denominator = parms->rate.denominator;
    cameraState.frameRateSupport = 1;
    if ( v4l2ioctl ( cameraState.fd, VIDIOC_S_PARM, &parm )) {
      perror ( "VIDIOC_S_PARM v4l2ioctl failed" );
      return -1;
    }
  } else {
    cameraState.frameRateSupport = 0;
  }

  CLEAR( req );
  req.count = V4L2_BUFFERS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if ( v4l2ioctl( cameraState.fd, VIDIOC_REQBUFS, &req )) {
    perror ( "VIDIOC_REQBUFS v4l2ioctl failed" );
    return -1;
  }

  cameraState.configuredBuffers = 0;
  cameraState.lastUsedBuffer = -1;
  cameraState.buffers = calloc( req.count, sizeof ( struct buffer ));
  for ( n = 0;  n < req.count; n++ ) {
    CLEAR ( buf );
    buf.type = req.type;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n;
    if ( v4l2ioctl ( cameraState.fd, VIDIOC_QUERYBUF, &buf )) {
      perror ( "VIDIOC_QUERYBUF v4l2ioctl failed" );
      return -1;
    }
    cameraState.buffers[ n ].length = buf.length;
    if ( MAP_FAILED == ( cameraState.buffers[ n ].start = v4l2_mmap ( 0,
        buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cameraState.fd,
        buf.m.offset ))) {
      perror ( "mmap" );
      if ( n ) {
        for ( m = 0; m < n; m++ ) {
          v4l2_munmap ( cameraState.buffers[ m ].start,
              cameraState.buffers[ m ].length );
        }
      }
      return -1;
    }
    cameraState.configuredBuffers++;
  }

  for ( n = 0; n < req.count; n++ ) {
    CLEAR( buf );
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n;
    if ( v4l2ioctl ( cameraState.fd, VIDIOC_QBUF, &buf ) < 0 ) {
      perror ( "init VIDIOC_QBUF" );
      for ( m = 0; m < cameraState.configuredBuffers; m++ ) {
        v4l2_munmap ( cameraState.buffers[ m ].start,
            cameraState.buffers[ m ].length );
      }
      return -1;
    }
  }

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_STREAMON, &type ) < 0 )  {
    if ( -ENOSPC == errno ) {
      fprintf ( stderr, "Insufficient bandwidth for camera on the USB bus\n" );
    }
    perror ( "VIDIOC_STREAMON" );
    for ( m = 0; m < cameraState.configuredBuffers; m++ ) {
      v4l2_munmap ( cameraState.buffers[ m ].start,
          cameraState.buffers[ m ].length );
    }
    return -1;
  }

  return 0;
}


void
oaV4L2CameraStop ( oaCamera* camera )
{
  int                n;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if ( v4l2ioctl ( cameraState.fd, VIDIOC_STREAMOFF, &type ) < 0 ) {
    perror ( "VIDIOC_STREAMOFF" );
  }

  // FIX ME -- refactor buffer release

  if ( cameraState.configuredBuffers ) {
    for ( n = 0; n < cameraState.configuredBuffers; n++ ) {
      v4l2_munmap ( cameraState.buffers[ n ].start,
          cameraState.buffers[ n ].length );
    }
  }
}


int
oaV4L2CameraReset ( oaCamera* camera )
{
  // FIX ME -- should check here that buffers have been released etc.?

  v4l2_close ( cameraState.fd );
  cameraState.lastUsedBuffer = -1;
  if (( cameraState.fd = v4l2_open ( cameraState.devicePath,
      O_RDWR | O_NONBLOCK, 0 )) < 0 ) {
    fprintf ( stderr, "cannot open video device %s\n",
        cameraState.devicePath );
    // errno should be set?
    free (( void * ) camera );
    return 0;
  }

  return -1;
}


int
oaV4L2CameraSetBitDepth ( oaCamera *camera, int depth )
{
  fprintf ( stderr, "attempt to set depth of %d in V4L2\n", depth );
  return -1;
}

