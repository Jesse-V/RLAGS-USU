/******************************************************************************/
/*                  HIGH LEVEL INTERFACE FUNCTIONS TO QSI API                 */
/*                                                                            */
/* All the routines for interfacing with the QSI API are contained in         */
/* this module. The C to C++ conversion takes place in qsiapi_c.cpp.          */
/*                                                                            */
/* Copyright (C) 2009 - 2013  Edward Simonson                                 */
/*                                                                            */
/* This file is part of GoQat.                                                */
/*                                                                            */
/* GoQat is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 3 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, see <http://www.gnu.org/licenses/> .      */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBQSIAPI
#define HAVE_QSI 1
#endif

#ifdef HAVE_QSI

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccd.h"
#include "telescope.h"

#define TRUE  1
#define FALSE 0

enum CameraState
{
	CameraIdle 		= 0,	/* At idle state, available to start exposure     */
	CameraWaiting 	= 1,	/* Exposure started but waiting (e.g shutter etc) */
	CameraExposing 	= 2,	/* Exposure currently in progress                 */    
	CameraReading	= 3,	/* CCD array is being read out (digitized)        */
	CameraDownload 	= 4,	/* Downloading data to PC                         */
	CameraError 	= 5		/* Camera error condition: no further operations  */
};

enum GuideDirections
{
	guideNorth 		= 0,
	guideSouth 		= 1,
	guideEast 		= 2,
	guideWest 		= 3
};

enum FanMode
{
	fanOff			= 0,
	fanQuiet		= 1,
	fanFull			= 2
};

enum FlushCycles
{
	FlushZero		= 0,
	FlushOne		= 1,
	FlushTwo		= 2,
	FlushFour		= 3,
	FlushEight		= 4
};

enum ShutterMode
{
	ShutterMechanical = 0,
	ShutterElectronic = 1
};

enum PreExposureFlush
{
	FlushNone = 0,		
	FlushModest = 1,
	FlushNormal = 2,
	FlushAggressive = 3,
	FlushVeryAggressive = 4
};

enum ShutterPriority
{
	ShutterPriorityMechanical = 0,
	ShutterPriorityElectronic = 1
};

enum CameraGain 
{
	CameraGainHigh = 0,
	CameraGainLow = 1
};

enum ReadoutSpeed
{
	HighImageQuality = 0,
	FastReadout = 1
};

enum AntiBloom 
{
	AntiBloomNormal = 0,
	AntiBloomHigh = 1
};

extern const char *QSI_func;     /* Name of most recently called QSI function */

static int CanGetCoolPower = FALSE;
static int CanSetCCDTemp = FALSE;
static int HasFilterWheel = FALSE;

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

int qsi_get_cameras (const char *serial[], const char *desc[], int *num);
int qsi_connect (int connect, const char *serial);
int qsi_get_cap (struct ccd_capability *cam_cap);
int qsi_set_state (enum CamState state, int ival, double dval, ...);
int qsi_get_state (struct ccd_state *state, int AllSettings, ...);
int qsi_set_imagearraysize (long x, long y, long x_wid, long y_wid, long x_bin, 
						    long y_bin);
int qsi_start_exposure (char *dateobs, double length, int light);
int qsi_cancel_exposure (void);
int qsi_interrupt_exposure (void);
int qsi_get_imageready (int *ready);
int qsi_get_exposuretime (char *dateobs, double *length);
int qsi_get_imagearraysize (int *x_wid, int *y_wid, int *bytes);
int qsi_get_imagearray (unsigned short *array);
void qsi_pulseguide (enum TelMotion direction, int duration); 
static int QSI_STATUS (int OK);
static void (*error_func) (int *err, const char *func, char *msg);
void qsi_error_func (void (*err_func) (int *err, const char *func, char *msg ));


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

int qsi_get_cameras (const char *serial[], const char *desc[], int *num)
{
	/* Find all the connected cameras.  On entry, 'num' is the maximum
	 * number of cameras to search for.  On return, 'num' is the actual
	 * number found.
	 */
	
	return (QSI_STATUS (QSICamera_get_AvailableCameras (serial, desc, num)));
}  

