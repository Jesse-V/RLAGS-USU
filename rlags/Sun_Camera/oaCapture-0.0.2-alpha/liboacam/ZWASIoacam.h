/*****************************************************************************
 *
 * ZWASIoacam.h -- header for ZW ASI camera API
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

#ifndef OA_ZWASI_H
#define OA_ZWASI_H

extern int		oaZWASIGetCameras ( oaDevice** );
extern oaCamera*	oaZWASIInitCamera ( oaDevice* );
extern void             oaZWASICloseCamera ( oaCamera* );
extern int              oaZWASICameraStart ( oaCamera*, START_PARMS* );
extern int              oaZWASICameraReset ( oaCamera* );

extern void		oaZWASICameraStartReadFrame ( oaCamera*, int );
extern int		oaZWASICameraReadFrame ( oaCamera*, void** );
extern void		oaZWASICameraFinishReadFrame ( oaCamera* );

extern int		oaZWASICameraHas16Bit ( oaCamera* );
extern int		oaZWASICameraHas16Binning ( oaCamera*, int );
extern int		oaZWASICameraHasFixedFrameSizes ( oaCamera* );
extern int		oaZWASICameraHasFixedFrameRates ( oaCamera*, int, int );
extern int		oaZWASICameraHasFrameRateSupport ( oaCamera* );
extern int		oaZWASICameraHasTemperature ( oaCamera* );
extern int		oaZWASICameraIsColour ( oaCamera* );

extern void		oaZWASICameraGetControlRange ( oaCamera*, int,
				int*, int*, int*, int* );
extern FRAMESIZE*	oaZWASICameraGetFrameSizes ( oaCamera* );
extern FRAMERATE*	oaZWASICameraGetFrameRates ( oaCamera*, int, int );
extern const char*	oaZWASICameraGetName ( oaCamera* );
extern int		oaZWASICameraGetFramePixelFormat ( oaCamera* );
extern float		oaZWASICameraGetTemperature ( oaCamera* );
extern int		oaZWASICameraGetPixelFormatForBitDepth ( oaCamera*, int );


extern int		oaZWASICameraSetControl ( oaCamera*, int, int );
extern int		oaZWASICameraSetROI ( oaCamera*, int, int );
extern int		oaZWASICameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaZWASICameraSetBitDepth ( oaCamera*, int );

#define	ZWO_ASI130MM	0
#define ZWO_ASI120MM	1
#define ZWO_ASI120MC	2
#define ZWO_ASI035MM	3
#define ZWO_ASI035MC	4
#define ZWO_ASI030MC	5
#define ZWO_NUM_CAMERAS	6

#endif	/* OA_ZWASI_H */
