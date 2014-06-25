/******************************************************************************/
/*             INTERFACE FUNCTIONS TO STARLIGHT XPRESS HARDWARE               */
/*                                                                            */
/* All the routines for interfacing with SX hardware are contained in         */
/* this module.                                                               */
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

#define TRUE  1
#define FALSE 0

#define SX_VID 0x1278

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sx.h"

#ifdef HAVE_SX_FILTERWHEEL

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <libudev.h>

#endif /* HAVE_SX_FILTERWHEEL */

#ifdef HAVE_SX_CAM

#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#include "ccd.h"
#include "telescope.h"

#define SHORT_EXPOSURE 0.1

enum RowData {
	SXOddRows = 1,
	SXEvenRows,
	SXAllRows
};	

enum SXGuide {
	SXWest = 1,
	SXSouth = 2,
	SXNorth = 4,
	SXEast = 8,
	SXStop = 16,
	SXStopAll = 0
};

enum SXStatus
{
	SXIdle,
	SXWaiting,
	SXExposing,
	SXReading,
	SXDownload,
	SXError
};

enum CamType {
	CCD,
	AUG
};

int cam_pids[SX_MAX_CAMERAS] = {
				0x0507,
				0x0509,
				0x0517,
				0x0519,
				0x0100,
				0x0115,
				0x0119,
				0x0719,
				0x0720,
				0x0319,
				0x0126,
				0x0127,
				0x0128,
				0x0135,
				0x0136,
				0x0137,
				0x0139,
				0x0174,
				0x0374,
				0x0194,
				0x0394,
				0x0198,
				0x0398,
				0x0105,
				0x0107,
				0x0109,
				0x0305,
				0x0307,
				0x0308,
				0x0325,
				0x0326,
				0x0200,
				0                  /* Zero marks end of list */
};

const char cam_descriptions[SX_MAX_CAMERAS][15] = {
				"Lodestar",
				"Superstar",
				"CoStar",
				"Oculus",
				"SXVRH9",
				"SXVRH5",
				"SXVRH9",
				"LuntLSI",
				"LuntHLSI",
				"SXVRH9C",
				"SXVRH16",
				"SXVRH17",
				"SXVRH18",
				"SXVRH35",
				"SXVRH36",
				"SXVRH360",
				"SXVRH390",
				"SXVRH674",
				"SXVRH674C",
				"SXVRH694",
				"SXVRH694C",
				"SXVRH814",
				"SXVRH814C",
				"SXVFM5",
				"SXVFM7",
				"SXVRM9",
				"SXVRM5C",
				"SXVRM7C",
				"SXVRM8C",
				"SXVRM25C",
				"SXVRM26C",
				"SXVInterface"
};

struct sx_cam *guide_cam = NULL; /* Pointer to whichever camera is used for   */
                                 /*  issuing guide commands.                  */
struct sx_cam c_cam;      /* This represents the CCD camera.  It can't be     */
                          /* passed in via function calls because the function*/
                          /* prototypes need to match the ones defined by the */
                          /* QSI API (since I wrote that bit first!).         */

#endif /* HAVE_SX_CAM */

#ifdef HAVE_SX_FILTERWHEEL
                          
int fw_fd;                /* File descriptor for filter wheel device node     */

#endif /* HAVE_SX_FILTERWHEEL */

#ifdef HAVE_SX_CAM

/******************************************************************************/
/*    THE FOLLOWING FUNCTION PROTOTYPES MATCH THOSE DEFINED BY THE QSI API    */
/*                           FOR CAMERA CONTROL                               */
/******************************************************************************/

int sx_get_cameras (const char *serial[], const char *desc[], int *num);
int sx_connect (int connect, const char *serial);
int sx_get_cap (struct ccd_capability *cam_cap);
int sx_set_state (enum CamState state, int ival, double dval, ...);
int sx_get_state (struct ccd_state *state, int AllSettings, ...);
int sx_set_imagearraysize (long x, long y, long x_wid, long y_wid, long x_bin, 
						   long y_bin);
int sx_start_exposure (char *dateobs, double length, int light);
int sx_cancel_exposure (void);
int sx_interrupt_exposure (void);
int sx_get_imageready (int *ready);
int sx_get_exposuretime (char *dateobs, double *length);
int sx_get_imagearraysize (int *x_wid, int *y_wid, int *bytes);
int sx_get_imagearray (unsigned short *array);
void sx_pulseguide (enum TelMotion direction, int duration); 
/* The following functions are in addition to the QSI prototypes              */
void sx_guide_start (enum TelMotion direction);
void sx_guide_stop (enum TelMotion direction);

/******************************************************************************/
/* THE FOLLOWING FUNCTION PROTOTYPES MATCH THE QSI PROTOTYPES ABOVE BUT WITH  */
/* THE ADDITION OF A CAMERA POINTER AS THE FIRST PARAMETER IN MOST CASES.     */
/* THIS ENABLES US TO DISTINGUISH BETWEEN TWO SX CAMERAS USED SIMULTANEOUSLY  */
/* AS THE MAIN AND GUIDE CAMERAS.                                             */
/******************************************************************************/
 
int sxc_get_cameras (struct sx_cam *cam, const char *serial[], 
					 const char *desc[], int *num);
int sxc_connect (struct sx_cam *cam, int connect, const char *serial);
int sxc_get_cap (struct sx_cam *cam, struct ccd_capability *cam_cap);
int sxc_set_state (struct sx_cam *cam, enum CamState state, int ival, 
				   double dval);
int sxc_get_state (struct sx_cam *cam, struct ccd_state *state,int AllSettings);
int sxc_set_imagearraysize (struct sx_cam *cam, long x, long y, long x_wid, 
							long y_wid, long x_bin, long y_bin);
int sxc_start_exposure (struct sx_cam *cam, char *dateobs, double length, 
						int light);
int sxc_cancel_exposure (struct sx_cam *cam);
int sxc_interrupt_exposure (struct sx_cam *cam);
int sxc_get_imageready (struct sx_cam *cam, int *ready);
int sxc_get_exposuretime (struct sx_cam *cam, char *dateobs, double *length);
int sxc_get_imagearraysize (struct sx_cam *cam, int *x_wid, int *y_wid, 
							int *bytes);
int sxc_get_imagearray (struct sx_cam *cam, unsigned short *array);
void sxc_pulseguide (enum TelMotion direction, int duration); 
/* The following functions are in addition to the QSI prototypes              */
void sxc_guide_start (enum TelMotion direction);
void sxc_guide_stop (enum TelMotion direction);
void sxc_set_guide_command_cam (struct sx_cam *cam);

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

extern struct sx_cam *augcam_get_sx_cam_struct (void);
static int SX_STATUS (struct usbdevice *u_dev, int OK);
struct usbdevice sx_get_ccdcam_usbd (void);
static int sx_reset (struct usbdevice *u_dev);
static char *sx_get_firmware_version (struct usbdevice *u_dev);
static int sx_get_camera_info (struct usbdevice *u_dev, 
							   struct ccd_capability *cam_cap);
static int sx_open_shutter (struct usbdevice *u_dev, int HasShutter);
static int sx_close_shutter (struct usbdevice *u_dev, int HasShutter);
static int sx_dump_charge (struct usbdevice *u_dev);
static int sx_clear_vert (struct usbdevice *u_dev);
static int sx_get_row_data (struct usbdevice *u_dev, enum RowData rows, 
							unsigned char *buf, long bytes, 
							unsigned int x, unsigned int y, 
							unsigned int x_wid, unsigned int y_wid,
							unsigned int x_bin, unsigned int y_bin);
