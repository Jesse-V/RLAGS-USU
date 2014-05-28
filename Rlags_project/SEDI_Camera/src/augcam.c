/******************************************************************************/
/*                       AUTOGUIDER CAMERA ROUTINES                           */
/*                                                                            */
/* All the routines for interfacing with the autoguider camera are contained  */
/* in this module.  The camera is assumed to be either:                       */
/*                                                                            */
/*   An Imaging Source USB or firewire astronomy camera via Unicap            */
/*   Or a camera connected to an Imaging Source DFG/1394 firewire converter   */
/*                                                      via Unicap            */
/*   Or a V4L compatible webcam via Unicap or GoQat's native V4L support      */
/*   Or a Starlight Xpress CCD camera                                         */
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#ifdef HAVE_LIBV4L2
#include <libv4l2.h>
#endif
#ifdef HAVE_LIBGRACE_NP
#include <grace_np.h>
#endif
#ifdef HAVE_LIBPARAPIN
#include <parapin.h>
#endif

#define GOQAT_AUGCAM
#include "interface.h"
#ifdef HAVE_SX
#include "sx.h"
#endif

#define AUG_BLACK         0  /* Black level for autoguider data        */
#define AUG_WHITE_1     255  /* White level for 1-byte autoguider data */
#define AUG_WHITE_2   65535  /* White level for 2-byte autoguider data */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping   */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

#ifdef HAVE_SX
struct sx_cam a_cam;
#endif
static gushort ndark = 0;  /* Number of dark exposures in dark frame   */
static struct cam_img aug_cam_obj, *aug;
static FILE *fps = NULL;               /* Star positions file          */
static FILE *fpd = NULL;               /* Dark frame file              */

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void augcam_init (void);
gboolean augcam_open (void);
gboolean augcam_close (void);
gboolean augcam_start_exposure (void);
gboolean augcam_image_ready (void);
gboolean augcam_capture_exposure (void);
static void augcam_set_image_size (gushort width, gushort height);
void augcam_set_camera_binning (gushort h_bin, gushort v_bin);
gboolean augcam_set_vidval (enum Range range, gdouble value);
static void initialise_image_window (enum CamDevice dev);
#ifdef HAVE_LIBV4L2
static void V4L_initialise_bcg_sliders (gchar *name, gint id, gint item);
#endif
gboolean augcam_write_starpos_time (void);
gboolean augcam_write_starpos (gushort WormPos);
static void augcam_draw_starpos (void);
void augcam_read_dark_frame (void);
void augcam_flush_dark_frame (void);
static void augcam_write_dark_frame (void);
#ifdef HAVE_UNICAP
static gboolean OpenUnicapDevice (void);
static gboolean CloseUnicapDevice (void);
void augcam_unicap_new_frame_cb (unicap_event_t event, 
								 unicap_handle_t handle, 
						         unicap_data_buffer_t *buffer,
								 gpointer *data);
#endif  /* UNICAP */
#ifdef HAVE_LIBV4L2
static gboolean OpenV4LDevice (gchar *device);
static gboolean CloseV4LDevice (void);
static gboolean GrabV4LBuffer (void);
static gboolean v4l2ioctl (gint fh, gint request, void *arg);
#endif  /* V4L2 */
#ifdef HAVE_SX
gboolean augcam_get_sx_cameras (int *num);
static gboolean OpenSXDevice (void);
static gboolean CloseSXDevice (void);
static gboolean OpenSXGuideHead (void);
static gboolean CloseSXGuideHead (void);
struct sx_cam *augcam_get_sx_cam_struct (void);
#endif /* SX */
struct cam_img *get_aug_image_struct (void);

/******************************************************************************/
/*                         VIDEO CONVERSION FUNCTIONS                         */
/******************************************************************************/

static gboolean convert_to_grey (enum CamDevice device, guchar *image, 
								 guchar *rgb, gushort *grey, gushort *disp_16_1,
								 guchar *disp_8_3, gint x, gint y);
static inline void move_420_block (gint yTL, gint yTR, gint yBL, gint yBR,
                                   gint u, gint v, gint rowPixels, guchar *rgb,
                                   gint bits);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void augcam_init (void)
{
	static gboolean first_pass = TRUE;
	
	/* Initialise the autoguider camera data */
	
	aug = &aug_cam_obj;

	#ifdef HAVE_UNICAP
	aug->u_handle = NULL;
	aug->ugtk_display = NULL;
	aug->ugtk_window = NULL;
	#endif
	aug->vid_buf.buffers = NULL;
	aug->exd.req_len = 1.0;           /* Must match initial setting in Glade */
	aug->exd.h_bin = 1;
	aug->exd.v_bin = 1;
	aug->imdisp.B = AUG_BLACK;
	aug->imdisp.W = AUG_WHITE_1;
	aug->imdisp.black = AUG_BLACK;
	aug->imdisp.gamma = 1.0;
	aug->canv.cviImage = NULL;
	aug->canv.r.htl = 0;
	aug->canv.r.vtl = 0;
	aug->canv.r.hbr = AUGCANV_H;
	aug->canv.r.vbr = AUGCANV_V;
	aug->canv.cursor_x = 0;
	aug->canv.cursor_y = 0;
	aug->canv.NewRect = FALSE;
	aug->canv.Centroid = TRUE;        /* Must match initial setting in Glade */
	aug->dark.dvadr = NULL;
	aug->dark.Capture = FALSE;
	aug->dark.Subtract = FALSE;       /* Must match initial setting in Glade */
	aug->autog.s.Rate_NS = 10.0;
	aug->autog.s.Rate_EW = 10.0;
	aug->autog.s.Uvec_N[0] = 0.0;
	aug->autog.s.Uvec_N[1] = -1.0;
	aug->autog.s.Uvec_E[0] = -1.0;
	aug->autog.s.Uvec_E[1] = 0.0;
	aug->autog.s.CalibDec = 999.0;    /* An 'error' value, outside bounds    */
	aug->autog.s.CalibGuideSpeed = 999.0; /* An 'error' value, as above      */
	aug->autog.Write = FALSE;         /* Must match initial setting in Glade */
	aug->autog.Worm = FALSE;          /* Must match initial setting in Glade */
	aug->autog.Guide = FALSE;
	aug->autog.Pause = FALSE;
	aug->graph.zoom = 1.0;
	aug->graph.showh = TRUE;
	aug->graph.showv = FALSE;
	aug->graph.draw = FALSE;
	aug->graph.reset = TRUE;
	aug->ds9.stream = NULL;
	aug->ds9.window = NULL;
	aug->ds9.display = NULL;
	aug->set_state = NULL;
	aug->get_state = NULL;
	aug->vadr = NULL;	
	aug->id = AUG;
	aug->devnum = -1;
	aug->fd = 0;
	aug->disp_16_1 = NULL;
	aug->rgb = NULL;
	aug->disp_8_3 = NULL;
	aug->idp = NULL;
	aug->Open = FALSE;                /* Must match initial setting in Glade */
	aug->Expose = FALSE;
	aug->AutoSave = FALSE;
	aug->SavePeriodic = FALSE;
	aug->FileSaved = TRUE;
	aug->Error = FALSE;
	
	if (first_pass) {
		aug->imdisp.greyscale = MAXIM;/* Must match initial setting in Glade */
		aug->device = NO_CAM;
		first_pass = FALSE;
	}
}

