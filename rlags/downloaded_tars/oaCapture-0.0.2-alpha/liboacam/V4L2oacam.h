/*****************************************************************************
 *
 * V4L2oacam.h -- header for V4L2 camera API
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

#ifndef OA_V4L2_H
#define OA_V4L2_H

#define	SYS_V4L_PATH		"/sys/class/video4linux"

extern int		oaV4L2GetCameras ( oaDevice** );
extern oaCamera*	oaV4L2InitCamera ( oaDevice* );
extern void             oaV4L2CloseCamera ( oaCamera* );
extern int              oaV4L2CameraStart ( oaCamera*, START_PARMS* );
extern int              oaV4L2CameraReset ( oaCamera* );

extern void		oaV4L2CameraStartReadFrame ( oaCamera*, int );
extern int		oaV4L2CameraReadFrame ( oaCamera*, void** );
extern void		oaV4L2CameraFinishReadFrame ( oaCamera* );

extern int		oaV4L2CameraHas16Bit ( oaCamera* );
extern int		oaV4L2CameraHas16Binning ( oaCamera*, int );
extern int		oaV4L2CameraHasFixedFrameSizes ( oaCamera* );
extern int		oaV4L2CameraHasFixedFrameRates ( oaCamera*, int, int );
extern int		oaV4L2CameraHasFrameRateSupport ( oaCamera* );
extern int		oaV4L2CameraHasTemperature ( oaCamera* );
extern int		oaV4L2CameraIsColour ( oaCamera* );

extern void		oaV4L2CameraGetControlRange ( oaCamera*, int,
				int*, int*, int*, int* );
extern FRAMESIZE*	oaV4L2CameraGetFrameSizes ( oaCamera* );
extern FRAMERATE*	oaV4L2CameraGetFrameRates ( oaCamera*, int, int );
extern const char*	oaV4L2CameraGetName ( oaCamera* );
extern int              oaV4L2CameraGetFramePixelFormat ( oaCamera* );
extern float            oaV4L2CameraGetTemperature ( oaCamera* );
extern int              oaV4L2CameraGetPixelFormatForBitDepth ( oaCamera*, int );

extern int		oaV4L2CameraSetControl ( oaCamera*, int, int );
extern int		oaV4L2CameraSetROI ( oaCamera*, int, int );
extern int		oaV4L2CameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaV4L2CameraSetBitDepth ( oaCamera*, int );

#endif	/* OA_V4L2_H */