static void sx_guide (struct usbdevice *u_dev, unsigned short cmd);
static void sx_guide_pulse (struct usbdevice *u_dev, enum SXGuide cmd,
							int duration);
static int sx_set_cooling (struct usbdevice *u_dev, struct cooler *cool);
static void *sx_expose (void *data);
static void delay (struct usbdevice *u_dev, struct timespec *length);
static unsigned short int torben (unsigned short int m[], int n);

#endif /* HAVE_SX_CAM */

#ifdef HAVE_SX_FILTERWHEEL

/******************************************************************************/
/* THE FOLLOWING FUNCTION PROTOTYPES ARE FOR FILTER WHEEL CONTROL             */
/******************************************************************************/
 
int sxf_get_filterwheels (const char *devnode[], const char *desc[], int *num);
int sxf_connect (int connect, const char *devnode);
int sxf_set_filter_pos (unsigned short pos);
unsigned short sxf_get_filter_pos (void);

#endif /* HAVE_SX_FILTERWHEEL */

/******************************************************************************/
/*                           ERROR HANDLING                                   */
/******************************************************************************/

static void (*error_func) (int *err, const char *func, char *msg);
void sx_error_func (void (*err_func) (int *err, const char *func, char *msg ));

#ifdef HAVE_SX_CAM


/******************************************************************************/
/*         THE FOLLOWING FUNCTIONS MATCH THOSE DEFINED BY THE QSI API         */
/******************************************************************************/

int sx_get_cameras (const char *serial[], const char *desc[], int *num)
{
	return sxc_get_cameras (&c_cam, serial, desc, num);
}

int sx_connect (int connect, const char *serial)
{
	return sxc_connect (&c_cam, connect, serial);
}

int sx_get_cap (struct ccd_capability *cam_cap)
{
	return sxc_get_cap (&c_cam, cam_cap);
}

int sx_set_state (enum CamState state, int ival, double dval, ...)
{
	/* sx_set_state must always be called with a fourth argument to indicate
	 * whether this is for the main CCD camera or the autoguider camera.
	 */
	
	va_list ap;
	
    va_start (ap, dval);
	switch (va_arg (ap, int)) {
		case CCD:
			va_end (ap);
			return sxc_set_state (&c_cam, state, ival, dval);
			break;
		case AUG:
			va_end (ap);
			return sxc_set_state (augcam_get_sx_cam_struct (), state,ival,dval);
			break;
		default:
			break;
	}
	
	return FALSE;
}

int sx_get_state (struct ccd_state *state, int AllSettings, ...)
{
	/* sx_get_state must always be called with a third argument to indicate
	 * whether this is for the main CCD camera or the autoguider camera.
	 */
	
	va_list ap;
	
    va_start (ap, AllSettings);
    
	switch (va_arg (ap, int)) {
		case CCD:
			va_end (ap);
			return sxc_get_state (&c_cam, state, AllSettings);
			break;
		case AUG:
			va_end (ap);
			return sxc_get_state (augcam_get_sx_cam_struct(),state,AllSettings);
			break;
		default:
			break;
	}
	
	return FALSE;
}

int sx_set_imagearraysize (long x, long y, long x_wid, long y_wid, long x_bin, 
						   long y_bin)
{
	return sxc_set_imagearraysize (&c_cam, x, y, x_wid, y_wid, x_bin, y_bin);
}

int sx_start_exposure (char *dateobs, double length, int light)
{
	return sxc_start_exposure (&c_cam, dateobs, length, light);
}

int sx_cancel_exposure (void)
{
	return sxc_cancel_exposure (&c_cam);
}

int sx_interrupt_exposure (void)
{
	return sxc_interrupt_exposure (&c_cam);
}

int sx_get_imageready (int *ready)
{
	return sxc_get_imageready (&c_cam, ready);
}

int sx_get_exposuretime (char *dateobs, double *length)
{
	return sxc_get_exposuretime (&c_cam, dateobs, length);
}

int sx_get_imagearraysize (int *x_wid, int *y_wid, int *bytes)
{
	return sxc_get_imagearraysize (&c_cam, x_wid, y_wid, bytes);
}

int sx_get_imagearray (unsigned short *array)
{
	return sxc_get_imagearray (&c_cam, array);
}

void sx_pulseguide (enum TelMotion direction, int duration)
{
	sxc_pulseguide (direction, duration); 
}

void sx_guide_start (enum TelMotion direction)
{
	sxc_guide_start (direction);	
}

void sx_guide_stop (enum TelMotion direction)
{
	sxc_guide_stop (direction);	
}

/******************************************************************************/
/* THE FOLLOWING FUNCTIONS MATCH THOSE ABOVE BUT WITH THE ADDITION OF A       */
/* CAMERA POINTER AS THE FIRST PARAMETER.  THIS ENABLES US TO DISTINGUISH     */
/* BETWEEN TWO SX CAMERAS USED SIMULTANEOUSLY AS THE MAIN AND GUIDE CAMERAS.  */
/******************************************************************************/

int sxc_get_cameras (struct sx_cam *cam, const char *serial[], 
					 const char *desc[], int *num)
{
	/* Return a list of detected cameras and the total number found.  On entry, 
	 * 'num' is the maximum number of cameras to search for.  On return, 'num' 
	 * is the actual number found. 
	 */
	
	int i = 0, j = 0, k = 0, nd;
	int pids[SX_MAX_CAMERAS];
	static char s[SX_MAX_CAMERAS][15];
	
	*num = *num > SX_MAX_CAMERAS ? SX_MAX_CAMERAS : *num;
	if ((nd = gqusb_list_devices (SX_VID, *num, pids, cam->sxcams)) < 0)
		return FALSE;
		
	/* Note below that the synthetic serial number must be the index of the
	 * device in the pids array returned by gqusb_list_devices; otherwise GoQat
	 * may not find the correct device when it tries to open it.
	 */
		
	while (cam_pids[i++]) {
		for (k = 0; k < nd; k++) {
			if (cam_pids[i - 1] == pids[k]) {
				sprintf (s[j], "#%02d", k);  /* synthetic serial number */
				serial[j] = &s[j][0];
				desc[j] = cam_descriptions[i - 1];
				cam->idx[j] = i - 1;
				j++;
			}
		}
	}
	*num = j;
	
	return TRUE;
}

int sxc_connect (struct sx_cam *cam, int connect, const char *serial)
{
	/* Connect/disconnect the camera */
	
	if (connect) {
		if (cam->usbd.id == 0) { /* Only need to do this for main camera */
			cam->usbd.dev = cam->sxcams[strtol (serial+1, NULL, 10)];
			cam->usbd.idx = cam->idx[strtol (serial+1, NULL, 10)];
			if (!SX_STATUS (&cam->usbd, gqusb_open_device (&cam->usbd)))
				return FALSE;
		}
		sx_reset (&cam->usbd);
		cam->status = SXIdle;
	} else {
		gqusb_close_device (&cam->usbd);
		if (cam->all_buf) {
			free (cam->all_buf);
			cam->all_buf = NULL;
		}
		if (cam->e_buf) {
			free (cam->e_buf);
			cam->e_buf = NULL;
		}
		if (cam->o_buf) {
			free (cam->o_buf);
			cam->o_buf = NULL;
		}
		if (cam->img_buf) {
			free (cam->img_buf);
			cam->img_buf = NULL;
		}
	}
	
	return TRUE;
}