gboolean augcam_open (void)
{
	/* Open the autoguider camera */
	
	gchar V4Ldev[12];
	gchar *err;
	gchar *FilePath = NULL;
	
	switch (aug->device) {
		#ifdef HAVE_UNICAP
		case UNICAP:
			L_print ("{b}****---->>>> Opening Unicap camera\n");
			if (!OpenUnicapDevice ()) {
				return show_error (__func__, "Error opening Unicap camera");
			}
			aug->vid_buf.pixfmt = V4L2_PIX_FMT_GREY; /* User must select grey */
			aug->imdisp.W = AUG_WHITE_1;             /*  (Y800) output        */
			aug->exd.FreeRunning = TRUE;
			initialise_image_window (aug->device);
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
		#endif
        #ifdef HAVE_LIBV4L2
		case V4L:
			strncpy (V4Ldev, menu.aug_camera+5, 11);
			V4Ldev[11] = '\0';
			L_print ("{b}****---->>>> Opening V4L camera on %s..\n", V4Ldev);
			if (!OpenV4LDevice (V4Ldev)) {
				return show_error (__func__, "Error opening V4L camera");
			}
			aug->imdisp.W = AUG_WHITE_1;
			aug->exd.FreeRunning = TRUE;
			initialise_image_window (aug->device);
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
        #endif
        #ifdef HAVE_SX
        case SX:
			L_print ("{b}****---->>>> Opening Starlight Xpress camera\n");
			if (!OpenSXDevice ()) {
				return show_error (__func__, "Error opening Starlight "
											 "Xpress camera");
			}
			aug->imdisp.W = aug->cam_cap.max_adu;
			aug->exd.FreeRunning = FALSE;
			initialise_image_window (aug->device);
			ports[USBAUG].guide_pulse = sxc_pulseguide;
		    ports[USBAUG].guide_start = sxc_guide_start;
		    ports[USBAUG].guide_stop = sxc_guide_stop;
		    if (autog_comms->pnum == USBAUG)
				sxc_set_guide_command_cam (&a_cam);
			break;
        case SX_GH:
			L_print ("{b}****---->>>> Opening Starlight Xpress guide head\n");
			if (!OpenSXGuideHead ()) {
				return show_error (__func__, "Error opening Starlight "
											 "Xpress guide head");
			}
			aug->imdisp.W = aug->cam_cap.max_adu;
			aug->exd.FreeRunning = FALSE;
			initialise_image_window (aug->device);
			/* Can't issue guide commands via guide head, only via main camera*/
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
		#endif
		case NO_CAM:
			return show_error (__func__, "No autoguider camera selected");
			break;
		default:
			return show_error (__func__, "Unknown camera device");
	}
	
	if (aug->cam_cap.max_h > AUGCANV_H || aug->cam_cap.max_v > AUGCANV_V) {
		err = "Autoguider camera image exceeds display limits";
		goto open_err;
	}
    
	/* Allocate storage for image data */
	
	if (!(aug->disp_16_1 = (gushort *) g_malloc0 (aug->cam_cap.max_h * 
									   aug->cam_cap.max_v * sizeof(gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->rgb = (guchar *) g_malloc0 (aug->cam_cap.max_h * 
								aug->cam_cap.max_v * 3 * sizeof (guchar)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->disp_8_3 = (guchar *) g_malloc0 (aug->cam_cap.max_h * 
									 aug->exd.v_pix * 3 * sizeof (guchar)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->dark.dvadr =(gushort *)g_malloc0 (aug->cam_cap.max_h *
									  aug->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->vadr = (gushort *) g_malloc0 (aug->cam_cap.max_h *
								  aug->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->rect.mode[GREY].hist = (guint *) g_malloc0 ((aug->imdisp.W + 1) *
												sizeof (guint)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	/* Open the star positions file, in case needed */
	
	FilePath = g_strconcat (PrivatePath, "/star_pos.csv", NULL);
	if (!(fps = fopen (FilePath, "a"))) {
		err = "Error opening star position file";
		goto open_err;
	}
	g_free (FilePath);
	FilePath = NULL;
	
	/* Open the dark frame file, in case needed.  Create it initially if it
	 * doesn't already exist, then re-open for update.
	 */
	
	FilePath = g_strconcat (PrivatePath, "/dark_frame", NULL);
	if (!(fpd = fopen (FilePath, "a"))) {
		err = "Error opening dark frame file";
		goto open_err;
	}
	fclose (fpd);
	if (!(fpd = fopen (FilePath, "r+"))) {
		err = "Error opening dark frame file";
		goto open_err;
	}
	g_free (FilePath);
	FilePath = NULL;
	
	/* All done! */
	
	tasks_task_done (T_AGN);
	
	aug->Open = TRUE;
	L_print ("{b}****---->>>> Opened autoguider camera\n");
	return TRUE;
	
open_err:

	if (FilePath) {
		g_free (FilePath);
		FilePath = NULL;
	}
	
	aug->Open = TRUE; /* Forces augcam_close to close and release resources */
	augcam_close ();
	return show_error (__func__, err);	
}

gboolean augcam_close (void)
{
	/* Close the autoguider camera */
	
	if (!aug->Open)  /* return if not opened */
		return TRUE;
	
	switch (aug->device) {
		#ifdef HAVE_UNICAP
		case UNICAP:
			if (CloseUnicapDevice () < 0)
				return show_error (__func__, "Error closing "
												    "autoguider camera device");
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
		#endif
        #ifdef HAVE_LIBV4L2
		case V4L:
			if (!CloseV4LDevice ())
				return show_error (__func__, "Error closing "
												    "autoguider camera device");
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
        #endif
        #ifdef HAVE_SX
        case SX:
			if (!CloseSXDevice ())
				return show_error (__func__, "Error closing "
													"autoguider camera device");
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
        case SX_GH:
			if (!CloseSXGuideHead ())
				return show_error (__func__, "Error closing "
													"autoguider camera device");
			ports[USBAUG].guide_pulse = telescope_d_pulse;
		    ports[USBAUG].guide_start = telescope_d_start;
		    ports[USBAUG].guide_stop = telescope_d_stop;
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
	
	/* Free any allocated memory */

	if (aug->disp_16_1) {
		g_free (aug->disp_16_1);
		aug->disp_16_1 = NULL;
	}
	
	if (aug->rgb) {
		g_free (aug->rgb);
		aug->rgb = NULL;
	}
	
	if (aug->disp_8_3) {
		g_free (aug->disp_8_3);
		aug->disp_8_3 = NULL;
	}
	
	if (aug->dark.dvadr) {
		g_free (aug->dark.dvadr);
		aug->dark.dvadr = NULL;
	}
	
	if (aug->vadr) {
		g_free (aug->vadr);
		aug->vadr = NULL;
	}
	
	if (aug->rect.mode[GREY].hist) {
		g_free (aug->rect.mode[GREY].hist);
		aug->rect.mode[GREY].hist = NULL;
	}
	
	if (aug->canv.cviImage) {
        goo_canvas_item_remove (aug->canv.cviImage);
		aug->canv.cviImage = NULL;
	}
	
	/* Close files */
	
	if (fps)
		fclose (fps);
	if (fpd)
		fclose (fpd);
	
	/* Tidy up */
	
	tasks_task_done (T_AGF);
	file_saved (aug, TRUE);  /* Set 'save file' button/menu item insensitive */
	aug->idp = NULL;
	aug->Expose = FALSE;
	aug->Open = FALSE;
	augcam_init ();
	L_print ("{b}****---->>>> Autoguider camera closed\n");
	return TRUE;
}

gboolean augcam_start_exposure (void)
{
	/* Start autoguider camera exposure */

	switch (aug->device) {
		case UNICAP:  /* Nothing to do... These devices are free-running and */ 
		case V4L:     /*  beyond our control.                                */
			return TRUE;
			break;
		#ifdef HAVE_SX
		case SX:
		case SX_GH:
			/* Subsets of image area not allowed - so set array size to 
			 * maximum possible value.
			 */
			sxc_set_imagearraysize (&a_cam, 0, 0, 
									aug->cam_cap.max_h, aug->cam_cap.max_v,
									aug->exd.h_bin, aug->exd.v_bin);
			if (!sxc_start_exposure (&a_cam, aug->fits.date_obs, 
														aug->exd.req_len, TRUE))
				return show_error (__func__, "Unable to start autoguider "
																	"exposure");
			aug->exd.exp_start = loop_elapsed_since_first_iteration ();
			return TRUE;
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
}

gboolean augcam_image_ready (void)
{
	/* Return true if an autoguider image is available.  We could introduce
	 * a delay equal to the exposure length for free running cameras here, but
	 * choose not to so that each grabbed frame is displayed (whether it
	 * represents a new integrated image or not).  Appropriate delays for
	 * free running cameras are introduced elsewhere in the code as required.
	 */
	 
	gboolean Ready;
	
	switch (aug->device) {
		case UNICAP:  /* Nothing to do... These devices are free-running and */ 
		case V4L:     /*  beyond our control.                                */
			return TRUE;
			break;
		#ifdef HAVE_SX
		case SX:
		case SX_GH:
			sxc_get_imageready (&a_cam, &Ready);
			return Ready;
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
}

gboolean augcam_capture_exposure (void)
{
	/* Capture the most recently exposed autoguider camera image and perform 
	 * subsequent processing.  Note that we don't go and grab a frame from the 
	 * Unicap device; instead it gives us one via a callback for each new frame.
	 * So this routine doesn't raise an error for capturing a frame from the 
	 * Unicap device because it doesn't know if there's been one.
	 */
	
	gushort WormPos = 0;
	gint h, v, bytes;
	
	switch (aug->device) {
		case UNICAP:
			/* augcam_unicap_new_frame_cb copies the Unicap buffer to 
			 * aug->rgb, so here we just set aug->idp to point to aug->rgb.
			 * NOTE it is assumed that the user has selected the greyscale
			 * option for the Unicap device so that aug->rgb is actually filled
			 * with one byte per pixel greyscale data.
			 */
		    aug->idp = aug->rgb;
		break;
        #ifdef HAVE_LIBV4L2
		case V4L:
			/* GrabV4LBuffer sets aug->idp to point to most recently available
			 * memory buffer.
			 */
			if (!GrabV4LBuffer ())
				return show_error (__func__, "Error grabbing frame from V4L "
								                                      "device");
			break;
        #endif
        #ifdef HAVE_SX
        case SX:
        case SX_GH:
			aug->exd.exp_end = loop_elapsed_since_first_iteration ();
			if (!sxc_get_exposuretime (&a_cam, aug->fits.date_obs, 
															 &aug->exd.act_len))
				return show_error (__func__, "Error retrieving exposure time");
			if (sxc_get_imagearraysize (&a_cam, &h, &v, &bytes)) {
				aug->exd.h_pix = (gushort) (h);
				aug->exd.v_pix = (gushort) (v);
			} else
				return show_error(__func__,"Error retrieving image array size");
			
			/* Read the data */
			
			if (!sxc_get_imagearray (&a_cam, aug->vadr))
				return show_error (__func__, "Failed to read image data");
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
		
	/* Get the RA worm position closest to this exposure, in case we are
	 * doing PEC analysis.  Note this works best if we have short exposures.
	 */
	
	if (aug->autog.Worm) {
		G_print ("Requesting worm position...\n");
		WormPos = telescope_get_RA_worm_pos ();
	}
	
	/* Convert to greyscale and dark subtract if required */
	
	if (!(convert_to_grey (aug->device, aug->idp, aug->rgb, aug->vadr, 
						   aug->disp_16_1, aug->disp_8_3, 
						   aug->exd.h_pix, aug->exd.v_pix))) 
		return FALSE;
	
	/* Display the image data */
	
	if (!ui_show_augcanv_image ())
		return FALSE;
	
	if (!image_calc_hist_and_flux ())
	    return FALSE;
	
	/* Write the motion of the star centroid, if requested (this is calculated 
	 * in the image_calc_hist_and_flux routine).  Also draw the position on the 
	 * autoguider trace display, if required.
	 */
		
	if (aug->autog.Write)
		augcam_write_starpos (WormPos);
	
	if (aug->graph.draw)
		augcam_draw_starpos ();
	
	/* Set 'file saved' condition to FALSE */
	
	file_saved (aug, FALSE);
	
	/* If autosave is selected, schedule the image to be saved */
	
	if (aug->AutoSave)
		loop_save_image (AUG);
	
	return TRUE;	
}

static void augcam_set_image_size (gushort width, gushort height)
{
	/* Set the parameters for the selected image size */
	
	aug->exd.h_pix = width;
	aug->exd.v_pix = height;
	aug->exd.h_top_l = 0;  /* Adjust these values if you want autoguider image*/
	aug->exd.v_top_l = 0;  /*  located other than at top left corner on canvas*/
	aug->exd.h_bot_r = aug->exd.h_top_l + aug->exd.h_pix - 1;
	aug->exd.v_bot_r = aug->exd.v_top_l + aug->exd.v_pix - 1;
}

void augcam_set_camera_binning (gushort h_bin, gushort v_bin)
{
	/* Set the binning for the next exposure */
	
	aug->exd.h_bin = h_bin;
	aug->exd.v_bin = v_bin;
	
	augcam_set_image_size (aug->cam_cap.max_h / h_bin, 
						   aug->cam_cap.max_v / v_bin);
	
    ui_set_augcanv_crosshair (aug->exd.h_top_l + aug->exd.h_pix / 2.0,
							  aug->exd.v_top_l + aug->exd.v_pix / 2.0);
							  
	ui_set_augcanv_rect_full_area ();
}

gboolean augcam_set_vidval (enum Range range, gdouble value)
{
	/* Set the appropriate picture value.  This routine is called when the
	 * user moves one of the sliders on the autoguider image window.
	 */
	
	struct v4l2_control vid_ctrl;
		
	if (range == AIW_BACKGROUND) {
		aug->imdisp.black = (gushort) value;
		return TRUE;
	}	
	
	switch (aug->device) {
		#ifdef HAVE_UNICAP
		case UNICAP:
			/* Brightness/Contrast/Gamma sliders set insenstive for Unicap.
			 * Adjust these values via the 'Set Unicap properties...' menu item.
			 */
			break;
		#endif
        #ifdef HAVE_LIBV4L2
		case V4L:
			memset (&vid_ctrl, 0, sizeof (vid_ctrl));
			switch (range) {
				case AIW_BRIGHTNESS:
					vid_ctrl.id = V4L2_CID_BRIGHTNESS;
			        vid_ctrl.value = (gint) value;
					break;
				case AIW_CONTRAST:
					vid_ctrl.id = V4L2_CID_CONTRAST;
			        vid_ctrl.value = (gint) value;
					break;
				case AIW_GAMMA:
					vid_ctrl.id = V4L2_CID_GAMMA;
			        vid_ctrl.value = (gint) value;
					break;
				case AIW_GAIN:
					vid_ctrl.id = V4L2_CID_GAIN;
			        vid_ctrl.value = (gint) value;
					break;
				default:
					break;
			}
		    ioctl (aug->fd, VIDIOC_S_CTRL, &vid_ctrl);
			break;
        #endif
        #ifdef HAVE_SX
		case SX:
		case SX_GH:
			aug->imdisp.gamma = value;
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
	
	return TRUE;
}

static void initialise_image_window (enum CamDevice dev)
{
	/* Initialise various autoguider image window settings */
	
	switch (dev) {
		
		case UNICAP:
			ui_set_aug_window_controls (dev, FALSE);
			set_range_minmaxstep (AIW_BACKGROUND, 0, aug->imdisp.W, 1, 0);
			set_range_value (AIW_BACKGROUND, TRUE, 0);
			/* Adjust brightness, contrast, gamma and gain via 'Unicap
			 * properties' menu option, so set window sliders insensitive.
			 */
			set_range_value (AIW_BRIGHTNESS, FALSE, 0);
			set_range_value (AIW_CONTRAST, FALSE, 0);
			set_range_value (AIW_GAMMA, FALSE, 0);
			set_range_value (AIW_GAIN, FALSE, 0);
			break;
		case V4L:
			ui_set_aug_window_controls (dev, FALSE);
			set_range_minmaxstep (AIW_BACKGROUND, 0, aug->imdisp.W, 1, 0);
			set_range_value (AIW_BACKGROUND, TRUE, 0);
			/* Pass a negative value to set_range_value to indicate that no
			 * range value should be set - just sensitise/desensitise the 
			 * control as appropriate.  The range values have already been set 
			 * by this point when the V4L device was opened.
			 */
			set_range_value (AIW_BRIGHTNESS, TRUE, -1);
			set_range_value (AIW_CONTRAST, TRUE, -1);
			set_range_value (AIW_GAMMA, TRUE, -1);
			set_range_value (AIW_GAIN, TRUE, -1);
			break;
		case SX:
		case SX_GH:
			ui_set_aug_window_controls (dev, TRUE);
			if (aug->cam_cap.max_adu > 255)
				set_range_minmaxstep (AIW_BACKGROUND, 0, aug->imdisp.W, 255, 0);
			else
				set_range_minmaxstep (AIW_BACKGROUND, 0, aug->imdisp.W, 1, 0);
			set_range_value (AIW_BACKGROUND, TRUE, 0);
			set_range_minmaxstep (AIW_GAMMA, 0.0, 1.0, 0.05, 2);
			set_range_value (AIW_BRIGHTNESS, FALSE, 0);
			set_range_value (AIW_CONTRAST, FALSE, 0);
			set_range_value (AIW_GAMMA, TRUE, 1.0);
			set_range_value (AIW_GAIN, FALSE, 0);
			break;
		default:
			L_print ("{r}%s: Unknown autoguider camera type!\n", __func__);
	}
}

#ifdef HAVE_LIBV4L2
static void V4L_initialise_bcg_sliders (gchar *name, gint id, gint item)
{
	/* Initialise brightness, contrast and gamma sliders for V4L */
	
	struct v4l2_control vid_ctrl;
	struct v4l2_queryctrl vid_queryctrl;
	
    memset (&vid_ctrl, 0, sizeof (vid_ctrl));
    memset (&vid_queryctrl, 0, sizeof (vid_queryctrl));
    vid_ctrl.id = id;
    vid_queryctrl.id = id;
    if (v4l2ioctl (aug->fd, VIDIOC_G_CTRL, &vid_ctrl)) {
		v4l2ioctl (aug->fd, VIDIOC_QUERYCTRL, &vid_queryctrl);
		L_print ("%s: %d (min: %d, max: %d, step: %d)\n", 
				 name, vid_ctrl.value, vid_queryctrl.minimum, 
				 vid_queryctrl.maximum, vid_queryctrl.step);
        set_range_minmaxstep (item, vid_queryctrl.minimum, 
				              vid_queryctrl.maximum, vid_queryctrl.step, 0);
		set_range_value (item, TRUE, vid_ctrl.value);
	} else
	    L_print ("{o}Device does not support manual %s control\n", name);
}
#endif

gboolean augcam_write_starpos_time (void)
{
	/* Write the date and time to the star positions file */

	struct tm *dt;	
	gchar *date;
	
	dt = get_time (UseUTC);
	date = g_strdup_printf ("%4i-%02i-%02iT%02i:%02i:%02i", 
	                        1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
	                        dt->tm_hour, dt->tm_min, dt->tm_sec);	
	
	if ((fprintf (fps, "\n%f\t%s\n", loop_elapsed_since_first_iteration () / 
				                                            1000.0, date) == 0))
		goto error;	
	
	if ((fprintf (fps, "     sec.\tWormPos\tN/S (+/-)\tE/W (-/+)\t"
		                                        "Guide\tCCD exposure\n") == 0))
		goto error;
	
	g_free (date);
	return TRUE;
	
error:
	
	g_free (date);
	return show_error (__func__, "Error writing to star positions file");	
}

gboolean augcam_write_starpos (gushort WormPos)
{
	/* Write RA worm counter and star positions to a file, also adding 
	 * information to indicate whether a CCD exposure is in progress.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
		
	struct tm *dt;
	gchar *date = NULL;		
	
	if (ccd->Expose) {
		dt = get_time (UseUTC);
		date = g_strdup_printf ("%4i-%02i-%02iT%02i:%02i:%02i", 
	                            1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
	                            dt->tm_hour, dt->tm_min, dt->tm_sec);			
	}		
	
	if ((fprintf (fps, "%9.3f\t%7d\t%9.3f\t%9.3f\t%5s\t%s\n",
		loop_elapsed_since_first_iteration () / 1000.0, WormPos, 
			aug->rect.shift_ns, aug->rect.shift_ew, 
	        aug->autog.Guide ? "*":"", ccd->Expose ? date : "")) == 0) {
		if (ccd->Expose)
			g_free (date);
		return show_error (__func__, "Error writing to star positions file");
	}
	
	if (ccd->Expose)
		g_free (date);
		
	return TRUE;
}

static void augcam_draw_starpos (void)
{
	/* Draws the N/S and E/W shift of the star on the autoguider trace display.
	 */
	
	#ifdef HAVE_LIBGRACE_NP
	static guint start_time = 0, prev_loop = 0;
	guint now;
	gfloat minutes;
	const gfloat GRACE_MINS = 8.0; /* Width of scrolling plot in minutes */
	
	if (aug->graph.reset) {        /* Initialise the plots */
		aug->graph.reset = FALSE;
		
		Grace_SetXAxis (0, 0.0, GRACE_MINS);
		Grace_SetYAxis (0, -(aug->canv.r.vbr - aug->canv.r.vtl + 1) / 2.0,
			                (aug->canv.r.vbr - aug->canv.r.vtl + 1) / 2.0);
		Grace_SetXAxis (1, 0.0, GRACE_MINS);
		Grace_SetYAxis (1, -(aug->canv.r.hbr - aug->canv.r.htl + 1) / 2.0,
						    (aug->canv.r.hbr - aug->canv.r.htl + 1) / 2.0);
		
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
			
			Grace_PlotPoints (0, 0, minutes, aug->rect.shift_ns);
			Grace_PlotPoints (1, 0, minutes, aug->rect.shift_ew);
			if (!Grace_Update ()) {
				aug->graph.draw = FALSE;
				aug->graph.reset = TRUE;
			}
	    }
	}
	#endif
}

void augcam_read_dark_frame (void)
{
	/* Read the dark frame from the file */
	
	gushort size_h, size_v;
	
	fseek (fpd, 0, SEEK_SET);
	fread (&size_h, sizeof (gushort), 1, fpd);
	fread (&size_v, sizeof (gushort), 1, fpd);
	if (size_h != aug->exd.h_pix || size_v != aug->exd.v_pix) {
		msg ("Warning - Saved dark frame size is different from currently "
			 "selected image resolution - \n Flushing saved dark frame.");
		augcam_flush_dark_frame ();
		return;
	}
	fread (&ndark, sizeof (gushort), 1, fpd);
	if (fread (aug->dark.dvadr, sizeof (gushort), aug->exd.h_pix *
		    aug->exd.v_pix, fpd) != aug->exd.h_pix * aug->exd.v_pix)
		msg ("Warning - Couldn't read dark frame from file");
}

void augcam_flush_dark_frame (void)
{
	/* Flush the dark frame */
	
	fseek (fpd, 0, SEEK_SET);
	fwrite (&aug->exd.h_pix, sizeof (gushort), 1, fpd);
	fwrite (&aug->exd.v_pix, sizeof (gushort), 1, fpd);
	ndark = 0;
	fwrite (&ndark, sizeof (gushort), 1, fpd);
	memset (aug->dark.dvadr, 0, aug->exd.h_pix * aug->exd.v_pix * 
	                                                          sizeof (gushort));
	if (fwrite (aug->dark.dvadr, sizeof (gushort), aug->exd.h_pix *
	                    aug->exd.v_pix, fpd) != aug->exd.h_pix * aug->exd.v_pix)
		msg ("Warning - Couldn't flush dark frame");
}

static void augcam_write_dark_frame (void)
{
	/* Write the dark frame to the file */
	
	fseek (fpd, 0, SEEK_SET);
	fwrite (&aug->exd.h_pix, sizeof (gushort), 1, fpd);
	fwrite (&aug->exd.v_pix, sizeof (gushort), 1, fpd);
	fwrite (&ndark, sizeof (gushort), 1, fpd);
	if (fwrite (aug->dark.dvadr, sizeof (gushort), aug->exd.h_pix *
	                    aug->exd.v_pix, fpd) != aug->exd.h_pix * aug->exd.v_pix)
		msg ("Warning - Couldn't write dark frame to file");
}

#ifdef HAVE_UNICAP
static gboolean OpenUnicapDevice (void)
{
	/* Open the selected Unicap device (the device and format structures are
	 * set in the main interface routines before calling augcam_open).
	 */
	
	L_print ("Device: %s\n", aug->u_device.identifier);
	L_print ("Format: %c%c%c%c, %dx%d\n", aug->u_format_spec.fourcc, 
			                              aug->u_format_spec.fourcc >> 8, 
					                      aug->u_format_spec.fourcc >> 16, 
			                              aug->u_format_spec.fourcc >> 24,
	                                      aug->u_format_spec.size.width,
	                                      aug->u_format_spec.size.height);
	
	/* Get the device information specific to the requested device */
	
	if (!SUCCESS(unicap_enumerate_devices (&aug->u_device, &aug->u_device, 0))){
		L_print ("{r}%s: Failed to get info for Unicap device %s\n", 
				                            __func__, aug->u_device.identifier);
	    return FALSE;
	}
	
	/* Acquire a handle to the device */

	if (!SUCCESS (unicap_open (&aug->u_handle, &aug->u_device)))
		return show_error (__func__, "Failed to acquire handle for Unicap "
						                                              "device");
	
	/* Get the full format information specific to the requested format and set
	 * the format.
	 */

	if (!SUCCESS (unicap_enumerate_formats (
						aug->u_handle, &aug->u_format_spec, &aug->u_format, 0)))
		return show_error (__func__, "Failed to get video format information");
	
	/* The DFG/1394 insists on returning the 768x576 size even if you ask for
	 * something else, so override the size setting here.  Assume this is OK
	 * for other Unicap devices.
	 */
	
	aug->u_format.size.width = aug->u_format_spec.size.width;
	aug->u_format.size.height = aug->u_format_spec.size.height;
	
	if (!SUCCESS (unicap_set_format (aug->u_handle, &aug->u_format)))
		return show_error (__func__, "Failed to set video format");
	
	/* Set the image size */
	
	aug->cam_cap.max_h = aug->u_format.size.width;
	aug->cam_cap.max_v = aug->u_format.size.height;
	augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);
	
	/* Create window for ugtk widget, and add widget to window.
	 * (Unicap crashes if the widget doesn't have a window to run in, although
	 * we can hide the window once it's been created).
	 */
	
	if (!open_liveview_window ())
		return show_error (__func__, "Failed to start image acquisition for "
						                                       "Unicap device");
	
	return TRUE;	
}
#endif

#ifdef HAVE_UNICAP
static gboolean CloseUnicapDevice (void)
{
	/* Close Unicap device */
	
	if (aug->ugtk_display)
		unicapgtk_video_display_stop (
								   UNICAPGTK_VIDEO_DISPLAY (aug->ugtk_display));
	aug->ugtk_display = NULL;
	if (aug->u_handle)
		if (!SUCCESS (unicap_close (aug->u_handle)))
			return FALSE;
	aug->u_handle = NULL;
	
	close_liveview_window ();
	return TRUE;
}
#endif

#ifdef HAVE_UNICAP
void augcam_unicap_new_frame_cb (unicap_event_t event, unicap_handle_t handle, 
						         unicap_data_buffer_t *buffer, gpointer *data)
{
	/* Make a copy of the unicap image buffer data for each new frame if 
	 * the autoguider needs it, and buffer the data if recording.  Note that
	 * the unicap buffer data are assumed to be in one byte per pixel greyscale
	 * format (so not rgb!).
	 */
	
	/* We assume this buffer size is not bigger than h*v*sizeof(guchar)! */

	if (aug->Open)  
		memcpy (aug->rgb, buffer->data, buffer->buffer_size);
	
	/* Buffer frame if recording */
	
	if (aug->Record )
		video_buffer_frame (buffer->data, &buffer->fill_time, 
							loop_elapsed_since_first_iteration());
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean OpenV4LDevice (gchar *device)
{
	/* Open and initialise a V4L device */

	struct v4l2_capability vid_cap;
    struct v4l2_format vid_fmt;
    struct v4l2_buffer buffer;
    struct v4l2_requestbuffers reqbuf;
    gushort i;
	gint buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    /* Open device */
        
	if ((aug->fd = v4l2_open (device, O_RDWR | O_NONBLOCK, 0)) < 0)
		return show_error (__func__, "Error opening V4L autoguider "
		                                                       "camera device");
	
    /* Get camera basic data */
	
    memset (&vid_cap, 0, sizeof (vid_cap));
	if (!v4l2ioctl (aug->fd, VIDIOC_QUERYCAP, &vid_cap))
		return show_error (__func__, "VIDIOC_QUERYCAP -- Could not get camera "
	                                                            "capabilities");
	
	L_print ("V4L device information:\n");
	L_print ("Driver = %s\n", vid_cap.driver);
	L_print ("Card = %s\n", vid_cap.card);
	L_print ("Bus info = %s\n", vid_cap.bus_info);
	L_print ("Version = %d\n", vid_cap.version);
	
	/* Set video format */
		                                                       
	memset (&vid_fmt, 0, sizeof (vid_fmt));
    vid_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vid_fmt.fmt.pix.width       = 640;
    vid_fmt.fmt.pix.height      = 480;
    vid_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    vid_fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	if (!v4l2ioctl (aug->fd, VIDIOC_S_FMT, &vid_fmt))
		return show_error (__func__, "VIDIOC_S_FMT -- Could not set camera "
	                                                            "video format");
	
	if (!v4l2ioctl (aug->fd, VIDIOC_G_FMT, &vid_fmt)) /* Check what we got */
		return show_error (__func__, "VIDIOC_G_FMT -- Could not get camera "
	                                                            "video format");
	L_print ("Picture width: %d\n", vid_fmt.fmt.pix.width);
	L_print ("Picture height: %d\n", vid_fmt.fmt.pix.height);
	
	aug->cam_cap.max_h = vid_fmt.fmt.pix.width;
	aug->cam_cap.max_v = vid_fmt.fmt.pix.height;
	augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);
	aug->vid_buf.pixfmt = (aug->imdisp.greyscale == MONO) ? V4L2_PIX_FMT_GREY :
	                                                vid_fmt.fmt.pix.pixelformat;

	/* Set the brightness, contrast, gamma and gain sliders on the autoguider
	 * image window.
	 */
	
	V4L_initialise_bcg_sliders ("brightness", V4L2_CID_BRIGHTNESS, AIW_BRIGHTNESS);
	V4L_initialise_bcg_sliders ("contrast", V4L2_CID_CONTRAST, AIW_CONTRAST);
	V4L_initialise_bcg_sliders ("gamma", V4L2_CID_GAMMA, AIW_GAMMA);
	V4L_initialise_bcg_sliders ("gain", V4L2_CID_GAIN, AIW_GAIN);
	
	/* Initialise memory buffers */
	
	memset (&reqbuf, 0, sizeof (reqbuf));
    reqbuf.count = 2;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
	if (!v4l2ioctl (aug->fd, VIDIOC_REQBUFS, &reqbuf))
		return show_error (__func__, "VIDIOC_REQBUFS -- Buffer memory request "
		                                                              "failed");
		                                                              
    aug->vid_buf.buffers = calloc (reqbuf.count, sizeof(*aug->vid_buf.buffers));
    for (aug->vid_buf.num = 0; aug->vid_buf.num < reqbuf.count; 
                                                           aug->vid_buf.num++) {
		memset (&buffer, 0, sizeof (buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = aug->vid_buf.num;

		if (!v4l2ioctl (aug->fd, VIDIOC_QUERYBUF, &buffer))
			return show_error (__func__, "VIDIOC_QUERYBUF -- Unable to query "
			                                              "buffer information");

        aug->vid_buf.buffers[aug->vid_buf.num].length = buffer.length;
        aug->vid_buf.buffers[aug->vid_buf.num].start = 
                                             v4l2_mmap (NULL, buffer.length,
                                             PROT_READ | PROT_WRITE, MAP_SHARED,
                                             aug->fd, buffer.m.offset);

        if (MAP_FAILED == aug->vid_buf.buffers[aug->vid_buf.num].start)
			return show_error (__func__, "Unable to map buffer memory");
    }
    
	/* Queue buffers */
    
	for (i = 0; i < aug->vid_buf.num; i++) {
		memset (&buffer, 0, sizeof (buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
		if (!v4l2ioctl (aug->fd, VIDIOC_QBUF, &buffer))
			return show_error (__func__, "VIDIOC_QBUF -- Unable to queue "
			                                                   "memory buffer");
	}
	
	/* Start streaming */
	
	if (!v4l2ioctl (aug->fd, VIDIOC_STREAMON, &buf_type))
		return show_error (__func__, "VIDIOC_STREAMON -- Unable to start "
			                                                 "video streaming");

	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean CloseV4LDevice (void)
{
	/* Close a V4L device */
	
	gushort i;
	gint buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	 
	/* Stop streeaming */
	
	if (!v4l2ioctl (aug->fd, VIDIOC_STREAMOFF, &buf_type))
		return show_error (__func__, "VIDIOC_STREAMOFF -- Unable to stop "
			                                                 "video streaming");
		
	/* Unmap memory buffers */
	
    for (i = 0; i < aug->vid_buf.num; i++)
		v4l2_munmap (aug->vid_buf.buffers[i].start, 
		             aug->vid_buf.buffers[i].length);
		             
	/* Close device */
                
     v4l2_close (aug->fd);

	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean GrabV4LBuffer (void)
{
	/* Grab a frame from the V4L device */
    
	struct v4l2_buffer buffer;
    struct timeval tv;
    fd_set fds;
    gint r, fd = -1;
	gint buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Check the device is available */
	
	do {
		FD_ZERO (&fds);
        FD_SET (aug->fd, &fds);

        /* Timeout */
        tv.tv_sec = 0;
        tv.tv_usec = 1000;

        r = select(fd + 1, &fds, NULL, NULL, &tv);
    } while ((r == -1 && (errno = EINTR)));
        if (r == -1) 
			return show_error (__func__, "Error polling V4L device");
        
    /* De-queue a buffer */
        
    memset (&buffer, 0, sizeof (buffer));
    buffer.type = buf_type;
    buffer.memory = V4L2_MEMORY_MMAP;
	if (!v4l2ioctl (aug->fd, VIDIOC_DQBUF, &buffer))
		return show_error(__func__, "VIDIOC_DQBUF -- Error de-queueing buffer");
		
	aug->idp = aug->vid_buf.buffers[buffer.index].start;  /* Better to take a */
	                                                      /*  copy of buffer? */
	/* Re-queue */
		
	if (!v4l2ioctl (aug->fd, VIDIOC_QBUF, &buffer))
		return show_error(__func__, "VIDIOC_QBUF -- Error queueing buffer");
		
	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean v4l2ioctl (gint fh, gint request, void *arg)
{
	/* Wrapper for v4l ioctl calls */
	
	gint r;

	do {
		r = v4l2_ioctl (fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	return (r == -1 ? FALSE : TRUE);
}
#endif

#ifdef HAVE_SX
gboolean augcam_get_sx_cameras (int *num)
{
	/* Return a list of detected sx cameras */
	
	int i;
	const char *serial[MAX_CAMERAS], *desc[MAX_CAMERAS];
	
	sx_error_func (ccdcam_sx_error_func);
	
	*num = MAX_CAMERAS;
	if (!sxc_get_cameras (&a_cam, serial, desc, num))
		return show_error (__func__, "Error searching for cameras!");
	if (*num > 0) {
		for (i = 0; i < *num; i++)
			L_print ("Found camera %d: %s, %s\n", i, desc[i], serial[i]);
	} else
	    L_print ("{o}Didn't find any cameras - are permissions "
	             "set correctly?\n");
	    
	  return TRUE;	
}
#endif

#ifdef HAVE_SX
static gboolean OpenSXDevice (void)
{
	/* Open and initialise a Starlight Xpress camera */
	
	gint num;
	const char *serial[MAX_CAMERAS], *desc[MAX_CAMERAS];
	
	strcpy (aug->cam_cap.camera_manf, "SX");
	
	sx_error_func (ccdcam_sx_error_func);
	
	num = MAX_CAMERAS;
	if (!sxc_get_cameras (&a_cam, serial, desc, &num))
		return show_error (__func__, "Error searching for cameras!");
	if (num == 0) {
	    L_print ("{o}Didn't find any cameras - are permissions "
	             "set correctly?\n");
		return FALSE;
	} else if (num == 1) {
		if (!sxc_connect (&a_cam, TRUE, serial[0]))
			return show_error (__func__, "Unable to connect to camera!");
		aug->devnum = 0;
	} else if (num > 1 && aug->devnum == -1) {
		L_print ("{r}Found more than one camera!  Please select the one you "
				 "want from the 'Cameras' menu\n");
	    return FALSE;
    } else if (num > 1 && aug->devnum < num) {
		if (!sxc_connect (&a_cam, TRUE, serial[aug->devnum]))
			return show_error (__func__, "Unable to connect to camera!");
	} else
		return show_error (__func__, "Error selecting camera!");
	
	strncpy (aug->cam_cap.camera_desc, desc[aug->devnum], 256);
	
	/* Get the selected camera's capabilities */
	
	if (!sxc_get_cap (&a_cam, &aug->cam_cap)) {
		CloseSXDevice ();
		return show_error (__func__, "Unable to get camera capabilities!");
	}
	
	/* Assign functions for setting/getting camera state */
	
	aug->set_state = sx_set_state;
	aug->get_state = sx_get_state;
	
	/* Set initial state based on stored configuration data */
	
	set_camera_state (aug);
	
	/* Set initial image size */
	
	augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);
		
	return TRUE;	
}
#endif

#ifdef HAVE_SX
static gboolean CloseSXDevice (void)
{
	/* Close a Starlight Xpress camera */
	
	sxc_cancel_exposure (&a_cam);
	if (!sxc_connect (&a_cam, FALSE, NULL))
		return show_error (__func__, "Unable to close Starlight Xpress camera");

	return TRUE;
}
#endif

#ifdef HAVE_SX
static gboolean OpenSXGuideHead (void)
{
	/* Initialise the guide head attached to the main Starlight Xpress camera */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	if (ccd->device == SX && ccd->Open) {
		
		/* Get a copy of the udevice structure for the CCD camera and set the
		 * camera ID to 1 to indicate that any calls made to the device are for
		 * the guide head rather than the main camera.
		 */
		
		a_cam.udev = sx_get_ccdcam_udev ();
		a_cam.udev.id = 1;
		
		/* Get the selected camera's capabilities */
		
		if (!sxc_get_cap (&a_cam, &aug->cam_cap)) {
			CloseSXGuideHead ();
			return show_error (__func__, "Unable to get camera capabilities!");
		}
		strcpy (aug->cam_cap.camera_desc, "GuideHead");
		
		/* Set initial image size */
		
		augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);
	} else
		return show_error (__func__, "Can't find main Starlight Xpress camera");
		
	return TRUE;
}
#endif

#ifdef HAVE_SX
static gboolean CloseSXGuideHead (void)
{
	/* Tidy up after using the Starlight Xpress guide head */
	
	sxc_cancel_exposure (&a_cam);
	if (!sxc_connect (&a_cam, FALSE, NULL))
		return show_error (__func__, "Unable to close Starlight Xpress camera");
		
	return TRUE;
}
#endif

#ifdef HAVE_SX
struct sx_cam *augcam_get_sx_cam_struct (void)
{
	return &a_cam;
}
#endif

struct cam_img *get_aug_image_struct (void)
{
	return aug;
}


/******************************************************************************/
/*                         VIDEO CONVERSION FUNCTIONS                         */
/******************************************************************************/

static gboolean convert_to_grey (enum CamDevice device, guchar *image, 
								 guchar *rgb, gushort *grey, gushort *disp_16_1,
								 guchar *disp_8_3, gint x, gint y)
{
	/* Convert raw image data and return as follows:
	 * (a) RGB format having one byte per pixel per colour channel, but with 
	 * each byte the same value so that the image is actually greyscale.  This 
	 * may be dark-subtracted or otherwise manipulated for display on the image 
	 * canvas and is returned as 'disp_8_3'.
	 * (b) A version of the display data in full 16-bit precision as greyscale
	 * 'disp_16_1'.  This is for performing centroiding calculations for star 
	 * positions and for showing histogram and flux plots.
	 * (c) The raw (unmanipulated) data as 16-bit precision greyscale 'grey'.
	 *
	 * On entering this routine:-
	 * 
	 * 	    For V4L devices:
	 * 
	 *          'image' points to the raw data
	 * 			For Unicap data only,
	 *          'rgb'   contains the raw data as one byte per pixel greyscale 
	 *                  format and 'image' points to it;
	 * 	        For CCD cameras used as autoguider cameras,
	 *          'grey'  already contains the raw data as a maximum of two bytes 
	 *                  per pixel (but may be fewer) greyscale data and so is 
	 *                  returned unchanged
	 * 
	 *      On leaving this routine:
	 * 
	 *          'rgb'   contains the raw data as:
	 *                  -see above for Unicap
	 *                  -RGB format for data that was YUV420 format initially
	 *          'grey'  contains the raw data converted to a maximum of two 
	 *                  bytes per pixel in greyscale format
	 *          'disp_8_3' contains the (possibly) dark-subtracted or background
	 *                  adjusted data in three bytes per pixel greyscale format
	 *                  (i.e. actually in rgb format, but with the r, g, and b 
	 *                  bytes for each pixel set to the same value)
	 *          'disp_16_1' contains the (possibly) dark-subtracted or 
	 *                  background adjusted data in a maximum of 16-bit 
	 *                  precision per pixel
	 */
	
    //const gint bytes = z;        /* (z*8) >> 3; */
	gushort row, col;
	gushort xo1, xo2, yo1, yo2;
	guint size, count, reject;
	gint bytes = 1;
	gint val = 0;
    gint numpix = x * y;
    gint i, j, k, y00, y01, y10, y11, u, v;
    gdouble gamma_scale, diff, rms;
    guchar *pY = image;
    guchar *pU = pY + numpix;
    guchar *pV = pU + numpix / 4;
    guchar *image2 = rgb;
	guchar *temp = NULL, *start;
	
	if (device == UNICAP || device == V4L) {
		
		if (!image)                            /* Return if no image          */
			return TRUE;
			
		switch (aug->vid_buf.pixfmt) {         /* Unicap: assumes             */
			case V4L2_PIX_FMT_GREY:            /*  input data is one byte per */
				bytes = 1;                     /*  pixel greyscale.           */
				start = image;
				break;
				
			case V4L2_PIX_FMT_YUV420: /* Webcam: YUV420 format must be        */
				bytes = 3;            /*  converted to RGB                    */
				for (j = 0; j <= y - 2; j += 2) {
					for (i = 0; i <= x - 2; i += 2) {
						y00 = *pY;
						y01 = *(pY + 1);
						y10 = *(pY + x);
						y11 = *(pY + x + 1);
						u = (*pU++) - 128;
						v = (*pV++) - 128;

						move_420_block (y00,y01,y10,y11,u, v, x, 
										image2, bytes * 8);

						pY += 2;
						image2 += 2 * bytes;
					}
					pY += x;
					image2 += x * bytes;
				}
				start = rgb;	
				break;
				
			case V4L2_PIX_FMT_RGB24:      /* Already RGB format               */
				bytes = 3;
				start = image;
				break;
				
			default:
				return show_error (__func__, "Unsupported video palette");
				break;	
		}
		temp = start;
	}	
	
	/* Read stored dark frame from file if capturing and averaging */
	
	if (aug->dark.Capture)
		augcam_read_dark_frame ();    /* Fills aug->dvadr */
	
    /* Initialise some values for calculating pixel statistics and get the
	 * coordinates of the selection rectangle within the displayed image.
	 */
	
	aug->pic.min[GREY].val = aug->imdisp.W;
	aug->pic.max[GREY].val = aug->imdisp.B;
	aug->rect.min[GREY].val = aug->imdisp.W;
	aug->rect.max[GREY].val = aug->imdisp.B;
	aug->img.min[GREY].val = aug->imdisp.W;
	aug->img.max[GREY].val = aug->imdisp.B;
	memset (aug->rect.mode[GREY].hist, 0, (aug->imdisp.W + 1) * sizeof (guint));
	aug->rect.mode[GREY].peakcount = 0;
	
	is_in_image (aug, &xo1, &yo1, &xo2, &yo2);

	/* Loop over entire image and convert to greyscale (if not already) */
	
	gamma_scale = pow (aug->imdisp.W, aug->imdisp.gamma) + 0.01;
	for (i = 0; i < numpix; i++) {
		
		if (device == UNICAP || device == V4L) {
			switch (aug->vid_buf.pixfmt) {          /* Convert to 1 bpp grey  */
				case V4L2_PIX_FMT_GREY:             /* Already 1 bpp greyscale*/
					val = temp[0];
					break;
				case V4L2_PIX_FMT_YUV420:  /* Is now RGB - see above     */
				case V4L2_PIX_FMT_RGB24:   /* Was RGB already            */
					switch (aug->imdisp.greyscale) {
						case LUMIN:  /* image is bgr at this point */
							val = (gushort) (0.11 * (gfloat) temp[0] +
													   0.59 * (gfloat) temp[1] +
													   0.30 * (gfloat) temp[2]);
							break;
						case DESAT:
							val = (MAX (temp[0], MAX (temp[1], temp[2])) +
								   MIN (temp[0], MAX (temp[1], temp[2]))) / 2;							
							break;
						case MAXIM:
							val = MAX (temp[0], MAX (temp[1], temp[2]));
							break;
						case MONO:
							break;
					}
					break;
			}
			*grey++ = val;                  /* All formats now 1bpp greyscale */
			temp += bytes;
		} else if (device == SX || device == SX_GH) {
			val = grey[i];
		}
		
		/* If this is a dark frame, average with the previous stored one(s) */
		
		if (aug->dark.Capture)
			aug->dark.dvadr[i] = (aug->dark.dvadr[i] * ndark + val)/(ndark + 1);		
	
		/* Subtract dark frame, if required */		

		if (aug->dark.Subtract)
			val = MAX (val - aug->dark.dvadr[i], 0);
	
		/* Subtract the background cut-off level */
		
		val = MAX (val - aug->imdisp.black, 0);
		
		/* Get pixel statistics in the (possibly dark-subtracted and/or 
		 * background-adjusted) image.
		 */
		
		/* aug->pic is for the entire displayed image; these values are       */
		/* written to the FITS header if an autoguider image is saved.        */
		aug->pic.min[GREY].val = MIN (aug->pic.min[GREY].val, val);
		aug->pic.max[GREY].val = MAX (aug->pic.max[GREY].val, val);
		
		/* aug->rect is for just the displayed image within the selection     */
		/* rectangle; these values are used for determining the guide star    */
		/* location.  aug->img is for the raw data (also just within the      */
		/* selection rectangle in this case); these values are reported on    */
		/* the Image window status bar.                                       */
		col = i%aug->exd.h_pix;
		row = i / aug->exd.h_pix;
		if (yo1 <= row && row <= yo2 && xo1 <= col && col <= xo2) {
			aug->rect.min[GREY].val = MIN (aug->rect.min[GREY].val, val);
			if (val > aug->rect.max[GREY].val) {
				aug->rect.max[GREY].val = val;
				aug->rect.max[GREY].h = col;
				aug->rect.max[GREY].v = row;
			}
			aug->img.min[GREY].val = MIN (aug->img.min[GREY].val, 
										 aug->vadr[row * aug->exd.h_pix + col]);
			aug->img.max[GREY].val = MAX (aug->img.max[GREY].val, 
										 aug->vadr[row * aug->exd.h_pix + col]);
			if (++aug->rect.mode[GREY].hist[val] > 
											   aug->rect.mode[GREY].peakcount) {
				aug->rect.mode[GREY].peakcount = aug->rect.mode[GREY].hist[val];
                aug->rect.mode[GREY].peakbin = val;
			}
		}
		
		/* Set the values for the displayed image.  Note that we expand up 
		 * to 3 bytes per pixel RGB format at this point for display on image 
		 * canvas.  Gamma-adjust the SX data for image display.
		 */
		 
		if (device == UNICAP || device == V4L) {       /* Assume 8-bit data  */
			for (k = 0; k < 3; k++)
				*disp_8_3++ = val;
		} else if (device == SX || device == SX_GH) {  /* Variable bit depth */
			if (aug->imdisp.gamma >= 0.995) { /* Gamma value essentially == 1 */
				for (k = 0; k < 3; k++)
					*disp_8_3++ = val >> (aug->cam_cap.bitspp - 8);
			} else {
				for (k = 0; k < 3; k++)
					*disp_8_3++ = (guchar) 255 * 
								   pow (val, aug->imdisp.gamma) / gamma_scale;
			}
		}
		disp_16_1[i] = val;                            /* For centroiding    */
	}
	
	/* Estimate the sky background as the median value in the selected area.  
	 * This will be a slight over-estimate but is more stable than the histogram
	 * peak bin.
	 */
	
	size = (yo2 - yo1 + 1) * (xo2 - xo1 + 1);
	for (i = 0, count = 0; i < (aug->imdisp.W + 1); i++) {
		count += aug->rect.mode[GREY].hist[i];
		if (count >= size / 2)
			break;
	}
	aug->rect.stdev[GREY].median = i;
	
	/* Estimate the standard deviation of the background in the selected area.
	 * Attempt to reject stars as far as possible by ignoring values that
	 * deviate from the median by more than 50%.
	 */
	
	rms = 0.0;
	reject = 0;
	for (i = yo1; i <= yo2; i++) {
		for (j = xo1; j <= xo2; j++) {	
			val = aug->disp_16_1[aug->exd.h_pix * i + j];
			if (val > 0.5 * aug->rect.stdev[GREY].median && 
				val < 1.5 * aug->rect.stdev[GREY].median) {
				diff = (gdouble) val - aug->rect.stdev[GREY].median;
				rms += diff * diff;
			} else
				reject++;
		}
	}
	aug->rect.stdev[GREY].val = sqrt (rms / (size - reject));
	
	/* Write dark frame to file if capturing and averaging */
	
	if (aug->dark.Capture) {
		ndark++;
		L_print ("Captured and averaged %d dark exposure%s\n", ndark, 
		                                                 ndark != 1 ? "s" : "");
		if (ndark == aug->dark.num)
			aug->dark.Capture = FALSE;
		augcam_write_dark_frame ();
	}

	return TRUE;
}

static inline void move_420_block (gint yTL, gint yTR, gint yBL, gint yBR,
                                   gint u, gint v, gint rowPixels, guchar *rgb,
                                   gint bits)
{
	/* This is a commonly available routine - no idea of the original source
	 * or author.
	 */
    const gint rvScale = 91881;
    const gint guScale = -22553;
    const gint gvScale = -46801;
    const gint buScale = 116129;
    const gint yScale = 65536;
    gint r, g, b;

    g = guScale * u + gvScale * v;
    if (1) {
        r = buScale * u;
        b = rvScale * v;
    } else {
        r = rvScale * v;
        b = buScale * u;
    }

    yTL *= yScale;
    yTR *= yScale;
    yBL *= yScale;
    yBR *= yScale;

    /* Write out top two pixels */
	
    rgb[0] = LIMIT (b + yTL);
    rgb[1] = LIMIT (g + yTL);
    rgb[2] = LIMIT (r + yTL);

    rgb[3] = LIMIT (b + yTR);
    rgb[4] = LIMIT (g + yTR);
    rgb[5] = LIMIT (r + yTR);

    /* Skip down to next line to write out bottom two pixels */
	
    rgb += 3 * rowPixels;
	
    rgb[0] = LIMIT (b + yBL);
    rgb[1] = LIMIT (g + yBL);
    rgb[2] = LIMIT (r + yBL);

    rgb[3] = LIMIT (b + yBR);
    rgb[4] = LIMIT (g + yBR);
    rgb[5] = LIMIT (r + yBR);
}
