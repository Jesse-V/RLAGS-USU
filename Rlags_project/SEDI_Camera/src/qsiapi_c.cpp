/******************************************************************************/
/*                 C INTERFACE ROUTINES TO QSI C++ LIBRARY                    */
/*                                                                            */
/* Conversion from C to the QSI C++ library takes place here.                 */
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
#include <string>
#include <iostream>
#include <QSIError.h>
#include <qsiapi.h>

static const int MAXCAMERAS = 128;

using namespace std;

// Name of most recently called QSI function */

const char *QSI_func;     

// Declare a QSI camera object

QSICamera Q;

// Functions

// Set structured exceptions

extern "C" int QSICamera_put_UseStructuredExceptions (bool newVal)
{
	QSI_func = __func__;
	Q.put_UseStructuredExceptions (newVal);
	return 1;
}

// Camera connection and error handling

extern "C" int QSICamera_get_AvailableCameras (const char *serial[], 
											   const char *desc[], int *num)
{
	int result, n;
	std::string s[*num];
	std::string d[*num];
	QSI_func = __func__;
	try {Q.get_AvailableCameras (s, d, *num);}
	catch (...) {return 0;}
	n = *num;
    while (n--) {
	    serial[n] = s[n].c_str();
		desc[n] = d[n].c_str();
	}
	return 1;
}

