/*****************************************************************************
 *
 * V4L2capture.c -- capture functions for V4L2 cameras
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
#include <sys/mman.h>
#include <libv4l2.h>

#include "oacam.h"
#include "V4L2oacam.h"
#include "v4l2ioctl.h"

#define cameraState             camera->_v4l2
#define CLEAR(x)                memset ( &(x), 0, sizeof ( x ))

int
oaV4L2CameraReadFrame ( oaCamera* camera, void** bufferPtr )
{
  struct v4l2_buffer	buf;
  struct timeval	tv;
  fd_set		fds;
  int			r;

  FD_ZERO ( &fds );
  FD_SET ( cameraState.fd, &fds );
  cameraState.bufferInUse = 0;
  tv.tv_sec = ( int ) ( cameraState.frameTime / 1000 );
  tv.tv_usec = ( cameraState.frameTime % 1000 ) * 1000;
  r = select ( cameraState.fd + 1, &fds, 0, 0, &tv );

  if ( -1 == r ) {
    perror ( "select" );
  }
  if ( r <= 0 ) {
    return 0;
  }

  if ( cameraState.lastUsedBuffer < 0 ) {
    cameraState.lastUsedBuffer = 0;
  }
  CLEAR( buf );
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = cameraState.lastUsedBuffer;
  if ( v4l2ioctl ( cameraState.fd, VIDIOC_DQBUF, &buf ) < 0 ) {
    perror ( "VIDIOC_DQBUF" );
    return -1;
  }

  *bufferPtr = cameraState.buffers[buf.index].start;
  cameraState.bufferInUse = 1;
  return buf.bytesused;
}


void
oaV4L2CameraStartReadFrame ( oaCamera* camera, int frameTime )
{
  cameraState.frameTime = frameTime;
  return;
}


void
oaV4L2CameraFinishReadFrame ( oaCamera* camera )
{
  struct v4l2_buffer	buf;

  if ( cameraState.bufferInUse ) {
    CLEAR( buf );
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = cameraState.lastUsedBuffer;
    if ( v4l2ioctl ( cameraState.fd, VIDIOC_QBUF, &buf )) {
      perror ( "finish read: VIDIOC_QBUF" );
    }
    cameraState.bufferInUse = 0;
    cameraState.lastUsedBuffer = ( cameraState.lastUsedBuffer + 1 ) %
        cameraState.configuredBuffers;
  }
}
