/*****************************************************************************
 *
 * camera.cc -- camera interface class
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

#include <unistd.h>
#include <errno.h>

#include "camera.h"
#include "state.h"

#define	DEFAULT_FRAME_TIME	100

Camera::Camera()
{
  initialised = 0;
  captureThreadStarted = 0;
  configUpdated = 0;
  configFlags[0] = 0;
  configFlags[1] = 0;
  configROIX = configROIY = configFPSNumerator = configFPSDenominator = 0;
  pthread_mutex_init ( &configMutex, 0 );
  pthread_cond_init ( &stopCapture, 0 );
  frameTime = DEFAULT_FRAME_TIME;
  framePixelFormat = OA_PIX_FMT_RGB24; // the default
}


Camera::~Camera()
{
  stop();
  // FIX ME -- wait for mutex?
  disconnect();
  pthread_mutex_destroy ( &configMutex );
  pthread_cond_destroy ( &stopCapture );
}


int
Camera::has16Bit ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHas16Bit ( handle ) : 0;
}


int
Camera::hasBinning ( int factor )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasBinning ( handle, factor ) : 0;
}


int
Camera::hasFixedFrameSizes ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasFixedFrameSizes ( handle ) : 0;
}


int
Camera::hasFrameRateSupport ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasFrameRateSupport ( handle ) : 0;
}


int
Camera::hasFixedFrameRates ( int xRes, int yRes )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasFixedFrameRates ( handle, xRes, yRes ) : 0;
}


int
Camera::hasControl ( int control )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasControl ( handle, control ) : 0;
}


int
Camera::hasTemperature ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraHasTemperature ( handle ) : 0;
}


int
Camera::isColour ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraIsColour ( handle ) : 0;
}


float
Camera::getTemperature ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraGetTemperature ( handle ) : 0;
}


const char*
Camera::name()
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraGetName ( handle ) : "";
}


void
Camera::controlRange ( int control, int* min, int* max, int* step, int* def )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  } else {
    return oaCameraGetControlRange ( handle, control, min, max, step, def );
  }
  return;
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
FRAMESIZE*
Camera::frameSizes ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraGetFrameSizes ( handle ) : 0;
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
FRAMERATE*
Camera::frameRates ( int xRes, int yRes)
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? oaCameraGetFrameRates ( handle, xRes, yRes ) : 0;
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
int
Camera::listConnected ( oaDevice** devs )
{
  return oaGetCameras ( devs );
}


int
Camera::initialise ( oaDevice* device )
{
  disconnect();
  if (( handle = oaInitCamera ( device ))) {
    initialised = 1;
    framePixelFormat = oaCameraGetFramePixelFormat ( handle );
  }
  return handle ? 1 : 0;
}


void
Camera::disconnect ( void )
{
  if ( initialised ) {
    oaCloseCamera ( handle );
    initialised = 0;
    captureThreadStarted = 0;
    frameTime = DEFAULT_FRAME_TIME;
  }
}


int
Camera::setControl ( int control, int value )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }

  // Some cameras get upset if we try to change the controls before
  // the camera is fully configured so we just store them here
  // for later use (or when the capture thread gets around to looking
  // at them
  // The exception to this rule is binning, which is forced to the
  // camera straight away because it can affect the information we
  // subsequently want back from the camera library.

  configValues[ control ] = value;
  if ( OA_CTRL_BINNING == control ) {
    oaCameraSetControl ( handle, control, value );
  } else {
    pthread_mutex_lock ( &configMutex );
    setConfigUpdateFlag ( control );
    if ( OA_CTRL_EXPOSURE_ABSOLUTE == control ) {
      frameTime = value;
    }
    pthread_mutex_unlock ( &configMutex );
  }
  return 0;
}


int
Camera::setROI ( int x, int y )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  int ret = 0;
  int restart = captureThreadStarted;
  stop();
  configROIX = x;
  configROIY = y;
  flushSettings();
  oaCameraReset ( handle );
  if ( restart ) {
    ret = start();
  }
  frameRateChangeDelayed = 0;
  return ret;
}


int
Camera::setFrameInterval ( int numerator, int denominator )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  int restart = 0;
  if ( !frameRateChangeDelayed ) {
    restart = captureThreadStarted;
    stop();
  }
  if ( denominator ) {
    frameTime = 1000 * (( float ) numerator / ( float ) denominator );
  }
  configFPSNumerator = numerator;
  configFPSDenominator = denominator;
  if ( !frameRateChangeDelayed ) {
    oaCameraReset ( handle );
    if ( restart ) {
      return start();
    }
  }
  return 0;
}


void
Camera::setConfigUpdateFlag ( int control )
{
  control--;
  int i = control / 32;
  control %= 32;
  if ( i < 2 ) {
    configFlags[i] |= 1 << control;
    configUpdated = 1;
  }
}


int
Camera::clearConfigUpdateFlag ( int control )
{
  control--;
  int i = control / 32;
  control %= 32;
  if ( i < 2 ) {
    configFlags[i] &= ( 0xffffffff ^ ( 1 << control ));
  }
  return ( 0 == configFlags[0] && 0 == configFlags[1] ) ? 0 : 1;
}


int
Camera::testConfigFlag ( int control )
{
  int res = 0, i;

  control--;
  i = control / 32;
  control %= 32;
  if ( i < 2 ) {
    res = configFlags[i] & ( 1 << control );
  }
  return res ? 1 : 0;
}

int
Camera::start ( void )
{
  int		ret;
  START_PARMS	parms;
  
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }
  parms.size.x = configROIX;
  parms.size.y = configROIY;
  parms.rate.numerator = configFPSNumerator;
  parms.rate.denominator = configFPSDenominator;

  if (( ret = oaCameraStart ( handle, &parms ))) {
    return ret;
  }

  // now set each control that we have flagged as changed

  flushSettings();

  // now create a new thread and continue

  captureThreadStarted = 0;
  captureThreadExit = 0;
  captureThreadDataInUse = 1;
  pthread_create ( &captureThread, 0, &Camera::capture, this );

  captureThreadStarted = 1;

  return 0;
}


void
Camera::stop ( void )
{
  // We don't care if the camera isn't initialised for this function.
  // Nothing will happen anyhow

  if ( !captureThreadStarted ) {
    return;
  }

  pthread_mutex_lock ( &configMutex );
  captureThreadExit = 1;
  pthread_cond_signal ( &stopCapture );
  pthread_mutex_unlock ( &configMutex );
  pthread_join ( captureThread, 0 );
  captureThreadStarted = 0;

  oaCameraStop ( handle );
}


void*
Camera::capture ( void *param )
{
  int		quitThread;
  Camera*	camera = ( Camera* ) param;
  void*		imageBuffer;

  while ( true ) {
    pthread_mutex_lock ( &camera->configMutex );
    quitThread = camera->captureThreadExit;
    pthread_mutex_unlock ( &camera->configMutex );
    if ( quitThread ) {
      break;
    }

    oaCameraStartReadFrame ( camera->handle, camera->frameTime );

    while ( true ) {
      int ret;
      ret = oaCameraReadFrame ( camera->handle, &imageBuffer );
      // -1 == error, 0 = timeout, > 0 ok
      if ( -1 == ret ) {
        if ( errno != EINTR ) {
          break;
        }
      } else {
        if ( ret ) {
          emit camera->frameGrabbed ( imageBuffer, ret );

          pthread_mutex_lock ( &camera->configMutex );
          while ( camera->captureThreadDataInUse &&
              !camera->captureThreadExit ) {
            pthread_cond_wait ( &camera->stopCapture, &camera->configMutex );
          }
          camera->captureThreadDataInUse = 1;
          pthread_mutex_unlock ( &camera->configMutex );
          oaCameraFinishReadFrame ( camera->handle );
        }
        break;
      }
    }

    // process any configuration updates

    pthread_mutex_lock ( &camera->configMutex );
    if ( camera->configUpdated ) {
      int c, r = 0;
      for ( c = 1; 0 == r && ( c < OA_CTRL_LAST_P1 ); c++ ) {
        if ( camera->testConfigFlag ( c )) {
          oaCameraSetControl ( camera->handle, c, camera->configValues[c] );
          r = camera->clearConfigUpdateFlag ( c );
        }
      }
/*
      camera->flushSettings();
*/
      camera->configUpdated = 0;
    }
    pthread_mutex_unlock ( &camera->configMutex );
  }
  return 0;
}