int sxc_get_cap (struct sx_cam *cam, struct ccd_capability *cam_cap)
{
	/* Get the camera capabilities and allocate memory for downloading and
	 * processing image data from the camera.
	 */
	
	long buf_size; 
	char *c;
	
	strcpy (cam_cap->camera_name, cam_descriptions[cam->usbd.idx]);
	//strcpy (cam_cap->camera_desc, ""); /* Already set by calling routine */
	cam_cap->max_well = 0;
	cam_cap->e_adu = 0;
	cam_cap->max_binh = 8;
	cam_cap->max_binv = 8;
	cam_cap->min_exp = 0.001;
	cam_cap->max_exp = 86400.0;
	cam_cap->CanAsymBin = 1;
	cam_cap->CanGetCoolPower = 0;  /* May be able to with recent models    */
	cam_cap->CanAbort = 1;
	cam_cap->CanStop = 1;
	cam_cap->HasFilterWheel = 0;
	if (cam->usbd.id == 0) {  /* Only need to do this for main camera */
		if ((c = sx_get_firmware_version (&cam->usbd)) == NULL)
			return FALSE;
		strcpy (cam_cap->camera_dinf, c);
	}
	if (!sx_get_camera_info (&cam->usbd, cam_cap))
		return FALSE;
	cam->ip.max_x = cam_cap->max_h;
	cam->ip.max_y = cam_cap->max_v;
	cam->cool.CanSetCCDTemp = cam_cap->CanSetCCDTemp;
	cam->bitspp = cam_cap->bitspp;
	cam->SXShutter = cam_cap->HasShutter;
	cam->SXInterlaced = cam_cap->IsInterlaced;
	cam->SXColour = cam_cap->IsColour;
	
	/* Assume a maximum of 2 bytes per pixel, i.e. 16-bit images */
	buf_size = 2 * cam_cap->max_h * cam_cap->max_v;
	
	/* Raw camera data for whole image as single byte chunks */
	if (cam->all_buf)
		free (cam->all_buf);
	cam->all_buf = (unsigned char *) malloc (buf_size);
	
	if (!cam_cap->IsInterlaced) {
		/* e_buf stores the whole image converted to 16-bit integers */
		if (cam->e_buf)
			free (cam->e_buf);
		cam->e_buf = (unsigned short *) malloc (buf_size);
	} else {
		/* Even and odd field data converted to 16-bit integers */
		if (cam->e_buf)
			free (cam->e_buf);
		cam->e_buf = (unsigned short *) malloc (buf_size / 2);
		if (cam->o_buf)
			free (cam->o_buf);
		cam->o_buf  = (unsigned short *) malloc (buf_size / 2);
	}
		
	/* Re-assembled data (if interlaced) and possibly flipped in some
	 * direction (interlaced or progressive) for whole image, converted to 
	 * 16-bit integers.
	 */
	if (cam->img_buf)
		free (cam->img_buf);
	cam->img_buf = (unsigned short *) malloc (buf_size);
	
	return TRUE;
}

int sxc_set_state (struct sx_cam *cam, enum CamState state, int ival, 
				   double dval)
{
	/* Set the camera state */
	
	switch (state) {
		case S_TEMP:
			/* Store the requested CCD temperature */
			cam->cool.req_temp = dval;
			break;
		case S_COOL:
			/* Turn cooling on or off */
			cam->cool.req_coolstate = ival;
			sx_set_cooling (&cam->usbd, &cam->cool);
			break;
		case S_INVERT:
			/* Set flag to invert image or not */
			cam->InvertImage = ival;
			break;
		default:
			break;
	}
	return TRUE;
}

int sxc_get_state (struct sx_cam *cam, struct ccd_state *state, int AllSettings)
{
	/* Check and return the camera state */
	
	if (cam->usbd.err)
		cam->status = SXError;
		
	switch (cam->status) {
		case SXIdle:
			strcpy (state->status, "Idle");
			break;
		case SXWaiting:
			strcpy (state->status, "Waiting");
			break;
		case SXExposing:
			strcpy (state->status, "Exposing");
			break;
		case SXReading:
			strcpy (state->status, "Reading");
			break;
		case SXDownload:
			strcpy (state->status, "Download");
			break;
		case SXError:
			strcpy (state->status, "ERROR");
			break;
		default:
			strcpy (state->status, "Unknown");
			break;
	}
	
	if (cam->cool.CanSetCCDTemp) {
		sx_set_cooling (&cam->usbd, &cam->cool);
		state->CoolState = cam->cool.act_coolstate;
		state->c_ccd = cam->cool.act_temp;
		state->c_amb = state->c_ccd;
	}

	return TRUE;
}

int sxc_set_imagearraysize (struct sx_cam *cam, long x, long y, long x_wid, 
							long y_wid, long x_bin, long y_bin)
{
	/* Store the image array parameters for this exposure.  If coordinates need
	 * adjusting so that the requested region of interest in DS9 matches the
	 * area returned from the camera, do that here.  The following works for 
	 * the Lodestar at least.
	 */
	
	cam->ip.x = (unsigned int) x;
	cam->ip.y = (unsigned int) y;
	cam->ip.x_wid = (unsigned int) x_wid;
	cam->ip.y_wid = (unsigned int) y_wid;
	cam->ip.x_bin = (unsigned int) x_bin;
	cam->ip.y_bin = (unsigned int) y_bin;
	
	/* For interlaced chips, the area to be exposed is specified for each field
	 * separately, so the y coordinates must be divided by two since each field
	 * has only half the number of rows of the entire chip.
	 */
	 
	if (cam->SXInterlaced) {
		cam->ip.y /= 2;
		cam->ip.y_wid /= 2;
	}
	
	return TRUE;
}

int sxc_start_exposure (struct sx_cam *cam, char *dateobs, double length, 
						int light)
{
	/* Start an exposure by creating an sx_expose thread */
	
	struct timeval tv;
	struct tm *dt;
	
	/* Set the UTC dateobs string in the format YYYY-MM-DDTHH:MM:SS.sss */
		
	gettimeofday (&tv, NULL);
	dt = gmtime (&tv.tv_sec);
	sprintf (dateobs, "%4i-%02i-%02iT%02i:%02i:%02i.%03i", 
			 1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
			 dt->tm_hour, dt->tm_min, dt->tm_sec, 
			 (int) ((double) tv.tv_usec / 1.0e3 + 0.5));
	
	/* Now start the exposure */
	 
	cam->ip.req_len = length;
	cam->Expose = TRUE;
	cam->ImageReady = FALSE;
	return (pthread_create(&cam->sx_expose_thread, NULL, &sx_expose, cam) == 0);
}

int sxc_cancel_exposure (struct sx_cam *cam)
{
	/* Cancel the current exposure.  Returns TRUE if an existing exposure was
	 * actually cancelled, or FALSE if the thread did not exist.
	 */
	
	if (!cam->sx_expose_thread)
		return TRUE;

	if (pthread_cancel (cam->sx_expose_thread) == 0) {
		pthread_join (cam->sx_expose_thread, NULL);
		cam->sx_expose_thread = 0;
		cam->status = SXIdle;
		return TRUE;
	} else {
		return FALSE;
	}
}