int qsi_connect (int connect, const char *serial)
{
	/* Connect/disconnect the camera */
	
	int Connected = 0;
	
	if (connect) {
		if (!QSI_STATUS (QSICamera_put_UseStructuredExceptions (TRUE)))
			return FALSE;
		if (!QSI_STATUS (QSICamera_put_SelectCamera (serial)))
			return FALSE;
		if (!QSI_STATUS (QSICamera_get_Connected (&Connected)))
			return FALSE;
		if (!Connected) {
			if (!QSI_STATUS (QSICamera_put_IsMainCamera (TRUE)))
				return FALSE;
			if (!QSI_STATUS (QSICamera_put_Connected (TRUE)))
				return FALSE;
		}
	} else {
		if (!QSI_STATUS (QSICamera_get_Connected (&Connected)))
			return FALSE;
		if (Connected)
			if (!QSI_STATUS (QSICamera_put_Connected (FALSE)))
				return FALSE;
	}
	return TRUE;
}

int qsi_get_cap (struct ccd_capability *cam_cap)
{
	/* Get the camera capabilities */
	
	int i;
	double d;
	
	if (QSI_STATUS (QSICamera_get_Name (cam_cap->camera_name))) {
		QSI_STATUS (QSICamera_get_Description (cam_cap->camera_desc));
		QSI_STATUS (QSICamera_get_SerialNumber (cam_cap->camera_snum));
		QSI_STATUS (QSICamera_get_DriverInfo (cam_cap->camera_dinf));
		QSI_STATUS (QSICamera_get_FullWellCapacity (&cam_cap->max_well));
		QSI_STATUS (QSICamera_get_MaxADU (&cam_cap->max_adu));
		QSI_STATUS (QSICamera_get_ElectronsPerADU (&cam_cap->e_adu));
		QSI_STATUS (QSICamera_get_PixelSizeX (&cam_cap->pixsiz_h));
		QSI_STATUS (QSICamera_get_PixelSizeY (&cam_cap->pixsiz_v));
		QSI_STATUS (QSICamera_get_CameraXSize (&i));
		cam_cap->max_h = (short) i;
		QSI_STATUS (QSICamera_get_CameraYSize (&i));
		cam_cap->max_v = (short) i;
		QSI_STATUS (QSICamera_get_MaxBinX (&cam_cap->max_binh));
		QSI_STATUS (QSICamera_get_MaxBinY (&cam_cap->max_binv));
		QSI_STATUS (QSICamera_get_MinExposureTime (&d));
		cam_cap->min_exp = (float) d;
		QSI_STATUS (QSICamera_get_MaxExposureTime (&d));
		cam_cap->max_exp = (float) d;
		QSI_STATUS (QSICamera_get_CanAsymmetricBin (&cam_cap->CanAsymBin));
		QSI_STATUS (QSICamera_get_CanGetCoolerPower(&cam_cap->CanGetCoolPower));
		CanGetCoolPower = cam_cap->CanGetCoolPower;
		QSI_STATUS (QSICamera_get_CanPulseGuide(&cam_cap->CanPulseGuide));
		QSI_STATUS(QSICamera_get_CanSetCCDTemperature(&cam_cap->CanSetCCDTemp));
		CanSetCCDTemp = cam_cap->CanSetCCDTemp;
		QSI_STATUS (QSICamera_get_CanAbortExposure (&cam_cap->CanAbort));
		QSI_STATUS (QSICamera_get_CanStopExposure (&cam_cap->CanStop));
		QSI_STATUS (QSICamera_get_HasFilterWheel (&cam_cap->HasFilterWheel));
		HasFilterWheel = cam_cap->HasFilterWheel;
		QSI_STATUS (QSICamera_get_HasShutter (&cam_cap->HasShutter));
		
		return TRUE;
	}
	return FALSE;	
}

