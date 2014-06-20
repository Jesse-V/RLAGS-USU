/******************************************************************************/
/*                           CCD CAMERA ROUTINES                              */
/*                                                                            */
/* All the routines for interfacing with the CCD camera are contained in here.*/
/*                                                                            */
/* Copyright (C) 2009 - 2014  Edward Simonson                                 */
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define GOQAT_CCDCAM
#include "interface.h"

#ifdef HAVE_QSI
#include "qsi.h"
#endif
#ifdef HAVE_SX_CAM
#include "sx.h"
#endif

#define CCD_BLACK     0                     /* Black level */
#define CCD_WHITE 65535                     /* White level */

static struct cam_img ccd_cam_obj, *ccd;
	
/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void ccdcam_init (void);
gboolean ccdcam_get_cameras (void);
gboolean ccdcam_open (void);
static gboolean ccdcam_get_cap (void);
gboolean ccdcam_close (void);
void ccdcam_set_exposure_data (struct exposure_data *exd);
gboolean ccdcam_start_exposure (void);
gboolean ccdcam_cancel_exposure (void);
gboolean ccdcam_interrupt_exposure (void);
gboolean ccdcam_image_ready (void);
gboolean ccdcam_download_image (void);
gboolean ccdcam_process_image (void);
gboolean ccdcam_debayer (void);
gboolean ccdcam_set_temperature (gboolean *AtTemperature);
void ccdcam_set_fast_readspeed (gboolean Set);
gboolean ccdcam_measure_HFD (gboolean Initialise, gboolean Plot, gint box,
						     struct exposure_data *exd, gdouble *hfd);
static gdouble HFD_row_flux (gushort row[], gushort npix, gushort pix_split,
							 gdouble pix_h, gdouble pix_v, 
							 gdouble meanh, gdouble meanv, 
                             gdouble hfd, gdouble bg);
gboolean ccdcam_plot_temperatures (void);
gboolean ccdcam_get_status (void);
static int (*camera_get_cameras) (const char *serial[], const char *desc[], 
								  int *num) = NULL;
static int (*camera_connect) (int connect, const char *serial) = NULL;
static int (*camera_get_cap) (struct ccd_capability *cam_cap) = NULL;
static int (*camera_set_imagearraysize) (long h, long v, long h_wid, 
									     long v_wid, long h_bin, long v_bin) 
                                                                         = NULL;
static int (*camera_start_exposure) (char *dateobs, double length, int light) 
                                                                         = NULL;
static int (*camera_cancel_exposure) (void) = NULL;
static int (*camera_interrupt_exposure) (void) = NULL;
static int (*camera_get_imageready) (int *ready) = NULL;
static int (*camera_get_exposuretime) (char *dateobs, double *length) = NULL;
static int (*camera_get_imagearraysize) (int *h_pix, int *v_pix, int *bytes) 
                                                                         = NULL;
static int (*camera_get_imagearray) (unsigned short *array) = NULL;
#ifdef HAVE_QSI
static void ccdcam_qsi_error_func (int *err, const char *func, char *msg);
#endif
void ccdcam_sx_error_func (int *err, const char *func, char *msg);
struct cam_img *get_ccd_image_struct (void);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void ccdcam_init (void)
{
	static gboolean FirstPass = TRUE;
	
	/* Initialise CCD camera data to sensible values */

	ccd = &ccd_cam_obj;
	
	ccd->cam_cap.max_h = CHP_MAX;
	ccd->cam_cap.max_v = CHP_MAX;
	ccd->cam_cap.max_binh = CHP_MAX;
	ccd->cam_cap.max_binv = CHP_MAX;
	ccd->cam_cap.min_exp = EXP_MIN;
	ccd->cam_cap.max_exp = EXP_MAX;
	memset (&ccd->state, 0, sizeof (struct ccd_state));
	ccd->cam_cap.HasFilterWheel = FALSE;
	ccd->exd.req_len = 0.0;
	ccd->exd.ccdtemp = 0.0;
	strcpy (ccd->exd.filename, " ");
	ccd->imdisp.B = CCD_BLACK;
	ccd->imdisp.W = CCD_WHITE;
	ccd->graph.reset = TRUE;
	ccd->ds9.stream = NULL;
	ccd->ds9.window = NULL;
	ccd->ds9.display = NULL;
	ccd->set_state = NULL;
	ccd->get_state = NULL;
	ccd->r161 = NULL;
	ccd->db163 = NULL;
	ccd->ff161 = NULL;
	ccd->ff163 = NULL;
	ccd->id = CCD;
	ccd->devnum = -1;
	ccd->fd = 0;
	ccd->bayer_pattern = 0;
	ccd->Open = FALSE;
	ccd->Expose = FALSE;
	ccd->FileSaved = TRUE;
	ccd->Display = TRUE;
	ccd->Error = FALSE;
	
	if (FirstPass) {
		/* Initialise some values for the first pass through this routine, but
		 * don't reset them if the camera is closed and the same or another one 
		 * is (re)opened during the same invocation of GoQat.  (They are reset
		 * if GoQat is restarted).
		 * 
		 * These values must match their initial settings in the glade interface
		 * file.  They are set by the user in the GUI and are not read from
		 * the configuration file at start-up.
		 */
		ccd->FastFocus = FALSE;
		ccd->AutoSave = FALSE;
		ccd->SavePeriodic = FALSE;
		
		FirstPass = FALSE;
	}
}

gboolean ccdcam_get_cameras (void)
{
	/* Fill the device selection structure with a list of detected cameras */
	
	gint i;
	
	switch (ccd->device) {
		#ifdef HAVE_QSI
		case QSI:
		    camera_get_cameras = qsi_get_cameras;
		    qsi_error_func (ccdcam_qsi_error_func);
			break;
		#endif
		#ifdef HAVE_SX_CAM
		case SX:
			camera_get_cameras = sx_get_cameras;
		    sx_error_func (ccdcam_sx_error_func);
			break;
		#endif
		case OTHER:
			/* Other camera stuff */
			break;
		default:
			break;
	}
	
	ds.num = MAX_DEVICES;
	if (!camera_get_cameras (ds.id, ds.desc, &ds.num))
		return show_error (__func__, "Error searching for cameras!");
	if (!ds.num) {
	    L_print ("{o}Didn't find any cameras - are permissions "
	             "set correctly?\n");
		return FALSE;
	}
	/* Need to store local copy after calling camera_get_cameras or the values
	 * returned from the QSI cameras may 'disappear' if an attempt is made to
	 * use them elsewhere.
	 */
	for (i = 0; i < ds.num; i++) {
		strncpy (ds.lid[i], ds.id[i], 255);
		ds.id[i] = ds.lid[i];
		strncpy (ds.ldesc[i], ds.desc[i], 255);
		ds.desc[i] = ds.ldesc[i];
	}
	
	return TRUE;
}

