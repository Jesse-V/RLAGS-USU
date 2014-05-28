/*****************************************************************************
 *
 * oacam.h -- camera API header
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

#ifndef OPENASTRO_CAMERA_H
#define OPENASTRO_CAMERA_H

#define OA_FRAMESIZES_DISCRETE		1
#define OA_FRAMESIZES_CONTINUOUS	2
#define OA_FRAMESIZES_STEPWISE		3

#define OA_FRAMERATES_DISCRETE		1
#define OA_FRAMERATES_CONTINUOUS	2
#define OA_FRAMERATES_STEPWISE		3

#define	OA_MAX_RESOLUTIONS		31
#define	OA_MAX_FRAMERATES		31

#define	OA_CTRL_BRIGHTNESS		1
#define	OA_CTRL_CONTRAST		2
#define	OA_CTRL_SATURATION		3
#define	OA_CTRL_HUE			4
#define	OA_CTRL_AUTO_WHITE_BALANCE	5
#define	OA_CTRL_WHITE_BALANCE		6
#define	OA_CTRL_BLUE_BALANCE		7
#define	OA_CTRL_RED_BALANCE		8
#define	OA_CTRL_GAMMA			9
#define	OA_CTRL_EXPOSURE		10
#define	OA_CTRL_AUTOGAIN		11
#define	OA_CTRL_GAIN			12
#define	OA_CTRL_HFLIP			13
#define	OA_CTRL_VFLIP			14
#define	OA_CTRL_POWER_LINE_FREQ		15
#define	OA_CTRL_HUE_AUTO		16
#define	OA_CTRL_WHITE_BALANCE_TEMP	17
#define	OA_CTRL_SHARPNESS		18
#define	OA_CTRL_BACKLIGHT_COMPENSATION	19
#define	OA_CTRL_CHROMA_AGC		20
#define	OA_CTRL_COLOUR_KILLER		21
#define	OA_CTRL_COLOR_KILLER		OA_CTRL_COLOUR_KILLER
#define	OA_CTRL_COLOURFX		22
#define	OA_CTRL_COLORFX			OA_CTRL_COLOURFX
#define	OA_CTRL_AUTO_BRIGHTNESS		23
#define	OA_CTRL_BAND_STOP_FILTER	24
#define	OA_CTRL_ROTATE			25
#define	OA_CTRL_BG_COLOUR		26
#define	OA_CTRL_BG_COLOR		OA_CTRL_BG_COLOUR
#define	OA_CTRL_CHROMA_GAIN		27
#define	OA_CTRL_MIN_BUFFERS_FOR_CAPTURE	28
#define	OA_CTRL_ALPHA_COMPONENT		29
#define	OA_CTRL_COLOURFX_CBCR		30
#define	OA_CTRL_COLORFX_CBCR		OA_CTRL_COLOURFX_CBCR
#define OA_CTRL_AUTO_EXPOSURE		31
#define OA_CTRL_EXPOSURE_ABSOLUTE	32
#define	OA_CTRL_PAN_RELATIVE		33
#define	OA_CTRL_TILT_RELATIVE		34
#define OA_CTRL_PAN_RESET		35
#define OA_CTRL_TILT_RESET		36
#define OA_CTRL_PAN_ABSOLUTE		37
#define OA_CTRL_TILT_ABSOLUTE		38
#define OA_CTRL_ZOOM_ABSOLUTE		39

#define	OA_CTRL_BACKLIGHT		40
#define	OA_CTRL_BLACKLEVEL		41
#define	OA_CTRL_FPS			42
#define	OA_CTRL_GAIN2X			43
#define	OA_CTRL_GAINBOOST		44
#define	OA_CTRL_HDR			45
#define	OA_CTRL_HPC			46
#define	OA_CTRL_HIGHSPEED		47
#define	OA_CTRL_LOWNOISE		48
#define	OA_CTRL_PIXELCLOCK		49
#define	OA_CTRL_RAW			50
#define	OA_CTRL_ROLLING_SHUTTER		51
#define	OA_CTRL_SHUTTER			52
#define	OA_CTRL_SIGNAL_BOOST		53
#define	OA_CTRL_SUBS_VOLTAGE		54
#define	OA_CTRL_TEMP_SETPOINT		55
#define	OA_CTRL_USBTRAFFIC		56
#define	OA_CTRL_ROI			57
#define	OA_CTRL_AUTO_GAMMA		58
#define	OA_CTRL_AUTO_RED_BALANCE	59
#define	OA_CTRL_AUTO_BLUE_BALANCE	60
#define	OA_CTRL_BINNING			61
#define	OA_CTRL_AUTO_USBTRAFFIC		62
#define	OA_CTRL_LAST_P1			OA_CTRL_AUTO_USBTRAFFIC+1


#define OA_MAX_NAME_LEN			40
#define OA_MAX_DEVICES			32

#define	OA_BIN_MODE_NONE		1
#define	OA_BIN_MODE_2x2			2
#define	OA_BIN_MODE_3x3			3
#define	OA_BIN_MODE_4x4			4

#define	OA_PIX_FMT_RGB24		1
#define	OA_PIX_FMT_BGR24		2
#define	OA_PIX_FMT_GREY8		3
#define	OA_PIX_FMT_GRAY8		3
#define	OA_PIX_FMT_GREY16BE		4
#define	OA_PIX_FMT_GRAY16BE		4
#define	OA_PIX_FMT_GREY16LE		5
#define	OA_PIX_FMT_GRAY16LE		5

enum oaInterfaceType {
  OA_IF_V4L2				= 1,
  OA_IF_PWC				= 2,
  OA_IF_ZWASI				= 3,
  OA_IF_QHY				= 4,
  OA_IF_PGR				= 5
};

typedef struct {
  int			x;
  int			y;
} FRAMESIZE;

typedef struct {
  int			numerator;
  int			denominator;
} FRAMERATE;

typedef struct {
  FRAMESIZE	size;
  FRAMERATE	rate;
} START_PARMS;

typedef struct {
  enum oaInterfaceType	interface;
  char			deviceName[OA_MAX_NAME_LEN+1];
  unsigned long		_devIndex;
  unsigned long		_devType;
} oaDevice;

// FIX ME -- ugly to include these here
#include "PGRstate.h"
#include "PWCstate.h"
#include "QHYstate.h"
#include "V4L2state.h"
#include "ZWASIstate.h"

typedef struct {
  enum oaInterfaceType  interface;
  char			deviceName[OA_MAX_NAME_LEN+1];
  unsigned char		controls[ OA_CTRL_LAST_P1 ];
  union {
    PGR_STATE		_pgr;
    PWC_STATE		_pwc;
    QHY_STATE		_qhy;
    V4L2_STATE		_v4l2;
    ZWASI_STATE		_zwasi;
  };
} oaCamera;

extern int		oaGetCameras ( oaDevice** );
extern oaCamera*	oaInitCamera ( oaDevice* );
extern void		oaCloseCamera ( oaCamera* );

extern int		oaCameraStart ( oaCamera*, START_PARMS* );
extern int		oaCameraStartWithROI ( oaCamera*, int, int );
extern int		oaCameraStop ( oaCamera* );
extern void		oaCameraStartReadFrame ( oaCamera*, int );
extern int		oaCameraReadFrame ( oaCamera*, void** );
extern void		oaCameraFinishReadFrame ( oaCamera* );
extern int		oaCameraReset ( oaCamera* );

extern int		oaCameraHas16Bit ( oaCamera* );
extern int		oaCameraHasBinning ( oaCamera*, int );
extern int		oaCameraHasControl ( oaCamera*, int );
extern int		oaCameraHasFixedFrameSizes ( oaCamera* );
extern int		oaCameraHasFixedFrameRates ( oaCamera*, int, int );
extern int		oaCameraHasFrameRateSupport ( oaCamera* );
extern int		oaCameraHasTemperature ( oaCamera* );
extern int		oaCameraIsColour ( oaCamera* );
extern int		oaCameraStartRequiresROI ( oaCamera* );

extern void		oaCameraGetControlRange ( oaCamera*, int, int*, int*,
                            int*, int* );
extern FRAMESIZE*	oaCameraGetFrameSizes ( oaCamera* );
extern FRAMERATE*	oaCameraGetFrameRates ( oaCamera*, int, int );
extern const char*	oaCameraGetName ( oaCamera* );
extern int		oaCameraGetFramePixelFormat ( oaCamera* );
extern float		oaCameraGetTemperature ( oaCamera* );
extern int		oaCameraGetPixelFormatForBitDepth ( oaCamera*, int );

extern int		oaCameraSetControl ( oaCamera*, int, int );
extern int		oaCameraSetROI ( oaCamera*, int, int );
extern int		oaCameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaCameraSetBitDepth ( oaCamera*, int );

#endif	/* OPENASTRO_CAMERA_H */