int qsi_set_state (enum CamState state, int ival, double dval, ...)
{
	/* Set the camera state */
	
	switch (state) {
		case S_FANS:
			if (!QSI_STATUS (QSICamera_put_FanMode (ival)))
				return FALSE;
			break;
		case S_TEMP:
			if (!QSI_STATUS (QSICamera_put_SetCCDTemperature (dval)))
				return FALSE;
			break;
		case S_COOL:
			if (!QSI_STATUS (QSICamera_put_CoolerOn (ival)))
				return FALSE;
			break;
		case S_PRIORITY:
			if (!QSI_STATUS (QSICamera_put_ShutterPriority (ival)))
				return FALSE;
			break;
		case S_MODE:
			if (!QSI_STATUS (QSICamera_put_ManualShutterMode (ival)))
				return FALSE;
			break;
		case S_OPEN:
			if (!QSI_STATUS (QSICamera_put_ManualShutterOpen (ival)))
				return FALSE;
			break;
		case S_FLUSH:
			if (!QSI_STATUS (QSICamera_put_PreExposureFlush (ival)))
				return FALSE;
			break;
		case S_HOST:
			if (!QSI_STATUS (QSICamera_put_HostTimedExposure (ival)))
				return FALSE;
			break;
		case S_GAIN:
			if (!QSI_STATUS (QSICamera_put_CameraGain (ival)))
				return FALSE;
			break;
		case S_SPEED:
			#ifdef HAVE_READOUT_SPEED
			if (!QSI_STATUS (QSICamera_put_ReadoutSpeed (ival)))
				return FALSE;
		    #endif
			break;
		case S_ABLOOM:
			if (!QSI_STATUS (QSICamera_put_AntiBlooming (ival)))
				return FALSE;
			break;
		case S_FILTER:
			if (!QSI_STATUS (QSICamera_put_Position (ival)))
				return FALSE;
			break;
		default:
			break;
	}
	return TRUE;
}

int qsi_get_state (struct ccd_state *state, int AllSettings, ...)
{
	/* Check and return the camera state */
	
	int status, OK;
	
	if (QSI_STATUS (QSICamera_get_CameraState (&status))) {
		switch (status) {
			case CameraIdle:
				strcpy (state->status, "Idle");
				break;
			case CameraWaiting:
				strcpy (state->status, "Waiting");
				break;
			case CameraExposing:
				strcpy (state->status, "Exposing");
				break;
			case CameraReading:
				strcpy (state->status, "Reading");
				break;
			case CameraDownload:
				strcpy (state->status, "Download");
				break;
			case CameraError:
				strcpy (state->status, "ERROR");
				break;
			default:
				strcpy (state->status, "Unknown");
				break;
		}
	} else
		return FALSE;
	
	if (!QSI_STATUS (QSICamera_get_CoolerOn (&state->CoolState)))
		return FALSE;
	
	if (!QSI_STATUS (QSICamera_get_FanMode (&state->c_fans)))
		return FALSE;
	
	if (CanGetCoolPower) {
		if (!QSI_STATUS (QSICamera_get_CoolerPower (&state->c_power)))
			return FALSE;
	}
	
	if (CanSetCCDTemp) {
		if (!QSI_STATUS (QSICamera_get_HeatSinkTemperature (&state->c_amb)))
			return FALSE;
		
		if (!QSI_STATUS (QSICamera_get_CCDTemperature (&state->c_ccd)))
			return FALSE;
	}
	
	if (HasFilterWheel) {
		if (!QSI_STATUS (QSICamera_get_Position (&state->c_filter)))
			return FALSE;
	}
	
	/* Return -1 rather than error for the following settings because some
	 * cameras may not support these functions, but that's OK.
	 */
	
	if (AllSettings) {
		if (!QSI_STATUS (QSICamera_get_ShutterPriority (&state->shut_prior)))
			state->shut_prior = -1;
		
		if (!QSI_STATUS (QSICamera_get_ManualShutterMode (&state->shut_mode)))
		    state->shut_mode = -1;
		
		/* No command to retrieve shutter open status - set it to -1 */
		state->shut_open = -1;
		
		if (!QSI_STATUS (QSICamera_get_PreExposureFlush (&state->pre_flush)))
		    state->pre_flush = -1;
		
		/* No command to retrieve host timed setting - set it to -1 */
		state->host_timed = -1;
			
		if (!QSI_STATUS (QSICamera_get_CameraGain (&state->cam_gain)))
		    state->cam_gain = -1;
		
		#ifdef HAVE_READOUT_SPEED
		if (!QSI_STATUS (QSICamera_get_ReadoutSpeed (&state->read_speed)))
			return FALSE;
		#else
		state->read_speed = -1;
		#endif
		
		if (!QSI_STATUS (QSICamera_get_AntiBlooming (&state->anti_bloom)))
		    state->anti_bloom = -1;
    }

	return TRUE;
}

