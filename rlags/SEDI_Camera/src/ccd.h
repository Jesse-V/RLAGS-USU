/******************************************************************************/
/*             HEADER FILE FOR GENERIC CCD CAMERA DATA STRUCTURES             */
/*                                                                            */
/* Generic CCD camera data structures are defined here.                       */
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

#ifndef GOQAT_CCD_H
#define GOQAT_CCD_H

enum CamFans {                   /* Camera fan settings                       */
	CCD_FAN_OFF, 
	CCD_FAN_AUTO, 
	CCD_FAN_HIGH
};

enum CamSpeed {                  /* Camera read speed settings                */
	CCD_SPEED_LOW,
	CCD_SPEED_HIGH
};

enum CamState {                /* Camera state settings    */
	S_CANCOOL,                 /* User sets this if camera can cool (SX only) */
	S_FANS,                    /* FanMode                  */
	S_TEMP,                    /* SetCCDTemperature        */
	S_COOL,                    /* CoolerOn                 */
	S_PRIORITY,                /* ShutterPriority          */ 
	S_MODE,                    /* ManualShutterMode        */
	S_OPEN,                    /* ManualShutterOpen        */
	S_FLUSH,                   /* PreExposureFlush         */
	S_HOST,                    /* HostTimedExposure        */
	S_GAIN,                    /* CameraGain               */
	S_SPEED,                   /* ReadoutSpeed             */
	S_ABLOOM,                  /* AntiBlooming             */
	S_INVERT,                  /* Invert                   */
	S_FILTER                   /* Position                 */
};

struct ccd_capability {          /* CCD camera capabilities                   */
	short max_h;                 /* Max. h dimension of chip                  */
	short max_v;                 /* Max. v dimension of chip                  */
	short max_binh;              /* Max. h binning for chip                   */
	short max_binv;              /* Max. v binning for chip                   */
	short bitspp;                /* Bits per pixel (SX cameras only)          */
	long max_adu;                /* Max. ADU value                            */
	float min_exp;               /* Min. exposure time (s)                    */
	float max_exp;               /* Max. exposure time (s)                    */
	double max_well;             /* Max. well capacity                        */
	double e_adu;                /* Electrons per ADU                         */
	double pixsiz_h;             /* Pixel size (h direction)                  */
	double pixsiz_v;             /* Pixel size (v direction)                  */
	char camera_manf[256];       /* Camera manufacturer                       */
	char camera_name[256];       /* Camera name                               */
	char camera_desc[256];       /* Camera description                        */
	char camera_snum[256];       /* Camera serial number                      */
	char camera_dinf[256];       /* Camera driver information                 */
	int CanAsymBin;              /* TRUE if asymmetric binning is possible    */
	int CanGetCoolPower;         /* TRUE if can get cooler power              */
	int CanPulseGuide;           /* TRUE if can pulse guide                   */
	int CanSetCCDTemp;           /* TRUE if can set CCD temperature           */
	int CanAbort;                /* TRUE if exposure can be aborted           */
	int CanStop;                 /* TRUE if exposure can be stopped           */
	int HasFilterWheel;          /* TRUE if camera has filter wheel           */
	int HasShutter;              /* TRUE if camera has a mechanical shutter   */
	int IsInterlaced;            /* TRUE if chip interlaced (SX cameras only) */
	int IsColour;                /* TRUE if bayer matrix (SX cameras only)    */
};

struct ccd_state {               /* CCD camera state                          */
	short c_filter;              /* Current filter wheel position             */
	int d_fans;                  /* Default fan state                         */
	int c_fans;                  /* Current fan state                         */
	int shut_prior;              /* Shutter priority                          */
	int shut_mode;               /* Shutter mode                              */
	int shut_open;               /* Shutter state                             */
	int pre_flush;               /* Amount of pre-flushing                    */
	int host_timed;              /* Fast exposure mode (interline)            */
	int cam_gain;                /* Camera gain setting                       */
	int read_speed;              /* Camera readout speed                      */
	int anti_bloom;              /* Anti-blooming setting                     */
	int invert;                  /* 1 to invert image, 0 otherwise            */
	double c_amb;                /* Current ambient temperature               */	
	double d_ccd;                /* Default CCD temperature                   */
	double c_ccd;                /* Current CCD temperature                   */
	double c_power;              /* Current cooler power                      */
	char status[10];             /* Camera status                             */
	int CoolOnConnect;           /* TRUE if cooling to be on when connected   */
	int CoolState;               /* Current cooler state (TRUE if on)         */
};

#endif /* GOQAT_CCD_H */