gboolean ccdcam_open (void)
{
	/* Open the CCD camera */

	gint num;
	gchar *err;
	
	if (ccd->Open)  /* return if already open */
		return TRUE;
	
	L_print ("{b}****---->>>> Opening CCD camera\n");
	
	switch (ccd->device) {
		#ifdef HAVE_QSI
		case QSI:
			
			strcpy (ccd->cam_cap.camera_manf, "QSI");
			
			/* Assign function pointers */
			
		    camera_get_cameras = qsi_get_cameras;
			camera_connect = qsi_connect;
			camera_get_cap = qsi_get_cap;
		    ccd->set_state = qsi_set_state;
		    ccd->get_state = qsi_get_state;
		    camera_set_imagearraysize = qsi_set_imagearraysize;
		    camera_start_exposure = qsi_start_exposure;
		    camera_cancel_exposure = qsi_cancel_exposure;
		    camera_interrupt_exposure = qsi_interrupt_exposure;
		    camera_get_imageready = qsi_get_imageready;
		    camera_get_exposuretime = qsi_get_exposuretime;
		    camera_get_imagearraysize = qsi_get_imagearraysize;
		    camera_get_imagearray = qsi_get_imagearray;
		    ports[USBCCD].guide_pulse = qsi_pulseguide;
		    ports[USBCCD].guide_start = telescope_d_start;
		    ports[USBCCD].guide_stop = telescope_d_stop;
		    qsi_error_func (ccdcam_qsi_error_func);
			break;
		#endif
		#ifdef HAVE_SX_CAM
		case SX:
			
			strcpy (ccd->cam_cap.camera_manf, "SX");
			
			/* Assign function pointers */
			
		    camera_get_cameras = sx_get_cameras;
			camera_connect = sx_connect;
			camera_get_cap = sx_get_cap;
		    ccd->set_state = sx_set_state;
		    ccd->get_state = sx_get_state;
		    camera_set_imagearraysize = sx_set_imagearraysize;
		    camera_start_exposure = sx_start_exposure;
		    camera_cancel_exposure = sx_cancel_exposure;
		    camera_interrupt_exposure = sx_interrupt_exposure;
		    camera_get_imageready = sx_get_imageready;
		    camera_get_exposuretime = sx_get_exposuretime;
		    camera_get_imagearraysize = sx_get_imagearraysize;
		    camera_get_imagearray = sx_get_imagearray;
		    ports[USBCCD].guide_pulse = sx_pulseguide;
		    ports[USBCCD].guide_start = sx_guide_start;
		    ports[USBCCD].guide_stop = sx_guide_stop;
		    if (autog_comms->pnum == USBCCD)
				sxc_set_guide_command_cam (NULL);
		    sx_error_func (ccdcam_sx_error_func);
			break;
		#endif
		default:
			return show_error (__func__, "Unknown camera device");
	}
	
	num = MAX_DEVICES;
	if (!camera_get_cameras (ds.id, ds.desc, &num))
		return show_error (__func__, "Error searching for cameras!");
	if (num == 0) {
	    L_print ("{o}Didn't find any cameras - are permissions "
	             "set correctly?\n");
		return FALSE;
	} else if (num == 1) {
		if (!camera_connect (TRUE, ds.id[0]))
			return show_error (__func__, "Unable to connect to camera - are "
			                             "permissions set correctly?\n");
		ccd->devnum = 0;
	} else if (num > 1 && ccd->devnum == -1) {
		L_print ("{o}Found more than one camera!  Please select the one you "
				 "want via the 'Cameras' menu\n");
	    return FALSE;
    } else if (num > 1 && ccd->devnum < num) {
		if (!camera_connect (TRUE, ds.id[ccd->devnum]))
			return show_error (__func__, "Unable to connect to camera - are "
			                             "permissions set correctly?\n");
	} else
		return show_error (__func__, "Error opening camera!");
	
	if (ccd->device == SX)  /* Have to set SX camera description manually */
		strncpy (ccd->cam_cap.camera_desc, ds.desc[ccd->devnum], 256);
		
	/* Get the selected camera's capabilities */
	
	if (!ccdcam_get_cap ())
		return show_error (__func__, "Unable to get camera capabilities!");
	ccd->imdisp.W = ccd->cam_cap.max_adu;  /* Set max. white level   */
	set_ccd_deftemp ();  /* Set default chip temp. on CCD camera tab */
	
	/* Set initial state based on stored configuration data */
	
	set_camera_state (ccd);
	
	/* Allocate storage for image data */
	
	if (!(ccd->r161 = (gushort *) g_malloc0 (ccd->cam_cap.max_h * 
		                              ccd->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(ccd->db163 = (gushort *) g_malloc0 (ccd->cam_cap.max_h * 
		                          ccd->cam_cap.max_v * 3 * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
    if (!(ccd->ff161 = (gushort *) g_malloc0 (ccd->cam_cap.max_h * 
		                              ccd->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
    if (!(ccd->ff163 = (gushort *) g_malloc0 (ccd->cam_cap.max_h * 
		                          ccd->cam_cap.max_v * 3 * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}	
	
	if (!(ccd->img.mode[GREY].hist = (guint *) g_malloc0 ((ccd->imdisp.W + 1) *
	                                                         sizeof (guint)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	ccd->Open = TRUE;
	L_print ("{b}****---->>>> Opened CCD camera\n");
	return TRUE;
	
open_err:

	ccd->Open = TRUE; /* Forces ccdcam_close to close and release resources */
	ccdcam_close ();
	return show_error (__func__, err);	
}

static gboolean ccdcam_get_cap (void)
{
	/* Get the initial parameters for the selected camera */
		
	if (camera_get_cap (&ccd->cam_cap)) {
		L_print ("Found %s\n", ccd->cam_cap.camera_name);
		L_print ("Manufacturer: %s\n", ccd->cam_cap.camera_manf);
		L_print ("Description: %s\n", ccd->cam_cap.camera_desc);
		L_print ("Serial/Model number: %s\n", ccd->cam_cap.camera_snum);
		L_print ("Driver information: %s\n", ccd->cam_cap.camera_dinf);
		if (ccd->device == QSI)
			L_print ("Full well capacity is... %.2f electrons\n", 
												         ccd->cam_cap.max_well);
		L_print ("Maximum ADU value is... %ld ADU\n", ccd->cam_cap.max_adu);
		if (ccd->device == QSI)
			L_print ("Gain is... %.2f electrons per ADU\n", ccd->cam_cap.e_adu);
		L_print ("Horizontal pixel size is... %.2f microns\n", 
												         ccd->cam_cap.pixsiz_h);
		L_print ("Vertical pixel size is... %.2f microns\n", 
												         ccd->cam_cap.pixsiz_v);
		L_print ("Horizontal array size is... %d pixels\n", ccd->cam_cap.max_h);
		L_print ("Vertical array size is... %d pixels\n", ccd->cam_cap.max_v);
		L_print ("Maximum number of h-bin pixels is... %d pixels\n", 
												         ccd->cam_cap.max_binh);
		L_print ("Maximum number of v-bin pixels is... %d pixels\n", 
												         ccd->cam_cap.max_binv);
		L_print ("Minimum exposure time is... %.4f s\n", ccd->cam_cap.min_exp);
		L_print ("Maximum exposure time is... %.2f s\n", ccd->cam_cap.max_exp);
		L_print ("Can bin asymmetrically?... %s\n", 
								        ccd->cam_cap.CanAsymBin ? "Yes" : "No");
		if (ccd->device == SX) {
			L_print ("Is interlaced?... %s\n", 
								      ccd->cam_cap.IsInterlaced ? "Yes" : "No");
			L_print ("Is colour?... %s\n", 
								          ccd->cam_cap.IsColour ? "Yes" : "No");
		}
		L_print ("Has shutter?... %s\n", 
								        ccd->cam_cap.HasShutter ? "Yes" : "No");
		if (ccd->device == QSI)
			L_print ("Has filter wheel?... %s\n", 
							        ccd->cam_cap.HasFilterWheel ? "Yes" : "No");
		L_print ("Can set CCD temperature?... %s\n", 
							         ccd->cam_cap.CanSetCCDTemp ? "Yes" : "No");
		if (ccd->device == QSI)
			L_print ("Can get cooler power?... %s\n", 
						           ccd->cam_cap.CanGetCoolPower ? "Yes" : "No");
		L_print ("Can pulse guide?... %s\n", 
						             ccd->cam_cap.CanPulseGuide ? "Yes" : "No");
		L_print ("Can abort exposure?... %s\n", 
								          ccd->cam_cap.CanAbort ? "Yes" : "No");
		L_print ("Can stop exposure?... %s\n", 
								           ccd->cam_cap.CanStop ? "Yes" : "No");
		return TRUE;
	}
	return FALSE;
}

gboolean ccdcam_close (void)
{
	/* Close the CCD camera */
	
	if (!ccd->Open)  /* return if not opened */
		return TRUE;
	
	switch (ccd->device) {
		#ifdef HAVE_QSI
		case QSI:
			if (!camera_connect (FALSE, NULL))
				return show_error (__func__, "Unable to close CCD camera");
			ports[USBCCD].guide_pulse = telescope_d_pulse;
		    ports[USBCCD].guide_start = telescope_d_start;
		    ports[USBCCD].guide_stop = telescope_d_stop;
			break;
		#endif
		#ifdef HAVE_SX_CAM
		case SX:
			sx_cancel_exposure ();
			if (!camera_connect (FALSE, NULL))
				return show_error (__func__, "Unable to close CCD camera");
		    ports[USBCCD].guide_pulse = telescope_d_pulse;
		    ports[USBCCD].guide_start = telescope_d_start;
		    ports[USBCCD].guide_stop = telescope_d_stop;
			break;
		#endif
		case OTHER:
			/* Other camera stuff */
			break;
		default:
			break;
	}
	
	/* Free any allocated memory */
	
	if (ccd->r161) {
		g_free (ccd->r161);	
		ccd->r161 = NULL;
	}
	
	if (ccd->db163) {
		g_free (ccd->db163);
		ccd->db163 = NULL;
	}
	
	if (ccd->ff161) {
		g_free (ccd->ff161);
		ccd->ff161 = NULL;
	}
	
	if (ccd->ff163) {
		g_free (ccd->ff163);
		ccd->ff163 = NULL;
	}
	
	if (ccd->img.mode[GREY].hist) {
		g_free (ccd->img.mode[GREY].hist);
		ccd->img.mode[GREY].hist = NULL;
	}
	
	file_saved (ccd, TRUE);  /* Set 'save file' button/menu item insensitive */
	ccd->Open = FALSE;
	ccdcam_init ();
	L_print ("{b}****---->>>> CCD camera closed\n");
	return TRUE;
}

void ccdcam_set_exposure_data (struct exposure_data *exd)
{
	/* All exposures are initiated via the task list.  The task handler calls
	 * this routine to set the data for this exposure.  Note that the exposure 
	 * type and filter type are pointers to elements of an array of strings 
	 * in the task list code.  This is OK since 'Expose' tasks aren't executed 
	 * asynchronously, so the task list data won't change until the exposure has
	 * been completed.
	 */
	
	ccd->exd.req_len = exd->req_len;
	ccd->exd.ccdtemp = exd->ccdtemp;
	ccd->exd.ExpType = exd->ExpType;
	ccd->exd.filter = exd->filter;
	
	switch (ccd->device) {
		case QSI:
	
			/* Convert from user to internal coordinates: origin of coordinates
			 * begins at (0,0) not (1,1), and v-coordinate increases downwards,
			 * not upwards.  
			 */
		
			ccd->exd.h_top_l = exd->h_top_l - 1;
			ccd->exd.h_bot_r = exd->h_bot_r - 1;
			ccd->exd.v_top_l = ccd->cam_cap.max_v - exd->v_bot_r;
			ccd->exd.v_bot_r = ccd->cam_cap.max_v - exd->v_top_l;
		
			/* Note that h_pix and v_pix are the number of pixels in the
			 * binned image data, whereas the top_l and bot_r values refer to
			 * the coordinates of the selected area in actual CCD photocells and
			 * so do not vary with binning.
			 */
			
			ccd->exd.h_bin = exd->h_bin;
			ccd->exd.v_bin = exd->v_bin;
			ccd->exd.h_pix = (ccd->exd.h_bot_r - ccd->exd.h_top_l+1)/exd->h_bin;
			ccd->exd.v_pix = (ccd->exd.v_bot_r - ccd->exd.v_top_l+1)/exd->v_bin;
			break;
		case SX:

			/* Convert from user to internal coordinates: origin of coordinates
			 * begins at (0,0) not (1,1), and v-coordinate increases downwards,
			 * not upwards.  
			 */
		
			ccd->exd.h_top_l = exd->h_top_l - 1;
			ccd->exd.h_bot_r = exd->h_bot_r - 1;
			ccd->exd.v_top_l = ccd->cam_cap.max_v - exd->v_bot_r;
			ccd->exd.v_bot_r = ccd->cam_cap.max_v - exd->v_top_l;
		
			ccd->exd.h_bin = exd->h_bin;
			if (ccd->cam_cap.IsInterlaced && exd->v_bin > 1 && exd->v_bin%2)
				L_print ("{o}Vertical binning value restricted to even numbers "
						 "for interlaced camera: using %dx%d\n",
						  exd->h_bin, --exd->v_bin);
			ccd->exd.v_bin = exd->v_bin;
			ccd->exd.h_pix = ccd->exd.h_bot_r - ccd->exd.h_top_l + 1;
			ccd->exd.v_pix = ccd->exd.v_bot_r - ccd->exd.v_top_l + 1;
			break;
		default:
			break;
	}
}

gboolean ccdcam_start_exposure (void)
{
    /* Set the exposure data in the camera and start the exposure */
    
    gushort h, v;
	
	if (!ccd->Open) {  /* return if not opened */
		L_print ("{o}Can't start exposure; CCD camera is not connected!\n");
		return FALSE;
	}
	
	if (ccd->ds9.Invert_h)
		h = ccd->cam_cap.max_h - (ccd->exd.h_top_l + ccd->exd.h_pix);
	else
		h = ccd->exd.h_top_l;
	if (ccd->ds9.Invert_v)
		v = ccd->cam_cap.max_v - (ccd->exd.v_top_l + ccd->exd.v_pix);
	else
		v = ccd->exd.v_top_l;
	
	switch (ccd->device) {
		case QSI:
			/* NOTE: QSI expects location and dimensions of image to be in 
			 * binned coordinates.
			 */
			if (!camera_set_imagearraysize (
									(long) (h / ccd->exd.h_bin),
									(long) (v / ccd->exd.v_bin),
									(long) ccd->exd.h_pix,
									(long) ccd->exd.v_pix,
									(long) ccd->exd.h_bin,
									(long) ccd->exd.v_bin))
				return show_error (__func__, "Unable to set camera image size");
			break;
		case SX:
			if (!camera_set_imagearraysize (
									(long) h,
									(long) v,
									(long) ccd->exd.h_pix,
									(long) ccd->exd.v_pix,
									(long) ccd->exd.h_bin,
									(long) ccd->exd.v_bin))
				return show_error (__func__, "Unable to set camera image size");
			break;
		case OTHER:
			/* Other camera stuff */
			break;
		default:
			break;
	}
	
	/* The call to either camera_start_exposure or camera_get_exposuretime 
	 * must return fits.date_obs in CCYY-MM-DDThh:mm:ss(.sss) format.
	 */
	 
	if (!camera_start_exposure (ccd->fits.date_obs, ccd->exd.req_len,
	                           (strcmp (ccd->exd.ExpType, "BIAS") ? 
								strcmp (ccd->exd.ExpType, "DARK") : 
								FALSE)))
		return show_error (__func__, "Unable to start exposure");
	ccd->exd.exp_start = loop_elapsed_since_first_iteration ();

	/* Set the status bar message on the main application window */

	set_exposure_buttons (TRUE);
	ccd->Expose = TRUE;
	return TRUE;
}

gboolean ccdcam_cancel_exposure (void)
{
	/* Cancel the exposure.  This is used to stop a current exposure
	 * without reading the data.
	 */
	
	if (ccd->cam_cap.CanAbort)
		if (!camera_cancel_exposure ())
			return show_error (__func__, "Error cancelling CCD exposure");
	
	L_print ("{b}CCD exposure cancelled!\n");
	set_exposure_buttons (FALSE);
	ccd->Expose = FALSE;
	tasks_task_done (T_EXP);
	
	return TRUE;
}

gboolean ccdcam_interrupt_exposure (void)
{
	/* Interrupt the exposure.  This is used to interrupt a current exposure
	 * and then wait for the data to be read.
	 */
	
	if (ccd->cam_cap.CanStop)
		if (!camera_interrupt_exposure ())
			return show_error (__func__, "Error interrupting CCD exposure");

	return TRUE;
}

gboolean ccdcam_image_ready (void)
{
	/* Check to see if the CCD camera image is ready;
	 * if so, return TRUE.
	 */
	
	gboolean ready = FALSE;
	
	if (camera_get_imageready (&ready))
		return ready;
	else
	    return show_error (__func__, "Error checking for downloaded image");
}	

gboolean ccdcam_download_image (void)
{
	/* Get the image data from the driver.  This function is called from outside
	 * the main thread so must not make gtk calls.
	 */
	
	int h, v, bytes;

	/* Get the actual exposure length from the driver.  The call to either 
	 * camera_start_exposure or camera_get_exposuretime must return 
	 * fits.date_obs in CCYY-MM-DDThh:mm:ss(.sss) format.
	 */
	
	if (!camera_get_exposuretime (ccd->fits.date_obs, &ccd->exd.act_len))
	    return show_error (__func__, "Error retrieving exposure time data");
	    
	/* Get the actual number of pixels in the horizontal and vertical
     * directions that the driver has used.
	 */

	if (camera_get_imagearraysize (&h, &v, &bytes)) {
		if (bytes != 2) /* Fix this problem before it ever happens! */
			return show_error (__func__, "Can't proceed - expected two bytes "
							                           "of storage per pixel!");
		ccd->exd.h_pix = (gushort) (h);
		ccd->exd.v_pix = (gushort) (v);
	} else
	    return show_error (__func__, "Error retrieving image array size");
	
	/* Read the data */
	
	if (!camera_get_imagearray (ccd->r161))
		return show_error (__func__, "Failed to read image data from driver");
		
	return TRUE;
}

gboolean ccdcam_process_image (void)
{
	/* Do some processing on the image data and tidy up after the exposure */
	
	image_get_stats (ccd, C_GREY);
	
	/* Optionally embed the image in the full chip area */
	
	if (ccd->FullFrame)
		image_embed_data (ccd);
	
	/* Debayer the data (if a colour image) */
	
	if (ccd->Debayer && ccd->exd.h_bin == 1 && ccd->exd.v_bin == 1) {
		if (ccdcam_debayer ())
			image_get_stats (ccd, C_RGB);
		else
			return show_error (__func__, "Failed to debayer image data");
	}
		
	/* Set the date, start time, telescope pointing information and focuser
	 * position and temperature to appear in the FITS header.  We call 
	 * set_fits_data with UseDateobs set to TRUE on the assumption that 
	 * fits.date_obs has already been filled with the start time of the 
	 * exposure.  Don't query the telescope controller and focuser if
	 * this image is being saved during autofocus calibration or
	 * autofocusing.
	 */
	
	set_fits_data (ccd, NULL, TRUE, !loop_focus_is_focusing());
	
	/* Set 'file saved' condition to FALSE */
	
	file_saved (ccd, FALSE);

	/* If autosave is selected, schedule the image to be saved */
	
	if (ccd->AutoSave)
		loop_save_image (CCD);
	
	/* Beep if requested */
	
	if (ccd->Beep)
		gdk_beep ();

	/* Tidy up */

	set_exposure_buttons (FALSE);
	ccd->Expose = FALSE;
	tasks_task_done (T_EXP);
	
	return TRUE;
}

gboolean ccdcam_debayer (void)
{
	/* Debayer the data */
	
	gint tile;
	
	if (!ccd->r161)
		return FALSE;
	if (ccd->FullFrame && !ccd->ff161)
		return FALSE;
	
	tile = debayer_get_tile (ccd->bayer_pattern, ccd->exd.h_top_l, 
							 ccd->exd.v_top_l);
	
	switch (ccd->imdisp.debayer) {
		case DB_SIMP:
			dc1394_bayer_Simple_uint16 (
			                            ccd->r161, ccd->db163, 
										ccd->exd.h_pix, ccd->exd.v_pix, 
										tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_Simple_uint16 (
										ccd->ff161, ccd->ff163, 
										ccd->cam_cap.max_h, ccd->cam_cap.max_v,
										tile, 16);
			break;
		case DB_NEAR:
			dc1394_bayer_NearestNeighbor_uint16 (
			                            ccd->r161, ccd->db163, 
										ccd->exd.h_pix, ccd->exd.v_pix,
										tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_NearestNeighbor_uint16 (
										ccd->ff161, ccd->ff163, 
										ccd->cam_cap.max_h, ccd->cam_cap.max_v,
									    tile, 16);
			break;
		case DB_BILIN:
			dc1394_bayer_Bilinear_uint16 (
			                            ccd->r161, ccd->db163,
										ccd->exd.h_pix, ccd->exd.v_pix, 
										tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_Bilinear_uint16 (
										ccd->ff161, ccd->ff163, 
									    ccd->cam_cap.max_h, ccd->cam_cap.max_v,
									    tile, 16);
			break;
		case DB_QUAL:
			dc1394_bayer_HQLinear_uint16 (
			                            ccd->r161, ccd->db163, 
										ccd->exd.h_pix, ccd->exd.v_pix, 
										tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_HQLinear_uint16 (
										ccd->ff161, ccd->ff163, 
										ccd->cam_cap.max_h, ccd->cam_cap.max_v,
										tile, 16);
			break;
		case DB_DOWN:
			dc1394_bayer_Downsample_uint16 (
			                            ccd->r161, ccd->db163, 
										ccd->exd.h_pix, ccd->exd.v_pix, 
										tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_Downsample_uint16 (
										ccd->ff161, ccd->ff163, 
									    ccd->cam_cap.max_h, ccd->cam_cap.max_v,
									    tile, 16);
			break;
		case DB_GRADS:
			dc1394_bayer_VNG_uint16 (
			                            ccd->r161, ccd->db163, 
									    ccd->exd.h_pix, ccd->exd.v_pix, 
									    tile, 16);
		    if (ccd->FullFrame)
				dc1394_bayer_VNG_uint16 (
										ccd->ff161, ccd->ff163, 
									    ccd->cam_cap.max_h, ccd->cam_cap.max_v,
										tile, 16);
			break;
	}
	return TRUE;
}

gboolean ccdcam_set_temperature (gboolean *AtTemperature)
{
	/* If the requested temperature is greater than the current heatsink
	 * temperature (c_amb), issue a warning but return 'AtTemperature' as TRUE.
	 * If the current temperature is within TOL of the requested temperature,
	 * return 'AtTemperature' as TRUE.
	 * If the requested temperature is less than the current heatsink 
	 * temperature (c_amb), set the CCD to the requested temperature and return 
	 * 'AtTemperature' as FALSE.   
	 * Note that the current temperature must be retrieved via a call to 
	 * ccd->get_state prior to calling this routine.  Cameras that do not return
	 * an ambient or heatsink temperature should set c_amb equal to the CCD
	 * temperature (c_ccd).
	 * Return FALSE if there is an error, TRUE otherwise. 
	 */
	
	if (!ccd->Open) {  /* Return if not opened */
		L_print ("{o}Can't set temperature; CCD camera is not connected!\n");
		*AtTemperature = FALSE;
		return FALSE;
	}
	
	if (ccd->state.IgnoreCooling) { /* Ignore cooling; pretend it's OK */
		*AtTemperature = TRUE;
		return TRUE;
	}
	
	if (!ccd->cam_cap.CanSetCCDTemp) { /* Can't set CCD temperature, so */
		*AtTemperature = TRUE;         /*  pretend it's OK              */
		return TRUE;
	}

	if (ccd->cam_cap.CanSetCCDTemp) {
		if (ccd->exd.ccdtemp > ccd->state.c_amb) {
			if (ccd->device == QSI)
				L_print ("{o}Requested temperature is greater than ambient "
						 "(heatsink) temperature!  Continuing anyway...\n");
			else if (ccd->device == SX)
				L_print ("{o}Requested temperature is greater than current CCD "
						 "temperature!  Continuing anyway...\n");
			else
				return show_error (__func__, "Unknown camera type");
			
			*AtTemperature = TRUE;
			return TRUE;
		}
		L_print ("Waiting for CCD to reach %5.1f +/- %3.1fC; now at %5.1fC\n",
						  ccd->exd.ccdtemp, ccd->state.c_tol, ccd->state.c_ccd);
		if (ABS (ccd->exd.ccdtemp - ccd->state.c_ccd) <= ccd->state.c_tol) {
			*AtTemperature = TRUE;
			return TRUE;
		}
		if (!ccd->set_state (S_TEMP, 0, ccd->exd.ccdtemp, ccd->id)) {
			*AtTemperature = FALSE;
			return FALSE;
		}
		if (!ccd->set_state (S_COOL, TRUE, 0.0, ccd->id)) {
			*AtTemperature = FALSE;
			return FALSE;
		}
	}
	
	*AtTemperature = FALSE;
	return TRUE;
}

void ccdcam_set_fast_readspeed (gboolean Set)
{
	/* If the fast focus readout option has been selected:
	 * If Set == TRUE, store the current readout speed and set to fast speed
	 * IF Set == FALSE, restore readout speed to previously stored value
	 */
	
	static gint speed = -1;
	
	if (ccd->FastFocus) {
		if (Set) {
			ccd->get_state (&ccd->state, TRUE);
			speed = ccd->state.read_speed;
			ccd->set_state (S_SPEED, CCD_SPEED_HIGH, 0.0);
		} else {
			if (speed != -1)
				ccd->set_state (S_SPEED, speed, 0.0);
		}
	}
}

gboolean ccdcam_measure_HFD (gboolean Initialise, gboolean Plot, gint box, 
						     struct exposure_data *exd, gdouble *hfd)
{
	gushort h, v, pix_h, pix_v;
	gushort npix, pix_split, niter;
	gint tick;
	gdouble *flux_h, *flux_v, totflux, bg, val;
	gdouble hfd_old, hfd_max, hfd_u, hfd_l;
	static gdouble hfd_plot = 0.0;
	gdouble f, flux = 0.0;
	
	gushort MAX_HFD_ITER = 100;
	
	/* If the requested image area is not already a subset of the full chip,
	 * set the selected imaging area to be a box centred on the brightest
	 * pixel.
	 */
	
	if (Initialise) {
		exd->h_pix = ccd->exd.h_pix;
		exd->v_pix = ccd->exd.v_pix;
		if (ccd->exd.h_bot_r - ccd->exd.h_top_l + 1 == ccd->cam_cap.max_h &&
			ccd->exd.v_bot_r - ccd->exd.v_top_l + 1 == ccd->cam_cap.max_v) {
			
			/* Remember to set the image bounds to be the actual (unbinned)
			 * location on the chip, starting at (1, 1) in the lower left
			 * corner.  This requires us to invert the v-coordinate since this
		     * will be inverted back again in the call to 
			 * ccdcam_set_exposure_data in the autofocus calibration thread in
			 * loop.c.
			 */
				
			exd->h_top_l = CLAMP (ccd->img.max[GREY].h * ccd->exd.h_bin + 
								  1 - box, 1, ccd->cam_cap.max_h);
			exd->v_top_l = CLAMP (ccd->cam_cap.max_v - (ccd->img.max[GREY].v * 
								  ccd->exd.v_bin + box), 1, ccd->cam_cap.max_v);
			exd->h_bot_r = CLAMP (ccd->img.max[GREY].h * ccd->exd.h_bin + 
								  1 + box, 1, ccd->cam_cap.max_h);
			exd->v_bot_r = CLAMP (ccd->cam_cap.max_v - (ccd->img.max[GREY].v * 
								  ccd->exd.v_bin - box), 1, ccd->cam_cap.max_v);
			exd->h_pix = (exd->h_bot_r - exd->h_top_l + 1) / ccd->exd.h_bin;
			exd->v_pix = (exd->v_bot_r - exd->v_top_l + 1) / ccd->exd.v_bin;
		}
		if (Plot) {
			#ifdef HAVE_LIBGRACE_NP
			Grace_SetXAxis (0, 0.0, (gfloat) exd->h_pix);
			tick = exd->h_pix / 10;
			if (tick%2) tick += 1;
			if (!tick) tick = 1;
			Grace_XAxisMajorTick (0, tick);
			Grace_Update ();
			hfd_plot = 0.0;
			#endif
		}
		return TRUE;
	}
	
    /* Calculate centre-of-weight of image.  Note that this is 0-based, and that
	 * 0 means C-of-W is at the centre of the zeroth pixel, 1 is at the centre
	 * of the first pixel (i.e. the second pixel in the row) etc.  Note also
	 * that the calculated value is relative to (0, 0) at the top left corner
	 * of the analysed image (which may be a subset of the full chip area).
	 */
  
    flux_h = (double *) g_malloc0 (ccd->exd.h_pix * sizeof (double));
    flux_v = (double *) g_malloc0 (ccd->exd.v_pix * sizeof (double));
    totflux = 0;        /* Set background to 4 sigma above mode */
    bg = ccd->img.mode[GREY].peakbin + 4.0 * ccd->img.stdev[GREY].val;
    for (v = 0; v < ccd->exd.v_pix; v++)
		for (h = 0; h < ccd->exd.h_pix; h++) {
			val = MAX (ccd->r161[v * ccd->exd.h_pix + h] - bg, 0);
			flux_h[h] += val;
			flux_v[v] += val;
			totflux += val;
		}
	
	ccd->img.mean[GREY].h = 0;	
	for (h = 0; h < ccd->exd.h_pix; h++)
		ccd->img.mean[GREY].h += h * flux_h[h];
	ccd->img.mean[GREY].h /= totflux;  /* Mean h position */
	ccd->img.mean[GREY].v = 0;	
	for (v = 1; v < ccd->exd.v_pix; v++)
		ccd->img.mean[GREY].v += v * flux_v[v];
	ccd->img.mean[GREY].v /= totflux;  /* Mean v position */
	
	if (isnan (ccd->img.mean[GREY].h) || isnan (ccd->img.mean[GREY].v)) {
		L_print ("{o}Can't find a star for focusing!\n");
		g_free (flux_h);
		g_free (flux_v);
		return FALSE;
	}
	
	/* Find half-flux diameter via binary search */
	
	hfd_max = 2 * MIN (MIN (ccd->exd.h_pix - ccd->img.mean[GREY].h, 
							ccd->img.mean[GREY].h),
	                   MIN (ccd->exd.v_pix - ccd->img.mean[GREY].v, 
							ccd->img.mean[GREY].v));
	*hfd = hfd_max / 2;  /* Starting value */
	
	hfd_old = 0.0;    /* Previous guess */
	hfd_u = hfd_max;  /* Upper limit    */
	hfd_l = 0.0;      /* Lower limit    */
	niter = 0;
	while (ABS (*hfd - hfd_old) / *hfd > 0.001) {
		
		/* For a given row of 'npix' pixels, with pixels of size 'pix_size' 
		 * units on a side, and starting image coordinate of 'pix_h', 'pix_v', 
		 * return the flux in this row bounded by a circle of diameter hfd 
		 * centred at ('meanh', 'meanv').  Here, pix starts at (0,0) in top left
		 * corner of top left pixel.  Repeat this for each row of data 
		 * containing the hfd circle, and sum total fluxes.
		 */
		
		npix = (gint) MIN (*hfd + 10, hfd_max); /* Allow space surrounding hfd*/
		if (npix % 2)                     /* Make sure even no. pixels in row */
			npix--;
	
		pix_h = (gint) ccd->img.mean[GREY].h - npix / 2;
		for (pix_v = (gint) ccd->img.mean[GREY].v - npix / 2, flux = 0.0; 
			 pix_v < (gint) ccd->img.mean[GREY].v + npix / 2; pix_v++) {
	
			/* Divide pixel into 100 each way if hfd <= 10.0 */
		    pix_split = *hfd > 10.0 ? 1 : 100;
			f = HFD_row_flux (ccd->r161+ccd->exd.h_pix*pix_v+pix_h, npix,
							  pix_split, (gdouble) pix_h, (gdouble) pix_v, 
							  (gdouble) ccd->img.mean[GREY].h, 
				              (gdouble) ccd->img.mean[GREY].v, 
							  *hfd, bg);
			flux += f;
		}
		hfd_old = *hfd;
		if (flux > 0.5 * totflux) {
			hfd_u = *hfd;
			*hfd = (*hfd + hfd_l) / 2.0;
		} else {
			hfd_l = *hfd;
			*hfd = (*hfd + hfd_u) / 2.0;
		}
		if (niter++ == MAX_HFD_ITER) {
			L_print ("{r}Maximum iterations reached in HFD calculation... "
					 "Giving up!\n");
			g_free (flux_h);
			g_free (flux_v);
			return FALSE;
		}			
	}
	L_print ("HFD flux: %.4f, total flux: %.4f, ratio: %.4f, HFD: %.4f, "
			 "position: %d\n", flux, totflux, flux / totflux, *hfd,
			 exd->focus_pos);
	
	/* Plot graphs and re-scale if necessary */
	
	if (Plot) {
		#ifdef HAVE_LIBGRACE_NP
		flux = 0.0;
		Grace_ErasePlot (0, 0);
		for (h = 0; h < ccd->exd.h_pix; h++) {
			flux = MAX (flux, flux_h[h]);
			Grace_PlotPoints (0, 0, h, flux_h[h]);
		}
		Grace_SetYAxis (0, -flux / 10.0, flux * 1.1);
		
		if (*hfd > hfd_plot) {
			hfd_plot = *hfd;
			Grace_SetYAxis (1, -hfd_plot / 10.0, hfd_plot * 1.1);
			tick = hfd_plot * 1.2 / 10;
			if (tick%2) tick += 1;
			if (!tick) tick = 1;
			Grace_YAxisMajorTick (1, tick);
		}
		Grace_PlotPoints (1, 0, exd->focus_pos, *hfd);
		Grace_Update ();
		#endif
	}
	
	/* Set new bounds for next image */
	
	exd->h_top_l = CLAMP (ccd->exd.h_top_l + (ccd->img.mean[GREY].h - 
						  ccd->exd.h_pix / 2) * ccd->exd.h_bin + 1, 
						  1, ccd->cam_cap.max_h);
	exd->v_top_l = CLAMP (ccd->cam_cap.max_v - (ccd->exd.v_top_l + 
			             (ccd->img.mean[GREY].v + ccd->exd.v_pix / 2) * 
						  ccd->exd.v_bin), 1, ccd->cam_cap.max_v);
	exd->h_bot_r = CLAMP (ccd->exd.h_top_l + 
			             (ccd->img.mean[GREY].h + ccd->exd.h_pix / 2) * 
						  ccd->exd.h_bin + 1, 1, ccd->cam_cap.max_h);
	exd->v_bot_r = CLAMP (ccd->cam_cap.max_v - (ccd->exd.v_top_l + 
			             (ccd->img.mean[GREY].v - ccd->exd.v_pix / 2) * 
						  ccd->exd.v_bin), 1, ccd->cam_cap.max_v);
	
	g_free (flux_h);
	g_free (flux_v);
	return TRUE;
}

static gdouble HFD_row_flux (gushort row[], gushort npix, gushort pix_split,
							 gdouble pix_h, gdouble pix_v, 
							 gdouble meanh, gdouble meanv, 
                             gdouble hfd, gdouble bg)
{
	/* Return the total flux lying within the circle of given hfd, for the
	 * given row of pixels.
	 */
	
	gushort corners;
	gushort i, h, v;
	gdouble hfd2, flux;
	gdouble pix_h_orig, pix_v_orig;
	 
	/* Check where corners of each pixel lie relative to HFD circle... 
	 * Note that relative to the top left corner of the pixel, meanh and meanv
	 * are actually shifted by 0.5 pixels.  (In other words, the top left corner
	 * of the top left pixel (pixel 0) lies at (0, 0), but meanh = 0 and 
	 * meanv = 0 means that the centre-of-weight lies at the middle of that 
	 * pixel, i.e. at (0.5, 0.5)).  A value of 0.25 of the total pixel (or sub-
	 * pixel) flux is assigned for each pixel corner that lies within the HFD
	 * circle - this seems to give accurate results even for a 'star' of just
	 * one pixel.
	 */	 
	 
	hfd2 = hfd * hfd / 4.0;
	flux = 0.0;
	pix_v_orig = pix_v;
	pix_h_orig = pix_h - 1;
	for (i = 0; i < npix; i++) {
		pix_h_orig++;
	 	for (v = 0; v < pix_split; v++) {
			pix_v = pix_v_orig + (gdouble) v / pix_split;
	 		for (h = 0; h < pix_split; h++) {
				corners = 0;
				pix_h = pix_h_orig + (gdouble) h / pix_split;
	 			/* Top left corner */
	 			if (pow (pix_h - (meanh + 0.5), 2) + 
	 	    		pow (pix_v - (meanv + 0.5), 2) <= hfd2)
	 				corners++;
	 			/* Top right corner */
	 			if (pow ((pix_h + 1.0 / pix_split) - (meanh + 0.5), 2) + 
	 	    		pow (pix_v - (meanv + 0.5), 2) <= hfd2)
	 				corners++;
	 			/* Bottom left corner */
	 			if (pow (pix_h - (meanh + 0.5), 2) + 
	 	    		pow ((pix_v + 1.0 / pix_split) - (meanv + 0.5), 2) < hfd2) 
	 				corners++;
	 			/* Bottom right corner */
	 			if (pow ((pix_h + 1.0 / pix_split) - (meanh + 0.5), 2) + 
	 	   		    pow ((pix_v + 1.0 / pix_split) - (meanv + 0.5), 2) < hfd2)
	 				corners++;
				
				flux += corners / 4.0 * MAX (row[i] - bg, 0);
	 		}
	 	}
	}
		
	/* Scale value according to pix_split squared */
	
	return flux / pow (pix_split, 2);
}

gboolean ccdcam_plot_temperatures (void)
{
	/* Plot CCD temperatures and cooler power using Grace.
	 * Note that the current values must be retrieved via a call to 
	 * ccd->get_state prior to calling this routine.
	 */
	
	#ifdef HAVE_LIBGRACE_NP
	static guint prev_loop = 0, start_time = 0;
	guint now;
	gfloat minutes;
	const gfloat GRACE_MINS = 8.0; /* Width of scrolling plot in minutes */
	
	if (ccd->graph.reset) {        /* Initialise the plots */
		ccd->graph.reset = FALSE;
				
		Grace_SetXAxis (0, 0.0, GRACE_MINS);
		Grace_SetYAxis (0, -5.0, 105.0);      /* Cooler power, percent */
		Grace_SetXAxis (1, 0.0, GRACE_MINS);
		if (ccd->cam_cap.CanSetCCDTemp)       /* Temperatures */
			Grace_SetYAxis (1, ccd->state.c_amb - 55.0,ccd->state.c_amb + 15.0);
		else
			Grace_SetYAxis (1, -60.0, 40.0);			
		
        start_time = loop_elapsed_since_first_iteration ();
		prev_loop = start_time;
	} else {
	    now = loop_elapsed_since_first_iteration ();
		if ((now - prev_loop) > 500) { /* Don't update more often than every  */
			prev_loop = now;           /* 500ms or Grace becomes unresponsive */
			minutes = (now - start_time) / 60000.0;

			if (minutes > GRACE_MINS) {
				Grace_SetXAxis (0, minutes - GRACE_MINS, minutes);
				Grace_SetXAxis (1, minutes - GRACE_MINS, minutes);
			}
			
			if (ccd->cam_cap.CanGetCoolPower)
				Grace_PlotPoints (0, 0, minutes, ccd->state.c_power); 
			if (ccd->cam_cap.CanSetCCDTemp) {
				Grace_PlotPoints (1, 0, minutes, ccd->state.c_amb);
				Grace_PlotPoints (1, 1, minutes, ccd->state.c_ccd);
			}
			if (!Grace_Update ()) {
				ccd->graph.reset = TRUE;
				return FALSE;
			}
	    }
	}
	#endif
	return TRUE;
}

gboolean ccdcam_get_status (void)
{
	/* Get the camera status */
	
	if (!ccd->Open)
		return FALSE;
	
	return ccd->get_state (&ccd->state, FALSE, ccd->id);
}

#ifdef HAVE_QSI
static void ccdcam_qsi_error_func (int *err, const char *func, char *c)
{
	/* Error message callback from qsi camera routines */
	
	if (err)
		L_print ("{r}%s: %s\n", func, c);
	else
		G_print ("%s: OK\n", func);
}
#endif

void ccdcam_sx_error_func (int *err, const char *func, char *c)
{
	/* Error message callback from sx camera routines */
	
	if (err)
		L_print ("{r}%s: %s\n", func, c);
	else
		G_print ("%s: OK\n", func);
}

struct cam_img *get_ccd_image_struct (void)
{
	return ccd;
}