int qsi_set_imagearraysize (long x, long y, long x_wid, long y_wid, long x_bin, 
						    long y_bin)
{
	/* Set the image array parameters.  Note that the starting x and y positions
	 * are in binned coordinates.
	 */

	if (!QSI_STATUS (QSICamera_put_StartX (x)))
		return FALSE;
	if (!QSI_STATUS (QSICamera_put_StartY (y)))
		return FALSE;
	if (!QSI_STATUS (QSICamera_put_NumX (x_wid)))
		return FALSE;
	if (!QSI_STATUS (QSICamera_put_NumY (y_wid)))
		return FALSE;
	if (!QSI_STATUS (QSICamera_put_BinX (x_bin)))
		return FALSE;
	if (!QSI_STATUS (QSICamera_put_BinY (y_bin)))
		return FALSE;
	
	return TRUE;
}

int qsi_start_exposure (char *dateobs, double length, int light)
{
	/* Start the exposure.  Set 'light' to TRUE if the shutter is to be
	 * opened, FALSE otherwise.
	 */
	
	return QSI_STATUS (QSICamera_StartExposure (length, light));
}

int qsi_cancel_exposure (void)
{
	/* Cancel the current exposure */

	return QSI_STATUS (QSICamera_AbortExposure ());
}

int qsi_interrupt_exposure (void)
{
	/* Interrupt the current exposure and initiate data reading */
	
	return QSI_STATUS (QSICamera_StopExposure ());
}

int qsi_get_imageready (int *ready)
{
	/* Test to see whether the downloaded data is available.  We don't invoke 
	 * QSI_STATUS in this routine because that involves a gtk call and this 
	 * routine is not called from the main gtk thread.
	 */
	
	return QSICamera_get_ImageReady (ready);
}

int qsi_get_exposuretime (char *dateobs, double *length)
{
	/* Get the actual exposure length, and the start time in FITS format */

	if (!QSI_STATUS (QSICamera_get_LastExposureStartTime (dateobs)))
		return FALSE;
	
	if (!QSI_STATUS (QSICamera_get_LastExposureDuration (length)))
		return FALSE;
	
	return TRUE;
}

int qsi_get_imagearraysize (int *x_wid, int *y_wid, int *bytes)
{
	/* Return the actual number of pixels read in the x and y directions, and
	 * the number of bytes per pixel.
	 */
	
	return QSI_STATUS (QSICamera_get_ImageArraySize (x_wid, y_wid, bytes));
}

int qsi_get_imagearray (unsigned short *array)
{
	/* Get the image data */
	
	return QSI_STATUS (QSICamera_get_ImageArray (array));
}

void qsi_pulseguide (enum TelMotion direction, int duration)
{
	/* Issue remotely timed guide pulse */
	
	if (direction & TM_NORTH)
		QSI_STATUS (QSICamera_PulseGuide (guideNorth, (long) duration));
	if (direction & TM_SOUTH)
		QSI_STATUS (QSICamera_PulseGuide (guideSouth, (long) duration));
	if (direction & TM_EAST)
		QSI_STATUS (QSICamera_PulseGuide (guideEast, (long) duration));
	if (direction & TM_WEST)
		QSI_STATUS (QSICamera_PulseGuide (guideWest, (long) duration));
	
	return;
}

static int QSI_STATUS (int OK)
{
	/* Prints QSI error message and returns FALSE if OK == FALSE */

	char c[80];
	
	if (OK) {
		error_func ((int *) 0, QSI_func, c);
		return TRUE;
	} else {
		QSICamera_get_LastError (c);
		error_func ((int *) 1, QSI_func, c);
		return FALSE;
	}
}

void qsi_error_func (void (*err_func) (int *err, const char *func, char *msg))
{
	/* Set the error function callback */
	
	error_func = err_func;
}
#endif /* HAVE_QSI */