extern "C" int QSICamera_put_SelectCamera (char *serialNum)
{
	QSI_func = __func__;
	try {Q.put_SelectCamera (serialNum);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_IsMainCamera (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_IsMainCamera (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_Connected (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_Connected (pVal);}
	catch (...) {return 0;}
	return 1;
}
					
extern "C" int QSICamera_put_Connected (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_Connected (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CameraState (QSICamera::CameraState *pVal)
{
	QSI_func = __func__;
	try {Q.get_CameraState (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_LastError (char *pVal)
{
	std::string ss;
	try {Q.get_LastError (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

// Get camera capability

extern "C" int QSICamera_get_Name (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_Name (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_ModelNumber (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_ModelNumber (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_Description (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_Description (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_SerialNumber (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_SerialNumber (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_DriverInfo (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_DriverInfo (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_FullWellCapacity (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_FullWellCapacity (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_MaxADU (long *pVal)
{
	QSI_func = __func__;
	try {Q.get_MaxADU (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_ElectronsPerADU (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_ElectronsPerADU (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_PixelSizeX (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_PixelSizeX (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_PixelSizeY (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_PixelSizeY (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CameraXSize (long *pVal)
{
	QSI_func = __func__;
	try {Q.get_CameraXSize (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CameraYSize (long *pVal)
{
	QSI_func = __func__;
	try {Q.get_CameraYSize (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_MaxBinX (short *pVal)
{
	QSI_func = __func__;
	try {Q.get_MaxBinX (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_MaxBinY (short *pVal)
{
	QSI_func = __func__;
	try {Q.get_MaxBinY (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_MinExposureTime (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_MinExposureTime (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_MaxExposureTime (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_MaxExposureTime (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanAsymmetricBin (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanAsymmetricBin (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanGetCoolerPower (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanGetCoolerPower (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanPulseGuide (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanPulseGuide (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanSetCCDTemperature (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanSetCCDTemperature (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanAbortExposure (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanAbortExposure (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CanStopExposure (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CanStopExposure (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_HasFilterWheel (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_HasFilterWheel (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_HasShutter (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_HasShutter (pVal);}
	catch (...) {return 0;}
	return 1;
}

// Set exposure data

extern "C" int QSICamera_put_StartX (long newVal)
{
	QSI_func = __func__;
	try {Q.put_StartX (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_StartY (long newVal)
{
	QSI_func = __func__;
	try {Q.put_StartY (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_NumX (long newVal)
{
	QSI_func = __func__;
	try {Q.put_NumX (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_NumY (long newVal)
{
	QSI_func = __func__;
	try {Q.put_NumY (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_BinX (long newVal)
{
	QSI_func = __func__;
	try {Q.put_BinX (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_BinY (long newVal)
{
	QSI_func = __func__;
	try {Q.put_BinY (newVal);}
	catch (...) {return 0;}
	return 1;
}

// Exposure control and status

extern "C" int QSICamera_StartExposure (double Duration, bool Light)
{
	QSI_func = __func__;
	try {Q.StartExposure (Duration, Light);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_AbortExposure (void)
{
	QSI_func = __func__;
	try {Q.AbortExposure ();}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_StopExposure (void)
{
	QSI_func = __func__;
	try {Q.StopExposure ();}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_ImageReady (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_ImageReady (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_LastExposureDuration (double* pVal)
{
	QSI_func = __func__;
	try {Q.get_LastExposureDuration (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_LastExposureStartTime (char *pVal)
{
	std::string ss;
	QSI_func = __func__;
	try {Q.get_LastExposureStartTime (ss);}
	catch (...) {return 0;}
	strcpy (pVal, ss.c_str ());
	return 1;
}

extern "C" int QSICamera_get_ImageArraySize (int &xSize, int &ySize, 
											 int &elementSize)
{
	QSI_func = __func__;
	try {Q.get_ImageArraySize (xSize, ySize, elementSize);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_ImageArray (unsigned short *pVal)
{
	QSI_func = __func__;
	try {Q.get_ImageArray (pVal);}
	catch (...) {return 0;}
	return 1;
}

// Cooling

extern "C" int QSICamera_put_CoolerOn (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_CoolerOn (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CoolerOn (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_CoolerOn (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_FanMode (QSICamera::FanMode newVal)
{
	QSI_func = __func__;
	try {Q.put_FanMode (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_FanMode (QSICamera::FanMode &pVal)
{
	QSI_func = __func__;
	try {Q.get_FanMode (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_SetCCDTemperature (double newVal)
{
	QSI_func = __func__;
	try {Q.put_SetCCDTemperature (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CoolerPower (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_CoolerPower (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_HeatSinkTemperature (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_HeatSinkTemperature (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_CCDTemperature (double *pVal)
{
	QSI_func = __func__;
	try {Q.get_CCDTemperature (pVal);}
	catch (...) {return 0;}
	return 1;
}

// Filters

extern "C" int QSICamera_get_FilterCount (int &count)
{
	QSI_func = __func__;
	try {Q.get_FilterCount (count);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_Position (short *pVal)
{
	QSI_func = __func__;
	try {Q.get_Position (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_Position (int newVal)
{
	QSI_func = __func__;
	try {Q.put_Position (newVal);}
	catch (...) {return 0;}
	return 1;
}

//extern "C" int QSICamera_put_Names (const char *s[], int num)
//{
//	int i;
//	std::string names[num];
//	QSI_func = __func__;
//	for (i = 0; i < num; i++) if (s[i]) names[i].assign (s[i]);
//	try {Q.put_Names (names);}
//  catch (...) {return 0;}
//	return 1;
//}

// Exposure control

extern "C" int QSICamera_get_ShutterPriority (QSICamera::ShutterPriority *pVal)
{
	QSI_func = __func__;
	try {Q.get_ShutterPriority (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_ShutterPriority (QSICamera::ShutterPriority newVal)
{
	QSI_func = __func__;
	try {Q.put_ShutterPriority (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_ManualShutterMode (bool *pVal)
{
	QSI_func = __func__;
	try {Q.get_ManualShutterMode (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_ManualShutterMode (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_ManualShutterMode (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_ManualShutterOpen (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_ManualShutterOpen (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_get_PreExposureFlush (QSICamera::PreExposureFlush *pVal)
{
	QSI_func = __func__;
	try {Q.get_PreExposureFlush (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_PreExposureFlush (QSICamera::PreExposureFlush newVal)
{
	QSI_func = __func__;
	try {Q.put_PreExposureFlush (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_HostTimedExposure (bool newVal)
{
	QSI_func = __func__;
	try {Q.put_HostTimedExposure (newVal);}
	catch (...) {return 0;}
	return 1;
}

// Miscellaneous

extern "C" int QSICamera_get_CameraGain (QSICamera::CameraGain *pVal)
{
	QSI_func = __func__;
	try {Q.get_CameraGain (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_CameraGain (QSICamera::CameraGain newVal)
{
	QSI_func = __func__;
	try {Q.put_CameraGain (newVal);}
	catch (...) {return 0;}
	return 1;
}

#ifdef HAVE_READOUT_SPEED
extern "C" int QSICamera_get_ReadoutSpeed (QSICamera::ReadoutSpeed &pVal)
{
	QSI_func = __func__;
	try {Q.get_ReadoutSpeed (pVal);}
	catch (...) {return 0;}
	return 1;
}
#endif

#ifdef HAVE_READOUT_SPEED
extern "C" int QSICamera_put_ReadoutSpeed (QSICamera::ReadoutSpeed newVal)
{
	QSI_func = __func__;
	try {Q.put_ReadoutSpeed (newVal);}
	catch (...) {return 0;}
	return 1;
}
#endif

extern "C" int QSICamera_get_AntiBlooming (QSICamera::AntiBloom *pVal)
{
	QSI_func = __func__;
	try {Q.get_AntiBlooming (pVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_put_AntiBlooming (QSICamera::AntiBloom newVal)
{
	QSI_func = __func__;
	try {Q.put_AntiBlooming (newVal);}
	catch (...) {return 0;}
	return 1;
}

extern "C" int QSICamera_PulseGuide (QSICamera::GuideDirections Direction, 
									 long Duration)
{
	QSI_func = __func__;
	try {Q.PulseGuide (Direction, Duration);}
	catch (...) {return 0;}
	return 1;
}
#endif