int sxc_interrupt_exposure (struct sx_cam *cam)
{
	/* Interrupt the current exposure and read the chip.  We do this by
	 * cancelling the existing thread and starting a new one with Expose set
	 * to FALSE.
	 */

	if (cam->status == SXIdle)
		return TRUE;
		
	if (pthread_cancel (cam->sx_expose_thread) == 0) {
		pthread_join (cam->sx_expose_thread, NULL);
		sx_clear_vert (&cam->usbd);
		cam->Expose = FALSE;
		return (pthread_create (&cam->sx_expose_thread, NULL,
								&sx_expose, cam) == 0);
	} else
		return FALSE;
}

int sxc_get_imageready (struct sx_cam *cam, int *ready)
{
	/* Returns TRUE when an image has been read from the camera */
	
	*ready = cam->ImageReady;
	if (*ready) {  /* Release thread resources */
		pthread_join (cam->sx_expose_thread, NULL);
		cam->sx_expose_thread = 0;
	}
		
	return TRUE;
}

int sxc_get_exposuretime (struct sx_cam *cam, char *dateobs, double *length)
{
	*length = cam->ip.act_len;
	
	return TRUE;
}

int sxc_get_imagearraysize (struct sx_cam *cam, int *x_wid, int *y_wid, 
							int *bytes)
{
	/* Return the actual image array size (may be less than requested after
	 * binning).
	 */
	 
	*x_wid = (int) (cam->ip.x_wid / cam->ip.x_bin);
	*y_wid = (int) (cam->ip.y_wid / cam->ip.y_bin);
	if (cam->SXInterlaced)
		*y_wid *= 2;
	*bytes = cam->bitspp > 8 ? 2 : 1;  /* Assume not greater than 16-bit */
	
	return TRUE;
}

int sxc_get_imagearray (struct sx_cam *cam, unsigned short *array)
{
	/* Return the image array data */
	
	int x_wid, y_wid;
	
	if (cam->img_buf) {
		x_wid = (int) (cam->ip.x_wid / cam->ip.x_bin);
		y_wid = (int) (cam->ip.y_wid / cam->ip.y_bin);
		if (cam->SXInterlaced)
			y_wid *= 2;
		memcpy (array, cam->img_buf, x_wid * y_wid * sizeof (unsigned short));
		return TRUE;
	} else
		return FALSE;
}

void sxc_pulseguide (enum TelMotion direction, int duration)
{
	/* Implement pulse guiding here (though not remotely timed by the camera) */
	
	if (guide_cam == NULL)
		return;
	
	if (direction & TM_NORTH)
		sx_guide_pulse (&guide_cam->usbd, SXNorth, duration);
	if (direction & TM_SOUTH)
		sx_guide_pulse (&guide_cam->usbd, SXSouth, duration);
	if (direction & TM_EAST)
		sx_guide_pulse (&guide_cam->usbd, SXEast, duration);
	if (direction & TM_WEST)
		sx_guide_pulse (&guide_cam->usbd, SXWest, duration);
}

void sxc_guide_start (enum TelMotion direction)
{
	/* Start a guiding motion in the requested direction */
	
	if (guide_cam == NULL)
		return;
	
	if (direction & TM_NORTH)
		sx_guide (&guide_cam->usbd, SXNorth);
	if (direction & TM_SOUTH)
		sx_guide (&guide_cam->usbd, SXSouth);
	if (direction & TM_EAST)
		sx_guide (&guide_cam->usbd, SXEast);
	if (direction & TM_WEST)
		sx_guide (&guide_cam->usbd, SXWest);
}

void sxc_guide_stop (enum TelMotion direction)
{
	/* Stop a guiding motion in the requested direction */
	
	if (guide_cam == NULL)
		return;
		
	if (direction & TM_NORTH)
		sx_guide (&guide_cam->usbd, (SXStop | SXNorth));
	if (direction & TM_SOUTH)
		sx_guide (&guide_cam->usbd, (SXStop | SXSouth));
	if (direction & TM_EAST)
		sx_guide (&guide_cam->usbd, (SXStop | SXEast));
	if (direction & TM_WEST)
		sx_guide (&guide_cam->usbd, (SXStop | SXWest));
}

void sxc_set_guide_command_cam (struct sx_cam *cam)
{
	/* Set a pointer to the structure that represents the camera used for
	 * sending commands to the telescope guide port.
	 */
	 
	 if (cam == NULL)
		guide_cam = &c_cam;
	else
		guide_cam = cam;
}

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

static int SX_STATUS (struct usbdevice *u_dev, int OK)
{
	/* Prints error message and returns FALSE if OK == FALSE */

	char c[80];
	
	if (OK) {
		error_func ((int *) 0, u_dev->func, c);
		return TRUE;
	} else {
		strcpy (c, u_dev->err);
		error_func ((int *) 1, u_dev->func, c);
		return FALSE;
	}
}

struct usbdevice sx_get_ccdcam_usbd (void)
{
	/* Return the usbdevice part of the ccd camera structure for use by the
	 * autoguiding code if it is using a guide head attached to the main
	 * camera.
	 */
	 
	return c_cam.usbd;
}

static int sx_reset (struct usbdevice *u_dev)
{
	/* Reset the camera */
	
	unsigned char CommandBlock[8];

	CommandBlock[0] = 64;
	CommandBlock[1] = 6;
	CommandBlock[2] = 0;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, NULL, 0)))
		return FALSE;
		
	return TRUE;
}

static char *sx_get_firmware_version (struct usbdevice *u_dev)
{
	/* Get firmware version */
	
	unsigned char CommandBlock[12];
	unsigned char ReadBlock[512];
	static char version[20];

	CommandBlock[0] = 64;
	CommandBlock[1] = 255;
	CommandBlock[2] = 0;
	CommandBlock[3] = 0;
	CommandBlock[4] = 0;
	CommandBlock[5] = 0;
	CommandBlock[6] = 4;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io(u_dev, CommandBlock, 8, ReadBlock, 4)))
		return NULL;
	
    sprintf (version, "%d.%d", (uint8_t) ReadBlock[2] |
							   ((uint8_t) ReadBlock[3] << 8),
							   (uint8_t) ReadBlock[0] |
							   ((uint8_t) ReadBlock[1]));

	return version;
}

static int sx_get_camera_info (struct usbdevice *u_dev, 
							   struct ccd_capability *cam_cap)
{
	/* Get camera details */

	unsigned short modnum;
	unsigned char CommandBlock[12];
	unsigned char ReadBlock[512];
	
	/* Camera model info... */
	