void
Camera::releaseImageData ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  if ( captureThreadStarted ) {
    pthread_mutex_lock ( &configMutex );
    captureThreadDataInUse = 0;
    pthread_cond_signal ( &stopCapture );
    pthread_mutex_unlock ( &configMutex );
  }
}


void
Camera::delayFrameRateChanges ( void )
{
  frameRateChangeDelayed = 1;
}


int
Camera::captureRunning ( void )
{
  return captureThreadStarted;
}


int
Camera::videoFramePixelFormat ( void )
{
  return framePixelFormat;
}


void
Camera::flushSettings ( void )
{
  int c;
  for ( c = 1; c < OA_CTRL_LAST_P1; c++ ) {
    if ( testConfigFlag ( c )) {
      oaCameraSetControl ( handle, c, configValues[c] );
      clearConfigUpdateFlag ( c );
    }
  }
  configUpdated = 0;
}


int
Camera::setBitDepth ( int depth )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  int ret = 0;
  int restart = captureThreadStarted;
  stop();
  oaCameraSetBitDepth ( handle, depth );
  flushSettings();
  oaCameraReset ( handle );
  framePixelFormat = oaCameraGetFramePixelFormat ( handle );
  if ( restart ) {
    ret = start();
  }
  return ret;
}


int
Camera::getPixelFormatForBitDepth ( int depth )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }
  return oaCameraGetPixelFormatForBitDepth ( handle, depth );
}


int
Camera::isInitialised ( void )
{
  return initialised;
}
