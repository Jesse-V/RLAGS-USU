/*****************************************************************************
 *
 * camera.h -- class declaration
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

#pragma once

#include "pthread.h"
#include "QtGui"

extern "C" {
#include "oacam.h"
}

class Camera : public QObject
{
  Q_OBJECT

  public:
    			Camera();
    			~Camera();
    int			listConnected ( oaDevice** );
    int			initialise ( oaDevice* );
    void		disconnect ( void );

    int			start ( void );
    void		stop ( void );
    void		releaseImageData ( void );

    int			has16Bit ( void );
    int			hasBinning ( int );
    int			hasControl ( int );
    int			hasFixedFrameSizes ( void );
    int			hasFixedFrameRates ( int, int );
    int			hasFrameRateSupport ( void );
    int			hasTemperature ( void );
    int			isColour ( void );
    int			isInitialised ( void );

    void		controlRange ( int, int*, int*, int*, int* );
    const char*		name ( void );
    FRAMESIZE*		frameSizes ( void );
    FRAMERATE*		frameRates ( int, int );
    void		delayFrameRateChanges ( void );
    int			captureRunning ( void );
    int			videoFramePixelFormat ( void );

    int			setControl ( int, int );
    int			setROI ( int, int );
    int			setFrameInterval ( int, int );
    float		getTemperature ( void );
    int			setBitDepth ( int );
    int			getPixelFormatForBitDepth ( int );

  signals:
    void		frameGrabbed ( void*, int );

  private:
    void		setConfigUpdateFlag ( int );
    int			clearConfigUpdateFlag ( int );
    int			testConfigFlag ( int );
    void		flushSettings ( void );
    static void*	capture ( void* );

    oaCamera*		handle;
    int			initialised;
    pthread_t		captureThread;
    pthread_mutex_t	configMutex;
    pthread_cond_t	stopCapture;
    int			configUpdated;
    unsigned int	configFlags[2];
    int			configValues[OA_CTRL_LAST_P1];
    int			configROIX;
    int			configROIY;
    int			configFPSNumerator;
    int			configFPSDenominator;
    int			captureThreadStarted;
    int			captureThreadExit;
    int			captureThreadDataInUse;
    int			frameTime;
    int			frameRateChangeDelayed;
    int			framePixelFormat;
};