	CommandBlock[0] = 192;
	CommandBlock[1] = 14;
	CommandBlock[2] = 0;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 2;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io(u_dev, CommandBlock, 8, ReadBlock, 2)))
		return FALSE;
	
	modnum = ReadBlock[0] | (ReadBlock[1] << 8);
	sprintf (cam_cap->camera_snum, "%d", modnum);
	
	cam_cap->IsInterlaced = modnum & 0x40 ? TRUE : FALSE;
	if (modnum == 0x84)
		cam_cap->IsInterlaced = TRUE;
	modnum &= 0x1F;
	if (modnum == 0x16 || modnum == 0x17 || modnum == 0x18 || modnum == 0x19)
		cam_cap->IsInterlaced = FALSE;
	
	cam_cap->IsColour = ReadBlock[0] & 0x80 ? TRUE : FALSE;
	
	/* CCD parameters... */

	CommandBlock[0] = 64;
	CommandBlock[1] = 8;
	CommandBlock[2] = 0;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 17;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev,gqusb_bulk_io(u_dev, CommandBlock, 8, ReadBlock, 17)))
		return FALSE;
	
	/* Chip size in pixels... */
	cam_cap->max_h = ReadBlock[2] | (ReadBlock[3] << 8);
	cam_cap->max_v = ReadBlock[6] | (ReadBlock[7] << 8);
	/* Interlaced cameras declare a size as though they were read progressively,
	 * i.e. both fields were read at once.  But if there are N pixels rows,
	 * there are only N/2 vertical registers for these cameras, so the declared 
	 * number of vertical pixels is half the actual value.
	 */
	if (cam_cap->IsInterlaced)
		cam_cap->max_v *= 2;
	
	/* Pixel size in microns... */	
	cam_cap->pixsiz_h = (double) (ReadBlock[8] | (ReadBlock[9] << 8)) / 256.0;
	cam_cap->pixsiz_v = (double) (ReadBlock[10] | (ReadBlock[11] << 8)) / 256.0;
	/* For interlaced cameras, the declared vertical pixel size is twice the
	 * actual value, in order to give the correct chip size when combined with 
	 * the declared number of pixels (see above).
	 */
	if (cam_cap->IsInterlaced)
		cam_cap->pixsiz_v /= 2;
	
	/* Bits per pixel and max adu value per pixel */
	cam_cap->bitspp = ReadBlock[14];
	cam_cap->max_adu = (1 << ReadBlock[14]) - 1;
	
	/* Pulse guide? */
	cam_cap->CanPulseGuide = ReadBlock[16] & 0x01 ? TRUE : FALSE;
	/* Cooler? */
	cam_cap->CanSetCCDTemp = ReadBlock[16] & 0x10 ? TRUE : FALSE;
	/* Shutter? */
	cam_cap->HasShutter = ReadBlock[16] & 0x20 ? TRUE : FALSE;
	
	return TRUE;	
}

static int sx_open_shutter (struct usbdevice *u_dev, int HasShutter)
{
	/* Open the camera shutter (if there is one) */
	
	unsigned char CommandBlock[8];
	unsigned char ReadBlock[2];
	
	if (HasShutter) {
		CommandBlock[0] = 64;
		CommandBlock[1] = 32;
		CommandBlock[2] = 0;
		CommandBlock[3] = 64;
		CommandBlock[4] = 0;
		CommandBlock[5] = 0;
		CommandBlock[6] = 0;
		CommandBlock[7] = 0;
		
		if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, 
		                                             ReadBlock, 2)))
			return FALSE;
	}
	
	return TRUE;
}

static int sx_close_shutter (struct usbdevice *u_dev, int HasShutter)
{
	/* Close the camera shutter (if there is one) */
	
	unsigned char CommandBlock[8];
	unsigned char ReadBlock[2];
	
	if (HasShutter) {
		CommandBlock[0] = 64;
		CommandBlock[1] = 32;
		CommandBlock[2] = 0;
		CommandBlock[3] = 128;
		CommandBlock[4] = 0;
		CommandBlock[5] = 0;
		CommandBlock[6] = 0;
		CommandBlock[7] = 0;
		
		if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, 
		                                             ReadBlock, 2)))
			return FALSE;
	}
	
	return TRUE;
}

static int sx_dump_charge (struct usbdevice *u_dev)
{
	/* Dump the charge in all pixels */
	
	unsigned char CommandBlock[8];

	CommandBlock[0] = 64;
	CommandBlock[1] = 1;
	CommandBlock[2] = 0;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, NULL, 0)))
		return FALSE;
		
	return TRUE;
}

static int sx_clear_vert (struct usbdevice *u_dev)
{
	/* Clear the vertical registers of thermal charge */
	
	unsigned char CommandBlock[8];

	CommandBlock[0] = 64;
	CommandBlock[1] = 1;
	CommandBlock[2] = 8;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, NULL, 0)))
		return FALSE;
		
	return TRUE;
}

static int sx_get_row_data (struct usbdevice *u_dev, enum RowData rows, 
							unsigned char *buf, long bytes, 
							unsigned int x, unsigned int y, 
							unsigned int x_wid, unsigned int y_wid,
							unsigned int x_bin, unsigned int y_bin)
{
	/* Get the pixel data */
	
	unsigned char CommandBlock[18];
	
	CommandBlock[0] = 64;
	CommandBlock[1] = 3;
	CommandBlock[2] = rows;
	CommandBlock[3] = 0;
	CommandBlock[4] = u_dev->id;
	CommandBlock[5] = 0;
	CommandBlock[6] = 10;
	CommandBlock[7] = 0;
	
	CommandBlock[8] = x & 0xFF;
	CommandBlock[9] = x >> 8;
	CommandBlock[10] = y & 0xFF;
	CommandBlock[11] = y >> 8;
	CommandBlock[12] = x_wid & 0xFF;
	CommandBlock[13] = x_wid >> 8;
	CommandBlock[14] = y_wid & 0xFF;
	CommandBlock[15] = y_wid >> 8;
	CommandBlock[16] = x_bin;
	CommandBlock[17] = y_bin;
	
	if (!SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 18, buf, bytes)))
		return FALSE;
		
	return TRUE;
}

static void sx_guide (struct usbdevice *u_dev, unsigned short cmd)
{
	/* Start or stop guide motions in the requested direction.  Guide commands
	 * sent using this routine are not protected by a mutex, so they can occur
	 * simultaneously and also at the same time as image download (for better or
	 * worse).
	 */
	
	static unsigned char guide = 0;
	unsigned char CommandBlock[8];

	if (SXStop & cmd)  /* Stop guiding */
		guide &= ~cmd;
	else
		guide |= cmd;  /* Start guiding */
		
	CommandBlock[0] = 64;
	CommandBlock[1] = 9;
	CommandBlock[2] = guide;
	CommandBlock[3] = 0;
	CommandBlock[4] = 0;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	SX_STATUS (u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, NULL, 0));
}

static void sx_guide_pulse (struct usbdevice *u_dev, enum SXGuide cmd,
							int duration)
{
	/* Pulse guide with timing done here.  The mutex controlling access to the
	 * camera is locked in gqusb_bulk_write_lock and unlocked in 
	 * gqusb_bulk_write_unlock.  This mutex is also shared by gqusb_bulk_io so
	 * guide_pulse commands and image capture are always guaranteed to occur
	 * atomically with respect to the camera, i.e. guide pulses cannot interrupt
	 * the downloading of a camera image, and neither can downloading a camera 
	 * image disrupt the pulse timing.  Another consequence is that if two 
	 * pulse guide commands are issued simultaneously, one will block
	 * until the other has finished.  Not sure whether it is necessary to be
	 * this careful with SX cameras though...
	 */
	
	struct timespec length;
	unsigned char CommandBlock[8];

	length.tv_sec = (time_t) duration / 1000.0; /* duration is milliseconds */
	length.tv_nsec = 1e+06 * (duration - 1000.0 * length.tv_sec);
																 
	CommandBlock[0] = 64;
	CommandBlock[1] = 9;
	CommandBlock[2] = cmd;
	CommandBlock[3] = 0;
	CommandBlock[4] = 0;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	if (SX_STATUS (u_dev, gqusb_bulk_write_lock (u_dev, CommandBlock, 8))) {
		nanosleep (&length, NULL);
		CommandBlock[2] = SXStopAll;
		SX_STATUS (u_dev, gqusb_bulk_write_unlock (u_dev, CommandBlock, 8));
	}
}

