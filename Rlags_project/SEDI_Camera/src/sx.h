/******************************************************************************/
/*               HEADER FILE FOR STARLIGHT XPRESS HARDWARE                    */
/*                                                                            */
/* Header file for Starlight Xpress hardware.                                  */
/*                                                                            */
/* Copyright (C) 2012 - 2014  Edward Simonson                                 */
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

#ifdef HAVE_LIBUDEV
#define HAVE_SX_FILTERWHEEL 1
#endif

#ifdef HAVE_SX_FILTERWHEEL
#define SX_MAX_FILTERWHEELS 128
#endif

#ifdef HAVE_LIBUSB
#define HAVE_SX_CAM 1
#endif

#ifdef HAVE_SX_CAM

#include "gqusb.h"
#include "ccd.h"
#include "telescope.h"

#define SX_MAX_CAMERAS 128

struct imparam {
	unsigned int x;                          /* Image coordinates and binning */
	unsigned int y;
	unsigned int x_wid;
	unsigned int y_wid;
	unsigned int x_bin;
	unsigned int y_bin;
	unsigned int max_x;                      /* Maximum chip width            */
	unsigned int max_y;                      /* Maximum chip height           */
	double req_len;                          /* Requested exposure length     */
	double act_len;                          /* Actual exposure length        */
};

struct cooler {
	int CanSetCCDTemp;                       /* 1 if user can set CCD temp.   */
	int req_coolstate;                       /* Requested cooler state 1 = on */
	int act_coolstate;                       /* Actual cooler state    0 = off*/
	double req_temp;                         /* Requested ccd temperature     */
	double act_temp;                         /* Actual ccd temperature        */
};	

struct sx_cam {
	struct libusb_device *sxcams[SX_MAX_CAMERAS]; /* Detected cameras         */
	struct usbdevice usbd;                        /* Low-level USB data       */
	struct imparam ip;                            /* Image data               */
	struct cooler cool;                           /* Cooler data              */
	pthread_t sx_expose_thread;                   /* Camera exposure thread   */
	int idx[SX_MAX_CAMERAS];                 /* Index no. of detected cameras */
	short bitspp;                            /* Bits (not bytes!) per pixel   */
	int status;                                   /* Camera status            */
	int SXShutter;                                /* 1 if has shutter         */
	int SXInterlaced;                             /* 1 if interlaced chip     */
	int SXColour;                                 /* 1 if colour chip         */
	int InvertImage;                              /* 1 if image to be inverted*/
	int Expose;                                   /* 1 if exposure to be made */
	int ImageReady;                               /* 1 if image is ready      */
	unsigned short *e_buf;                        /* Memory buffer pointers   */
	unsigned short *o_buf;
	unsigned short *img_buf;
	unsigned char *all_buf;
};

extern int sx_get_cameras (const char *serial[], const char *desc[], int *num);
extern int sx_connect (int connect, const char *serial);
extern int sx_get_cap (struct ccd_capability *cam_cap);
extern int sx_set_state (enum CamState state, int ival, double dval, ...);
extern int sx_get_state (struct ccd_state *state, int AllSettings, ...);
extern int sx_set_imagearraysize (long x, long y, long x_wid, long y_wid,
								  long x_bin, long y_bin);
extern int sx_start_exposure (char *dateobs, double length, int light);
extern int sx_cancel_exposure (void);
extern int sx_interrupt_exposure (void);
extern int sx_get_imageready (int *ready);
extern int sx_get_exposuretime (char *dateobs, double *length);
extern int sx_get_imagearraysize (int *x_wid, int *y_wid, int *bytes);
extern int sx_get_imagearray (unsigned short *array);
extern void sx_pulseguide (enum TelMotion direction, int duration); 
extern void sx_guide_start (enum TelMotion direction);
extern void sx_guide_stop (enum TelMotion direction);
extern void sx_error_func (void (*err_func) (int *err, const char *func, 
											 char *msg ));
extern struct usbdevice sx_get_ccdcam_usbd (void);
											 
extern int sxc_get_cameras (struct sx_cam *cam, const char *serial[], 
							const char *desc[], int *num);
extern int sxc_connect (struct sx_cam *cam, int connect, const char *serial);
extern int sxc_get_cap (struct sx_cam *cam, struct ccd_capability *cam_cap);
extern int sxc_set_state (struct sx_cam *cam, enum CamState state, int ival, 
						  double dval);
extern int sxc_get_state (struct sx_cam *cam, struct ccd_state *state, 
						  int AllSettings);
extern int sxc_set_imagearraysize (struct sx_cam *cam, long x, long y, 
								   long x_wid, long y_wid, 
								   long x_bin, long y_bin);
extern int sxc_start_exposure (struct sx_cam *cam, char *dateobs, double length, 
							   int light);
extern int sxc_cancel_exposure (struct sx_cam *cam);
extern int sxc_interrupt_exposure (struct sx_cam *cam);
extern int sxc_get_imageready (struct sx_cam *cam, int *ready);
extern int sxc_get_exposuretime (struct sx_cam *cam, char *dateobs, 
								 double *length);
extern int sxc_get_imagearraysize (struct sx_cam *cam, int *x_wid, int *y_wid, 
								   int *bytes);
extern int sxc_get_imagearray (struct sx_cam *cam, unsigned short *array);
extern void sxc_pulseguide (enum TelMotion direction, int duration); 
extern void sxc_guide_start (enum TelMotion direction);
extern void sxc_guide_stop (enum TelMotion direction);
extern void sxc_set_guide_command_cam (struct sx_cam *cam);

#endif /* HAVE_SX_CAM */

#ifdef HAVE_SX_FILTERWHEEL

extern int sxf_get_filterwheels (const char *devnode[], const char *desc[], 
                                 int *num);
extern int sxf_connect (int connect, const char *devnode);
extern unsigned short sxf_get_filter_pos (void);
extern int sxf_set_filter_pos (unsigned short pos);

#endif /* HAVE_SX_FILTERWHEEL */

void sx_error_func (void (*err_func) (int *err, const char *func, char *msg ));