static int sx_set_cooling (struct usbdevice *u_dev, struct cooler *cool)
{
	/* Start or stop the cooling at the desired temperature */
	
	unsigned char CommandBlock[8];
	unsigned char ReadBlock[3];
	int cool_temp;
	
	cool_temp = (int) rint (10.0 * (273.0 + cool->req_temp));

	CommandBlock[0] = 64;
	CommandBlock[1] = 30;
	CommandBlock[2] = cool_temp & 0xFF;
	CommandBlock[3] = (cool_temp >> 8) & 0xFF;
	CommandBlock[4] = cool->req_coolstate;
	CommandBlock[5] = 0;
	CommandBlock[6] = 0;
	CommandBlock[7] = 0;
	
	if (!SX_STATUS(u_dev, gqusb_bulk_io (u_dev, CommandBlock, 8, ReadBlock, 3)))
		return FALSE;
	
	cool->act_temp = (double)((ReadBlock[1] << 8) + ReadBlock[0] - 2730) / 10.0;
	cool->act_coolstate = ReadBlock[2] > 0 ? 1 : 0;
	
	return TRUE;
}

static void *sx_expose (void *params)
{
	/* This is the camera exposure thread.  All variables must be either local
	 * to this routine or passed in via the thread parameters.  This thread
	 * function must not change global values directly because it may be called
	 * simultaneously for the main and autoguider cameras.
	 * If called with cam->Expose == TRUE, then an exposure is made.
	 * If called with cam->Expose == FALSE, the chip is read immediately without 
	 * first making an exposure.  This is used if the user wishes to abort an
	 * existing exposure and read the chip, without waiting for the requested
	 * exposure time to finish.  See sx_interrupt_exposure.
	 */
	  
	struct sx_cam *cam = (struct sx_cam *) params;
	
	struct timespec length;
	struct timeval start, stop;
    int x_wid, y_wid, h, v, val, maxval;
    unsigned long i, ei, oi, buf_size;
	float o_med, e_med, ratio;
	double dsec;
	
	/* Set the exposure length */
	
	length.tv_nsec = 1e+09 * modf (cam->ip.req_len, &dsec);
	length.tv_sec = (time_t) dsec;
	
	/* Make an exposure */
	
	cam->status = SXExposing;
	if (!cam->SXInterlaced) {  /* Progressive chips... */
		
		x_wid = (int) (cam->ip.x_wid / cam->ip.x_bin);
		y_wid = (int) (cam->ip.y_wid / cam->ip.y_bin);
		/* Camera should return 2 bytes per pixel even for 8 bit data (the high
		 * byte will be empty) as far as I can tell.
		 */
		buf_size = 2 * x_wid * y_wid;
		
		if (cam->Expose) { /* ...otherwise, just read the chip */ 
			sx_dump_charge (&cam->usbd);
			gettimeofday (&start, NULL);
			sx_open_shutter (&cam->usbd, cam->SXShutter);
			delay (&cam->usbd, &length);
		}
		sx_close_shutter (&cam->usbd, cam->SXShutter);
		gettimeofday (&stop, NULL);
		cam->status = SXReading;
		sx_get_row_data (&cam->usbd, SXAllRows,
						 cam->all_buf, buf_size, 
						 cam->ip.x, cam->ip.y, 
						 cam->ip.x_wid, cam->ip.y_wid, 
						 cam->ip.x_bin, cam->ip.y_bin);

		/* Convert the byte data to short integers (assuming maximum of 
		 * 16 bits per pixel).
		 */
		 
		for (ei = 0, i = 0; i < buf_size; i+=2) {
				cam->e_buf[ei++] = (unsigned char) cam->all_buf[i] | 
								   (unsigned char) cam->all_buf[i+1] << 8;
		}
		
		/* Optionally invert the image if requested */
		
		i = cam->InvertImage ? x_wid * (y_wid - 1) : 0;
		for (v = 0; v < y_wid; v++) {
			for (h = 0; h < x_wid; h++) {
				cam->img_buf[i++] = cam->e_buf[(v + 1) * x_wid - (h + 1)];
			}
			i -= cam->InvertImage ? 2 * x_wid : 0;
		}
	
	} else {                   /* Interlaced chips... */
		
		if (cam->ip.y_bin == 1) {
			
			x_wid = (int) (cam->ip.x_wid / cam->ip.x_bin);
			y_wid = (int) cam->ip.y_wid;
			/* Camera should return 2 bytes per pixel even for 8 bit data (the 
			 * high byte will be empty) as far as I can tell.
			 */
			buf_size = 2 * x_wid * y_wid;
			
			/* For images that are not binned in the vertical direction, the
			 * two fields are read separately.  It takes about 0.1s to read
			 * either field, so for exposures of this duration or less, it is 
			 * best to expose and read the two fields sequentially (thus making
			 * two separate exposures).  For exposures longer than this, just 
			 * one exposure is made and the two fields are then read 
			 * consecutively.  The field that is read last will have been
			 * exposed for about 0.1s longer.  A scaling factor is applied when
			 * the two fields are combined (see below).
			 */
			 
			if (cam->ip.req_len < SHORT_EXPOSURE) {
				if (cam->Expose) { /* No point in permitting user to interrupt*/
					sx_dump_charge (&cam->usbd);  /* such a short exposure    */
					gettimeofday (&start, NULL);
					sx_open_shutter (&cam->usbd, cam->SXShutter);
					nanosleep (&length, NULL);
					sx_close_shutter (&cam->usbd, cam->SXShutter);
					gettimeofday (&stop, NULL);
					sx_get_row_data (&cam->usbd, SXOddRows,
									 cam->all_buf, buf_size, 
									 cam->ip.x, cam->ip.y,
									 cam->ip.x_wid, cam->ip.y_wid, 
									 cam->ip.x_bin, cam->ip.y_bin);
					sx_dump_charge (&cam->usbd);
					sx_open_shutter (&cam->usbd, cam->SXShutter);
					nanosleep (&length, NULL);
					sx_close_shutter (&cam->usbd, cam->SXShutter);
					sx_get_row_data (&cam->usbd, SXEvenRows,
									 cam->all_buf+buf_size, buf_size, 
									 cam->ip.x, cam->ip.y, 
									 cam->ip.x_wid, cam->ip.y_wid, 
									 cam->ip.x_bin, cam->ip.y_bin);
				}
			} else {
	
				if (cam->Expose) { /* ...otherwise, just read the chip */
					sx_dump_charge (&cam->usbd);
					gettimeofday (&start, NULL);
					sx_open_shutter (&cam->usbd, cam->SXShutter);
					delay (&cam->usbd, &length);
				}
				sx_close_shutter (&cam->usbd, cam->SXShutter);
				cam->status = SXReading;
				sx_get_row_data (&cam->usbd, SXOddRows,
								 cam->all_buf, buf_size, 
								 cam->ip.x, cam->ip.y, 
								 cam->ip.x_wid, cam->ip.y_wid, 
								 cam->ip.x_bin, cam->ip.y_bin);
				gettimeofday (&stop, NULL);
				sx_get_row_data (&cam->usbd, SXEvenRows,
								 cam->all_buf+buf_size, buf_size, 
								 cam->ip.x, cam->ip.y, 
								 cam->ip.x_wid, cam->ip.y_wid, 
								 cam->ip.x_bin, cam->ip.y_bin);
			}
			
			/* Convert the byte data to short integers (assuming maximum of 
			 * 16 bits per pixel).
			 */
			
			for (oi = 0, ei = 0, i = 0; i < buf_size; i += 2) {
				cam->o_buf[oi++] = 
							(unsigned char) cam->all_buf[i] | 
							(unsigned char) cam->all_buf[i + 1] << 8;
				cam->e_buf[ei++] = 
							(unsigned char) cam->all_buf[i + buf_size] | 
							(unsigned char) cam->all_buf[i + buf_size + 1] << 8;
			}
			
			/* Calculate the median values in each field and a scaling factor.
			 * Median should be more reliable than average for calculating the
			 * sky background in a star field.
			 */
			
			o_med = (float) torben (cam->o_buf, buf_size / 2);
			e_med = (float) torben (cam->e_buf, buf_size / 2);
			ratio = e_med / o_med;
					
			/* Optionally invert the image if requested.
			 * Scale the odd and even fields to the same median value to
			 * allow for differences in exposure length.  In practice we scale 
			 * the shorter exposure so that it matches the longer exposure.  
			 * This then gives an effective exposure length longer than the 
			 * requested value by an amount equal to the time to read the 
			 * shorter field exposure (slightly more than 0.1s).  Values
			 * in the shorter exposure are scaled up and capped at the
			 * maximum permissible value (typically 65535) and this gives a
			 * clean-looking image even in the saturated parts.
			 * Note that this simple scaling procedure scales bias + signal, 
			 * rather than scaling just the signal and then adding it onto the 
			 * bias, but the error is small.  Note also that field scaling 
			 * is done for exposure lengths less than SHORT_EXPOSURE, where the 
			 * two fields are exposed completely independently and in principle
			 * should have the same median value.
			 */
			
			maxval = (1 << cam->bitspp) - 1;
			i = cam->InvertImage ? x_wid * (2 * y_wid - 2) : 0;
			for (v = 0; v < y_wid; v++) {
				for (h = 0; h < x_wid; h++) {
					if (!cam->InvertImage) {
						val = cam->o_buf[(v + 1) * x_wid - (h + 1)];
						if (ratio >= 1)
							val = val * ratio < maxval ? val * ratio : maxval;
						cam->img_buf[i++] = val;
					} else {
						val = cam->e_buf[(v + 1) * x_wid - (h + 1)];
						if (ratio < 1)
							val = val / ratio < maxval ? val / ratio : maxval;
						cam->img_buf[i++] = val;
					}
				}
				for (h = 0; h < x_wid; h++) {
					if (!cam->InvertImage) {
						val = cam->e_buf[(v + 1) * x_wid - (h + 1)];
						if (ratio < 1)
							val = val / ratio < maxval ? val / ratio : maxval;
						cam->img_buf[i++] = val;
					} else {
						val = cam->o_buf[(v + 1) * x_wid - (h + 1)];
						if (ratio >= 1)
							val = val * ratio < maxval ? val * ratio : maxval;
						cam->img_buf[i++] = val;
					}
				}
				i -= cam->InvertImage ? 4 * x_wid : 0;
			}
			
		} else {   /* p->y_bin > 1 */
			
			/* GoQat restricts vertical binning to a factor of two for
			 * interleaved chips (e.g. the Lodestar).  Reading both fields
			 * simultaneously has the effect of binning x2 vertically, so we
			 * divide y_bin by two here.
			 */
			 
			x_wid = (int) (cam->ip.x_wid / cam->ip.x_bin);
			y_wid = (int) (cam->ip.y_wid / (cam->ip.y_bin / 2));
			/* Camera should return 2 bytes per pixel even for 8 bit data (the 
			 * high byte will be empty) as far as I can tell.
			 */
			buf_size = 2 * x_wid * y_wid;
			
			if (cam->Expose) { /* ...otherwise, just read the chip */ 
				sx_dump_charge (&cam->usbd);
				gettimeofday (&start, NULL);
				sx_open_shutter (&cam->usbd, cam->SXShutter);
				delay (&cam->usbd, &length);
			}
			sx_close_shutter (&cam->usbd, cam->SXShutter);
			gettimeofday (&stop, NULL);
			cam->status = SXReading;
			sx_get_row_data (&cam->usbd, SXAllRows,
							 cam->all_buf, buf_size, 
							 cam->ip.x, cam->ip.y, 
							 cam->ip.x_wid, cam->ip.y_wid, 
							 cam->ip.x_bin, cam->ip.y_bin / 2);

			/* Convert the byte data to short integers (assuming maximum of 
			 * 16 bits per pixel).  (e_buf is used as handy storage space - it's
			 * actually the odd and even fields combined in this case because
			 * we've read all the rows in one go).
			 */
			 
			for (ei = 0, i = 0; i < buf_size; i+=2) {
					cam->e_buf[ei++] = (unsigned char) cam->all_buf[i] | 
									   (unsigned char) cam->all_buf[i+1] << 8;
			}
			
			/* Optionally invert the image if requested */
			
			i = cam->InvertImage ? x_wid * (y_wid - 1) : 0;
			for (v = 0; v < y_wid; v++) {
				for (h = 0; h < x_wid; h++) {
					cam->img_buf[i++] = cam->e_buf[(v + 1) * x_wid - (h + 1)];
				}
				i -= cam->InvertImage ? 2 * x_wid : 0;
			}
		}
	}
	
	cam->ip.act_len = stop.tv_sec + stop.tv_usec / 1.e6 - 
					 (start.tv_sec + start.tv_usec/1.e6);
	cam->status = SXIdle;
	cam->ImageReady = TRUE;
}

static void delay (struct usbdevice *u_dev, struct timespec *length)
{
	/* Delay execution of the calling thread by the given amount.  If the
	 * requested value is longer than 5 seconds, then at 5 seconds before the
	 * end, clear the thermal charge from the CCD storage registers and then
	 * finish the delay.
	 */
	 
	struct timespec now, end;
	
	if (length->tv_sec < 5) {
		nanosleep (length, NULL);
	} else {
		/* CLOCK_MONOTONIC would be better below, but not sure which versions of
		 * Linux support it.
		 */
		clock_gettime (CLOCK_REALTIME, &now);
		end.tv_sec = now.tv_sec + length->tv_sec;
		end.tv_nsec = now.tv_nsec + length->tv_nsec;
		if (end.tv_nsec > 999999999L) {
			end.tv_nsec -= 1000000000L;
			end.tv_sec++;
		}
		length->tv_sec -= 5;
		nanosleep (length, NULL);
		sx_clear_vert (u_dev);
		clock_nanosleep (CLOCK_REALTIME, TIMER_ABSTIME, &end, NULL);
	}
}

/*
 * Calculate median without modifying original data array.
 * Algorithm by Torben Mogensen, implementation by N. Devillard.
 * This code is in the public domain.
 * See http://ndevilla.free.fr/median/median/index.html
 */

static unsigned short int torben (unsigned short int m[], int n)
{
	int i, less, greater, equal;
	unsigned short int  min, max, guess, maxltguess, mingtguess;

	min = max = m[0] ;
	for (i=1 ; i<n ; i++) {
		if (m[i]<min) min=m[i];
		if (m[i]>max) max=m[i];
	}

	while (1) {
		guess = (min+max)/2;
		less = 0; greater = 0; equal = 0;
		maxltguess = min ;
		mingtguess = max ;
		for (i=0; i<n; i++) {
			if (m[i]<guess) {
				less++;
				if (m[i]>maxltguess) maxltguess = m[i] ;
			} else if (m[i]>guess) {
				greater++;
				if (m[i]<mingtguess) mingtguess = m[i] ;
			} else equal++;
		}
		if (less <= (n+1)/2 && greater <= (n+1)/2) break ; 
		else if (less>greater) max = maxltguess ;
		else min = mingtguess;
	}
	if (less >= (n+1)/2) return maxltguess;
	else if (less+equal >= (n+1)/2) return guess;
	else return mingtguess;
}

#endif /* HAVE_SX_CAM */

#ifdef HAVE_SX_FILTERWHEEL

/******************************************************************************/
/*           THE FOLLOWING FUNCTIONS ARE FOR FILTER WHEEL CONTROL             */
/******************************************************************************/
 
int sxf_get_filterwheels (const char *devnode[], const char *desc[], int *num)
{
	/* Return device nodes and descriptions for any filter wheels found, as well
	 * as the number found.  On entry, 'num' is the maximum number of filter
	 * wheels to search for.  On return, 'num' is the actual number found. The 
	 * following is based on the example code by Alan Ott, Signal 11 Software.
	 */
	 
	#define MAX_STRING 128
	 
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *hiddev, *usbdev;

	int j;
	const char *path;
	static char dn[SX_MAX_FILTERWHEELS][MAX_STRING];
	static char ds[SX_MAX_FILTERWHEELS][MAX_STRING];
	
	/* Create a new udev object */
	
	udev = udev_new ();
	if (!udev) {
		error_func ((int *) 1, __func__, "Unable to search for device nodes");
		return FALSE;
	}
	
	/* Create a list of the devices in the 'hidraw' subsystem. */
	
	enumerate = udev_enumerate_new (udev);
	udev_enumerate_add_match_subsystem (enumerate, "hidraw");
	udev_enumerate_scan_devices (enumerate);
	devices = udev_enumerate_get_list_entry (enumerate);
	
	/* udev_list_entry_foreach is a macro that expands to a loop. The loop will 
	 * be executed for each member in devices, setting dev_list_entry to a list 
	 * entry that contains the device's path in /sys.
	 */
	
	j = 0; 
	*num = *num > SX_MAX_FILTERWHEELS ? SX_MAX_FILTERWHEELS : *num;
	udev_list_entry_foreach (dev_list_entry, devices) {
		
		/* Get the filename of the /sys entry for the device and create a 
		 * udev_device object (dev) representing it.
		 */
		 
		path = udev_list_entry_get_name (dev_list_entry);
		hiddev = udev_device_new_from_syspath (udev, path);

		/* The device pointed to by dev contains information about the hidraw 
		 * device. To get information about the USB device, get the parent 
		 * device with the subsystem/devtype pair of "usb"/"usb_device". This 
		 * will be several levels up the tree, but the function will find it.
		 */
		 
		usbdev = udev_device_get_parent_with_subsystem_devtype (hiddev, "usb",
		                                                          "usb_device");
		if (!usbdev) {
			error_func ((int *) 1, __func__, "Unable to find USB device");
			return FALSE;
		}
	
		/* Call get_sysattr_value() for each file in the device's /sys entry. 
		 * The strings passed into these functions (idProduct, idVendor, serial,
		 * etc.) correspond directly to the files in the /sys directory that
		 * represents the USB device.
		 */
		 
		if (strtol (udev_device_get_sysattr_value (
		                      usbdev, "idVendor"), NULL, 16) == (long) SX_VID) {
			strncpy (dn[j], udev_device_get_devnode(hiddev), MAX_STRING);
			snprintf (ds[j], MAX_STRING, "%s:%s %s %s %s\n",
						 udev_device_get_sysattr_value (usbdev, "idVendor"),
						 udev_device_get_sysattr_value (usbdev, "idProduct"),
						 udev_device_get_sysattr_value (usbdev, "manufacturer"),
						 udev_device_get_sysattr_value (usbdev, "product"),
						 udev_device_get_sysattr_value (usbdev, "serial"));
							 
			devnode[j] = &dn[j][0];
			desc[j] = &ds[j][0];
			j++;
		}
		udev_device_unref (hiddev);
		if (j == *num)
			break;  
	}
	udev_enumerate_unref (enumerate);
	udev_unref (udev);
	*num = j;
	
	return TRUE;
}

int sxf_connect (int connect, const char *devnode)
{
	/* Open/Close the given device node */
	
	if (connect) {
		if ((fw_fd = open (devnode, O_RDWR | O_NONBLOCK)) < 0) {
			error_func ((int *) 1, __func__, "Error opening device node");
			error_func ((int *) 1, __func__, strerror (errno));
			return FALSE;
		}
	} else { /* Assume we're being asked to close the node that we opened */
		if (close (fw_fd) < 0) {
			error_func ((int *) 1, __func__, "Error closing device node");
			error_func ((int *) 1, __func__, strerror (errno));
			return FALSE;
		}
	}
	return TRUE;
}

int sxf_set_filter_pos (unsigned short pos)
{
	/* Set the current filter wheel position */
	
	unsigned short wait_fw;
	int ret;
	char buf[3];
	
	wait_fw = 10;  /* Allow time for wheel to get to new position */
	while (wait_fw--) {
		buf[0] = 0;
		buf[1] = (char) pos;
		buf[2] = 0;
		if (write (fw_fd, buf, 3) < 0) {
			error_func ((int *) 1, __func__, "Error setting filter position");
			error_func ((int *) 1, __func__, strerror (errno));
			return FALSE;
		}
		sleep (1);
		
		ret = read (fw_fd, buf, 2);
		if (ret < 0 && ret != EAGAIN) {
			error_func ((int *) 1, __func__, "Error reading filter position");
			error_func ((int *) 1, __func__, strerror (errno));
			return FALSE;
		}
		if (buf[0] == pos) /* 0 if FW moving; otherwise buf[0] is filter pos. */
			return TRUE;
	}
	
	error_func ((int *) 1, __func__, "Timeout waiting for filter position");
	return FALSE;
}

unsigned short sxf_get_filter_pos (void)
{
	/* Return the current filter wheel position */
	
	struct timespec length;
	unsigned short wait_fd;
	int ret;
	char buf[3];
	
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	
	if (write (fw_fd, buf, 3) < 0) {
		error_func ((int *) 1, __func__, "Error requesting filter position");
		error_func ((int *) 1, __func__, strerror (errno));
		return 0;
	}
	
	/* Wait for multiples of 1 ms */
	
	length.tv_nsec = 1e+06;
	length.tv_sec = 0;
	wait_fd = 10;
	while (wait_fd--) {
		nanosleep (&length, NULL);
		ret = read (fw_fd, buf, 2);
		if (ret < 0 && ret != EAGAIN) {
			error_func ((int *) 1, __func__, "Error reading filter position");
			error_func ((int *) 1, __func__, strerror (errno));
			return 0;
		}
		if (buf[0] > 0) /* 0 if FW moving; otherwise buf[0] is filter position*/
			return (unsigned short) buf[0];
	}
	
	error_func ((int *) 1, __func__, "Timeout waiting for filter position");
	return 0;
}

#endif /* HAVE_SX_FILTERWHEEL */

void sx_error_func (void (*err_func) (int *err, const char *func, char *msg ))
{
	/* Set the error function callback */
	
	error_func = err_func;
}


