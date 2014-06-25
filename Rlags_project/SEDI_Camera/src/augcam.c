/******************************************************************************/
/*                       AUTOGUIDER CAMERA ROUTINES                           */
/*                                                                            */
/* All the routines for interfacing with the autoguider camera are contained  */
/* in this module.  Possible cameras are:                                     */
/*                                                                            */
/*   A Starlight Xpress CCD camera                                            */
/*   A V4L compatible webcam or frame grabber via Unicap or GoQat's           */
/*                                                      native V4L support    */
/*   An Imaging Source USB or firewire Astronomy Camera via Unicap            */
/*   A camera connected to an Imaging Source DFG/1394 video to firewire       */
/*                                                      converter via Unicap  */
/*                                                                            */
/* V4L support substantially re-written (2014) following examples and         */
/* suggestions by Vincent Hourdin.                                            */
/* The implementation of the fifo buffer for V4L frame grabbing is a direct   */
/* transcription of code by Vincent Hourdin.                                  */
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
#ifdef HAVE_SX_CAM
#include "sx.h"
#endif

#define AUG_BLACK         0  /* Black level for autoguider data        */
#define AUG_WHITE_1     255  /* White level for 1-byte autoguider data */
#define AUG_WHITE_2   65535  /* White level for 2-byte autoguider data */

#define FOURCC(a,b,c,d) (guint)((((guint)d)<<24)+(((guint)c)<<16)+(((guint)b)<<8)+a)
/* The following 4CC's aren't provided by V4L2 */
#define FOURCC_Y8   FOURCC ('Y', '8', ' ', ' ')
#define FOURCC_Y800 FOURCC ('Y', '8', '0', '0')
//#define FOURCC_YU12 FOURCC ('Y', 'U', '1', '2') /* Same as YUV420 */
//#define FOURCC_YV12 FOURCC ('Y', 'V', '1', '2') /* Same as YVU420 */
#define FOURCC_YUY2 FOURCC ('Y', 'U', 'Y', '2')

enum {VIDBUF_GREY1, VIDBUF_GREY2, VIDBUF_COL3};

#ifdef HAVE_SX_CAM
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
gboolean augcam_process_image (void);
static void augcam_set_image_size (gushort width, gushort height);
void augcam_set_camera_binning (gushort h_bin, gushort v_bin);
gboolean augcam_set_vidval (enum Range range, gdouble value);
static void initialise_image_window (enum HWDevice dev);
#ifdef HAVE_LIBV4L2
static void V4L_initialise_bcg_sliders (gchar *name, gint id, gint item);
#endif
gboolean augcam_write_starpos_time (void);
gboolean augcam_write_starpos (void);
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
gboolean augcam_grab_v4l_buffer (void);
static gboolean V4L_list_image_formats (struct v4l2_format *vid_fmt);
static gboolean V4L_list_video_settings (guint32 pixelformat);
static void V4L_list_frame_sizes (guint32 pixelformat);
static void V4L_list_frame_rates (guint32 pixelformat, gint width, gint height);
static void V4L_list_current_settings (struct v4l2_format *vid_fmt);
static gint v4l2ioctl (gint fh, gint request, void *arg);
#endif  /* V4L2 */
#ifdef HAVE_SX_CAM
gboolean augcam_get_sx_cameras (void);
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

static void initialise_image_decoding (void);
static gboolean convert_to_grey (void);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void augcam_init (void)
{
	static gboolean FirstPass = TRUE;
	
	/* Initialise the autoguider camera data */
	
	aug = &aug_cam_obj;

	#ifdef HAVE_UNICAP
	aug->ucp_handle = NULL;
	aug->ugtk_display = NULL;
	aug->ugtk_window = NULL;
	#endif
	memset (&aug->vid_dat, 0, sizeof (aug->vid_dat));
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
	aug->dark.dk161 = NULL;
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
	aug->r161 = NULL;	
	aug->id = AUG;
	aug->devnum = -1;
	aug->fd = 0;
	aug->ff161 = NULL;
	aug->r083 = NULL;
	aug->disp083 = NULL;
	aug->Open = FALSE;
	aug->Expose = FALSE;
	aug->FileSaved = TRUE;
	aug->Error = FALSE;
	
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
		aug->AutoSave = FALSE;
		aug->SavePeriodic = FALSE;
		
		FirstPass = FALSE;
	}
}

gboolean augcam_open (void)
{
	/* Open the autoguider camera */
	
	gchar *err;
	gchar *FilePath = NULL;
	#ifdef HAVE_LIBV4L2
	gchar V4Ldev[12];
	#endif
	
	switch (aug->device) {
		#ifdef HAVE_UNICAP
		case UNICAP:
			L_print ("{b}****---->>>> Opening Unicap camera\n");
			if (!OpenUnicapDevice ()) {
				return show_error (__func__, "Error opening Unicap camera");
			}
			aug->imdisp.W = AUG_WHITE_1;
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
        #ifdef HAVE_SX_CAM
        case SX:
			L_print ("{b}****---->>>> Opening Starlight Xpress camera\n");
			if (!OpenSXDevice ()) {
				return show_error (__func__, "Error opening Starlight Xpress "
				                             "camera - are permissions set "
				                             "correctly?\n");
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
				return show_error (__func__, "Error opening Starlight Xpress "
				                             "guide head - are permissions set "
				                             "correctly?\n");
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
		default:
			return show_error (__func__, "Unknown camera device");
	}
	
	if (aug->cam_cap.max_h > AUGCANV_H || aug->cam_cap.max_v > AUGCANV_V) {
		err = "Autoguider camera image exceeds display limits";
		goto open_err;
	}
    
	/* Allocate storage for image data */
	
	if (!(aug->ff161 = (gushort *) g_malloc0 (aug->cam_cap.max_h * 
									   aug->cam_cap.max_v * sizeof(gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	/* If V4L, aug->r083 points into the fifo buffer */
	if (aug->device != V4L) {
		if (!(aug->r083 = (guchar *) g_malloc0 (aug->cam_cap.max_h * 
								   aug->cam_cap.max_v * 3 * sizeof (guchar)))) {
			err = "Unable to allocate buffer memory";
			goto open_err;
		}
	}
	
	if (!(aug->disp083 = (guchar *) g_malloc0 (aug->cam_cap.max_h * 
									 aug->exd.v_pix * 3 * sizeof (guchar)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->dark.dk161 =(gushort *)g_malloc0 (aug->cam_cap.max_h *
									  aug->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->r161 = (gushort *) g_malloc0 (aug->cam_cap.max_h *
								  aug->cam_cap.max_v * sizeof (gushort)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	if (!(aug->rect.mode[GREY].hist = (guint *) g_malloc0 ((aug->imdisp.W + 1) *
												sizeof (guint)))) {
		err = "Unable to allocate buffer memory";
		goto open_err;
	}
	
	/* Set some image decoding parameters for Unicap and V4L devices, after
	 * memory has been allocated.
	 */
	
	if (aug->device == UNICAP || aug->device == V4L)
		initialise_image_decoding ();
	
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
        #ifdef HAVE_SX_CAM
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

	if (aug->ff161) {
		g_free (aug->ff161);
		aug->ff161 = NULL;
	}
	
	/* If V4L, aug->r083 points into the fifo buffer */
	if (aug->r083) {
		if (aug->device != V4L)
			g_free (aug->r083);
		else
			g_free (aug->vid_dat.fifo.alloc); /* Allocated in OpenV4LDevice */
	}
	aug->r083 = NULL;
	
	if (aug->disp083) {
		g_free (aug->disp083);
		aug->disp083 = NULL;
	}
	
	if (aug->dark.dk161) {
		g_free (aug->dark.dk161);
		aug->dark.dk161 = NULL;
	}
	
	if (aug->r161) {
		g_free (aug->r161);
		aug->r161 = NULL;
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
		#ifdef HAVE_SX_CAM
		case SX:
		case SX_GH:
			/* Subsets of image area not allowed - so set array size to 
			 * maximum possible value.  No need to worry about mapping from
			 * image display coordinates to camera coordinates for a subframe.
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
		case UNICAP:  /* Nothing to do... free-running and beyond our control */ 
			return TRUE;
			break;
		case V4L:
			g_static_mutex_lock (&aug->vid_dat.fifo.mutex);
			/* If buffer[0] is not NULL then it will point to the start of a
			 * block of allocated memory where an image can be found.
			 */
			Ready = (aug->vid_dat.fifo.buffer[0] != NULL);
			g_static_mutex_unlock (&aug->vid_dat.fifo.mutex);
			return Ready;
			break;
		#ifdef HAVE_SX_CAM
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
	/* Capture the most recently exposed autoguider camera image.  Note that we 
	 * don't go and grab a frame from the Unicap device; instead it gives us one
	 * via a callback for each new frame.  For V4L devices,
	 * augcam_grab_v4l_buffer is called by the thread in loop.c to do the
	 * grabbing.
	 */
	
	gint h, v, bytes;
	
	switch (aug->device) {
		case UNICAP:
			/* augcam_unicap_new_frame_cb copies the Unicap buffer to 
			 * aug->r083, so nothing more to do here.
			 */
		break;
		case V4L:
			/* augcam_grab_v4l_buffer stores a buffer in the fifo and
			 * augcam_process_image points aug->r083 to it, so nothing more 
			 * to do here.
			 */
			break;
       #ifdef HAVE_SX_CAM
        case SX:
        case SX_GH:
			/* Gets image data in aug->r161 */
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
			
			if (!sxc_get_imagearray (&a_cam, aug->r161))
				return show_error (__func__, "Failed to read image data");
			break;
		#endif
		default:
			return show_error (__func__, "Unknown autoguider camera device");
	}
	
	/* Get the RA worm position closest to this exposure if requested, for
	 * doing PEC analysis.  Note this works best if we have short exposures.
	 * A slightly more sophisticated approach would use the average of this
	 * value and the previous one.
	 */
	 
	if (aug->autog.Worm) {
		G_print ("Requesting worm position...\n");
		aug->autog.worm_pos = telescope_get_RA_worm_pos ();
	} else
		aug->autog.worm_pos = 0;
	
	return TRUE;
}

gboolean augcam_process_image (void)
{
	/* Do image processing */
	 
	if (aug->device == V4L) {
		g_static_mutex_lock (&aug->vid_dat.fifo.mutex);
		if (!aug->vid_dat.fifo.buffer[0]) { /* Shouldn't happen: image_ready */
			g_static_mutex_unlock (&aug->vid_dat.fifo.mutex);
			return show_error (__func__, "No image to process!");
		}
		/* buffer[0] always gets filled first, so points to the location of the
		 * older frame, if the buffer holds two.
		 */
		aug->r083 = aug->vid_dat.fifo.buffer[0];
		g_static_mutex_unlock (&aug->vid_dat.fifo.mutex);
	}
	
	/* Convert to greyscale and dark subtract if required */
	
	if (!convert_to_grey ())
		return FALSE;
		
	if (aug->device == V4L) {
		g_static_mutex_lock (&aug->vid_dat.fifo.mutex);
		/* Finished with the buffered frame, so now 'release' it to be
		 * overwritten.  buffer[0] will now point to the newer frame (if there 
		 * are two); otherwise is set to NULL if buffer[1] has not been set in
		 * the GrabV4LBuffer routine in the mean time.
		 */
		aug->vid_dat.fifo.buffer[0] = aug->vid_dat.fifo.buffer[1];
		aug->vid_dat.fifo.buffer[1] = NULL;
		g_static_mutex_unlock (&aug->vid_dat.fifo.mutex);
	}
	
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
		augcam_write_starpos ();
	
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
	
	#ifdef HAVE_LIBV4L2 
	struct v4l2_control vid_ctrl;
	#endif	 
	
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
        #ifdef HAVE_SX_CAM
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

static void initialise_image_window (enum HWDevice dev)
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
    if (!v4l2ioctl (aug->fd, VIDIOC_G_CTRL, &vid_ctrl)) {
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

gboolean augcam_write_starpos (void)
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
		loop_elapsed_since_first_iteration () / 1000.0, aug->autog.worm_pos, 
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
	if (fread (aug->dark.dk161, sizeof (gushort), aug->exd.h_pix *
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
	memset (aug->dark.dk161, 0, aug->exd.h_pix * aug->exd.v_pix * 
	                                                          sizeof (gushort));
	if (fwrite (aug->dark.dk161, sizeof (gushort), aug->exd.h_pix *
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
	if (fwrite (aug->dark.dk161, sizeof (gushort), aug->exd.h_pix *
	                    aug->exd.v_pix, fpd) != aug->exd.h_pix * aug->exd.v_pix)
		msg ("Warning - Couldn't write dark frame to file");
}

#ifdef HAVE_UNICAP
static gboolean OpenUnicapDevice (void)
{
	/* Open the selected Unicap device (the device and format structures are
	 * set in the main interface routines before calling augcam_open).
	 */
	 
	L_print ("Device: %s\n", aug->ucp_device.identifier);
	L_print ("Format: %c%c%c%c, %dx%d\n", aug->ucp_format_spec.fourcc,
			                              aug->ucp_format_spec.fourcc >> 8, 
					                      aug->ucp_format_spec.fourcc >> 16, 
			                              aug->ucp_format_spec.fourcc >> 24,
	                                      aug->ucp_format_spec.size.width,
	                                      aug->ucp_format_spec.size.height);
	
	/* Get the device information specific to the requested device */
	
	if (!SUCCESS (unicap_enumerate_devices (
	                                  &aug->ucp_device, &aug->ucp_device, 0))) {
		L_print ("{r}%s: Failed to get info for Unicap device %s\n", 
				                      __func__, aug->ucp_device.identifier);
	    return FALSE;
	}
	
	/* Acquire a handle to the device */

	if (!SUCCESS (unicap_open (&aug->ucp_handle, &aug->ucp_device)))
		return show_error (__func__, "Failed to acquire handle for Unicap "
						                                              "device");
	
	/* Get the full format information specific to the requested format and set
	 * the format.
	 */

	if (!SUCCESS (unicap_enumerate_formats (
				  aug->ucp_handle, &aug->ucp_format_spec, &aug->ucp_format, 0)))
		return show_error (__func__, "Failed to get video format information");
	
	/* The DFG/1394 insists on returning the 768x576 size even if you ask for
	 * something else, so override the size setting here.  Assume this is OK
	 * for other Unicap devices.
	 */
	
	aug->ucp_format.size.width = aug->ucp_format_spec.size.width;
	aug->ucp_format.size.height = aug->ucp_format_spec.size.height;
	aug->ucp_format.fourcc = aug->ucp_format_spec.fourcc;
	
	if (!SUCCESS (unicap_set_format (aug->ucp_handle, &aug->ucp_format)))
		return show_error (__func__, "Failed to set video format");
	
	aug->cam_cap.max_h = aug->ucp_format.size.width;
	aug->cam_cap.max_v = aug->ucp_format.size.height;
	augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);
	aug->vid_dat.pixfmt = aug->ucp_format.fourcc;
	
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
	if (aug->ucp_handle)
		if (!SUCCESS (unicap_close (aug->ucp_handle)))
			return FALSE;
	aug->ucp_handle = NULL;
	
	close_liveview_window ();
	return TRUE;
}
#endif

#ifdef HAVE_UNICAP
void augcam_unicap_new_frame_cb (unicap_event_t event, unicap_handle_t handle, 
						         unicap_data_buffer_t *buffer, gpointer *data)
{
	/* Make a copy of the unicap image buffer data for each new frame if 
	 * the autoguider needs it, and buffer the data if recording.
	 */

	if (aug->Open) {
		if (buffer->buffer_size <= aug->cam_cap.max_h * aug->cam_cap.max_v * 3)
			memcpy (aug->r083, buffer->data, buffer->buffer_size);
		else
			L_print ("{r}%s: unicap buffer size exceeds "
			         "allocated space\n", __func__);
	}
	
	/* Buffer frame if recording.  This buffers only the first width * height
	 * bytes of the data, so if the selected format is GREY, Y800, YUV420, 
	 * YVU420, YU12 or YV12 then we get a greyscale recorded image; otherwise it
	 * may be 'junk'.  We deliberately do not do any further format conversions
	 * beyond those done internally by unicap for performance reasons since we 
	 * need to be able to buffer and write every frame to disk when recording.
	 */
	
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
	struct v4l2_standard vid_std;
	struct v4l2_input vid_input;
    struct v4l2_format vid_fmt;
	struct v4l2_streamparm vid_sparm;
    struct v4l2_buffer buffer;
    struct v4l2_requestbuffers reqbuf;
	gint buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    /* Open device - no need to open NONBLOCK because buffers are dequeued
     * in a separate thread that can block until one is available.
     */
    
	//if ((aug->fd = v4l2_open (device, O_RDWR | O_NONBLOCK, 0)) < 0)
	if ((aug->fd = v4l2_open (device, O_RDWR, 0)) < 0)
		return show_error (__func__, "Error opening V4L autoguider "
		                                                       "camera device");
	
    /* Get camera basic data */
	
    memset (&vid_cap, 0, sizeof (vid_cap));
	if (v4l2ioctl (aug->fd, VIDIOC_QUERYCAP, &vid_cap))
		return show_error (__func__, "Could not get camera capabilities");
	
	L_print ("- V4L device information -\n");
	L_print ("Driver = %s\n", vid_cap.driver);
	L_print ("Card = %s\n", vid_cap.card);
	strncpy (aug->vid_dat.card, (const gchar *) vid_cap.card, 127);
	L_print ("Bus info = %s\n", vid_cap.bus_info);
	if (!(vid_cap.capabilities & V4L2_CAP_STREAMING)) {
		L_print ("{o}Selected V4L device does not support streaming!\n");
		return FALSE;
	}
	 
	/* Restore default settings (if any) */
	
    if (get_V4L_settings ((gchar *)vid_cap.card)) {
		
		if (aug->vid_dat.HasVideoStandard) {
			memset (&vid_std, 0, sizeof (vid_std));
			/* Config. file stores just the selected standard */
			vid_std.id = aug->vid_dat.vid_std.id[0];
			if (v4l2ioctl (aug->fd, VIDIOC_S_STD, &vid_std.id))
				L_print ("{o}Could not set video standard for V4L device\n");
		}
		
		memset (&vid_input, 0, sizeof (vid_input));
		vid_input.index = aug->vid_dat.vid_input.selected;
		if (v4l2ioctl (aug->fd, VIDIOC_S_INPUT, &vid_input))
			L_print ("{o}Could not set video input for V4L device\n");
			
		memset (&vid_fmt, 0, sizeof (vid_fmt));
		vid_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vid_fmt.fmt.pix.pixelformat = aug->vid_dat.pixfmt;
		vid_fmt.fmt.pix.width       = aug->vid_dat.width;
		vid_fmt.fmt.pix.height      = aug->vid_dat.height;
		vid_fmt.fmt.pix.field       = V4L2_FIELD_ANY;
		if (v4l2ioctl (aug->fd, VIDIOC_S_FMT, &vid_fmt))
			L_print ("{o}Could not set image size for V4L device\n");
			
		if (aug->vid_dat.fps > 0.0) {
			memset (&vid_sparm, 0, sizeof (vid_sparm));
			vid_sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vid_sparm.parm.capture.timeperframe.denominator=100*aug->vid_dat.fps;
			vid_sparm.parm.capture.timeperframe.numerator  =100;
			if (v4l2ioctl (aug->fd, VIDIOC_S_PARM, &vid_sparm))
				L_print ("{o}Could not set frame rate for V4L device\n");
		}
	}
	
	/* List all available image formats and choose an appropriate one */
	
	if (!V4L_list_image_formats (&vid_fmt))
		return FALSE;
	
	aug->vid_dat.pixfmt = (aug->imdisp.greyscale == MONO) ? V4L2_PIX_FMT_GREY :
	                                                vid_fmt.fmt.pix.pixelformat;
	                                                
   /* List all available video standards, video inputs, frame sizes and
     * frame rates.
     */
    
    if (!V4L_list_video_settings (vid_fmt.fmt.pix.pixelformat))
		return FALSE;
    
	/* Store and list default video standard, video input, image size and 
	 * frame rate.
	 */

	V4L_list_current_settings (&vid_fmt);
	augcam_set_image_size (aug->cam_cap.max_h, aug->cam_cap.max_v);

	/* Set the brightness, contrast, gamma and gain sliders on the autoguider
	 * image window.
	 */
	
	V4L_initialise_bcg_sliders ("brightness", V4L2_CID_BRIGHTNESS, AIW_BRIGHTNESS);
	V4L_initialise_bcg_sliders ("contrast", V4L2_CID_CONTRAST, AIW_CONTRAST);
	V4L_initialise_bcg_sliders ("gamma", V4L2_CID_GAMMA, AIW_GAMMA);
	V4L_initialise_bcg_sliders ("gain", V4L2_CID_GAIN, AIW_GAIN);
	
	/* Initialise and queue memory buffers */

	memset (&reqbuf, 0, sizeof (reqbuf));
    reqbuf.count = 2;  /* Note: The driver may allocate more than this */
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
	if (v4l2ioctl (aug->fd, VIDIOC_REQBUFS, &reqbuf))
		return show_error (__func__, "Buffer memory request failed");
		                                                              
    aug->vid_dat.buffers = calloc (reqbuf.count, sizeof(*aug->vid_dat.buffers));
    for (aug->vid_dat.bufnum = 0; aug->vid_dat.bufnum < reqbuf.count; 
                                                        aug->vid_dat.bufnum++) {
		memset (&buffer, 0, sizeof (buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = aug->vid_dat.bufnum;

		if (v4l2ioctl (aug->fd, VIDIOC_QUERYBUF, &buffer))
			return show_error (__func__, "Unable to query buffer information");

        aug->vid_dat.buffers[aug->vid_dat.bufnum].length = buffer.length;
        aug->vid_dat.buffers[aug->vid_dat.bufnum].start = 
                                             v4l2_mmap (NULL, buffer.length,
                                             PROT_READ | PROT_WRITE, MAP_SHARED,
                                             aug->fd, buffer.m.offset);

        if (MAP_FAILED == aug->vid_dat.buffers[aug->vid_dat.bufnum].start)
			return show_error (__func__, "Unable to map buffer memory");

		if (v4l2ioctl (aug->fd, VIDIOC_QBUF, &buffer))
			return show_error (__func__, "Unable to queue memory buffer");
    }
   
	/* Allocate fifo buffer */
	
	aug->vid_dat.fifo.size = aug->vid_dat.buffers[0].length;
	aug->vid_dat.fifo.alloc = g_malloc (aug->vid_dat.fifo.size * 2);
	
	/* Start streaming */
	
	if (v4l2ioctl (aug->fd, VIDIOC_STREAMON, &buf_type))
		return show_error (__func__, "Unable to start video streaming");
		
	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean CloseV4LDevice (void)
{
	/* Close a V4L device */
	
	gushort i;
	gint buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	 
	/* Stop streaming */
	
	if (v4l2ioctl (aug->fd, VIDIOC_STREAMOFF, &buf_type))
		return show_error (__func__, "Unable to stop video streaming");
		
	/* Unmap memory buffers */
	
    for (i = 0; i < aug->vid_dat.bufnum; i++) {
		v4l2_munmap (aug->vid_dat.buffers[i].start, 
		             aug->vid_dat.buffers[i].length);
	}
	
	if (aug->vid_dat.frames_tot)
		L_print ("Dropped %d out of %d frames (%5.2f %%)\n", 
	                                                  aug->vid_dat.frames_drop, 
	                                                  aug->vid_dat.frames_tot,
	                                    100 *(gfloat) aug->vid_dat.frames_drop / 
	                                         (gfloat) aug->vid_dat.frames_tot);
		             
	/* Close device */
                
     v4l2_close (aug->fd);

	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
gboolean augcam_grab_v4l_buffer (void)
{
	/* Grab a frame from the V4L device */
    
	struct v4l2_buffer buffer;
	gint r, fifo_pos = -1;
	guchar* fifo_dest;
	
	/* Check the device is available */
	
	if (!aug->Open)
		return FALSE;
		
    /* De-queue a buffer */
    
    memset (&buffer, 0, sizeof (buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
	if ((r = v4l2ioctl (aug->fd, VIDIOC_DQBUF, &buffer))) {
		if (r == ENODEV)
			return FALSE;  /* Device has gone away */
		/* With the fifo buffer system, it's not bad to return TRUE */
		return TRUE;
	}
			
	/* Copy buffered image in fifo */
	
	g_static_mutex_lock(&aug->vid_dat.fifo.mutex);
	if (!aug->vid_dat.fifo.buffer[0]) { /* i.e. if the buffer is empty */
		fifo_pos = 0;
		fifo_dest = aug->vid_dat.fifo.alloc;  /* Base of first location */
	} else if (!aug->vid_dat.fifo.buffer[1]) {  
		/* i.e. if augcam_process_image hasn't reset buffer[0] but buffer[1] 
		 * doesn't point to anything yet...
		 */
		fifo_pos = 1;
		if (aug->vid_dat.fifo.buffer[0] == aug->vid_dat.fifo.alloc) {  
			/* Occupied slot points to first location, so store in second 
			 * location.
			 */
			fifo_dest = aug->vid_dat.fifo.alloc + aug->vid_dat.fifo.size;
		} else {
			/* Occupied slot points to second location, so store in first 
			 * location.
			 */
			fifo_dest = aug->vid_dat.fifo.alloc;
		}
	}
	if (fifo_pos >= 0) {  
		/* Copy most-recently dequeued buffer to the appropriate empty slot...*/
		memcpy (fifo_dest,
				aug->vid_dat.buffers[buffer.index].start,
				aug->vid_dat.fifo.size);
		aug->vid_dat.fifo.buffer[fifo_pos] = fifo_dest; /*... and point to it */
		aug->vid_dat.frames_tot++;
	} else {
		aug->vid_dat.frames_drop++;
		G_print ("V4L dropping frame (%d/%d)\n", aug->vid_dat.frames_drop, 
		                                         aug->vid_dat.frames_tot);
	}
	g_static_mutex_unlock (&aug->vid_dat.fifo.mutex);
	
	/* Re-queue buffer */
		
	if (v4l2ioctl (aug->fd, VIDIOC_QBUF, &buffer))
		return show_error (__func__, "Error re-queueing buffer");
		
	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean V4L_list_image_formats (struct v4l2_format *vid_fmt)
{
	/* List available image formats and choose an appropriate one */
	
	struct v4l2_fmtdesc fmt_desc;
	gchar *fld;
	
	L_print ("- Image formats -\n");
	memset (&fmt_desc, 0, sizeof (fmt_desc));
	fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	do {
		if (v4l2ioctl (aug->fd, VIDIOC_ENUM_FMT, &fmt_desc))
			break;
		L_print ("%s; %s; %s; %c%c%c%c\n", 
		fmt_desc.flags & V4L2_FMT_FLAG_COMPRESSED?"Compressed" : "Uncompressed",
		fmt_desc.flags & V4L2_FMT_FLAG_EMULATED  ?"Emulated"     : "Native",
		fmt_desc.description,
		fmt_desc.pixelformat,
		fmt_desc.pixelformat >> 8,
		fmt_desc.pixelformat >> 16,
		fmt_desc.pixelformat >> 24);
	} while (++fmt_desc.index);
	if (!fmt_desc.index) {
		L_print ("{o}Didn't find any video capture image formats!\n");
		return FALSE;
	}
	
	/* See what the default format is... */
	
	memset (vid_fmt, 0, sizeof (*vid_fmt));
    vid_fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (v4l2ioctl (aug->fd, VIDIOC_G_FMT, vid_fmt))
		return show_error (__func__, "Could not get camera video format");
	L_print ("Current default image format is %c%c%c%c...\n",
                                            vid_fmt->fmt.pix.pixelformat,
		                                    vid_fmt->fmt.pix.pixelformat >> 8,
		                                    vid_fmt->fmt.pix.pixelformat >> 16,
		                                    vid_fmt->fmt.pix.pixelformat >> 24);
		                                     
	/* Try to set a format that GoQat can decode, if it isn't already suitable*/
	 	
	switch (vid_fmt->fmt.pix.pixelformat) {/* These options should also appear*/
		case V4L2_PIX_FMT_Y10:             /*  in initialise_image_decoding   */
		case V4L2_PIX_FMT_Y12:
		case V4L2_PIX_FMT_Y16:
		case V4L2_PIX_FMT_GREY:
		case FOURCC_Y8:
		case FOURCC_Y800:
		case V4L2_PIX_FMT_YUV420: /* Numerically identical to YU12 */
		case V4L2_PIX_FMT_YVU420: /* Numerically identical to YV12 */
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YVYU:
		case FOURCC_YUY2:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR24:
			break;  /* Do nothing; GoQat can handle these */
		default:
			/* If the format doesn't match any of the above, request conversion 
			 * to YUV420 (YU12).  libv4l2 should do this using libv4lconvert on 
			 * the fly, according to the documentation.  If libv4l2 converts the
			 * 'weird' formats that it knows about automatically then the 
			 * following case is actually redundant.
			 */
			L_print ("...requesting conversion to YUV420/YU12...\n");
			vid_fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
			vid_fmt->fmt.pix.field       = V4L2_FIELD_ANY;
			if (v4l2ioctl (aug->fd, VIDIOC_S_FMT, vid_fmt))
				return show_error (__func__, "Could not set "
				                                         "camera video format");
			break;
	}

	/* The VIDIOC_S_FMT request updates the values of the data struture, so see
	 * what we actually got.
	 */
	 
	if (vid_fmt->fmt.pix.field == V4L2_FIELD_NONE)
		fld = "(Progressive image)";
	else if (vid_fmt->fmt.pix.field == V4L2_FIELD_INTERLACED)
		fld = "(Interlaced fields)";
	else if (vid_fmt->fmt.pix.field == V4L2_FIELD_TOP)
		fld = "(Top field only)";
	else if (vid_fmt->fmt.pix.field == V4L2_FIELD_BOTTOM)
		fld = "(Bottom field only)";
	else if (vid_fmt->fmt.pix.field == V4L2_FIELD_ALTERNATE)
		fld = "(Successive fields alternately)";
	else
		fld = "(Interlaced image format)";
	L_print ("...using %c%c%c%c %s\n", vid_fmt->fmt.pix.pixelformat,
									   vid_fmt->fmt.pix.pixelformat >> 8,
									   vid_fmt->fmt.pix.pixelformat >> 16,
									   vid_fmt->fmt.pix.pixelformat >> 24,
									   fld);
									   
	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static gboolean V4L_list_video_settings (guint32 pixelformat)
{
	/* List the available video standards and inputs */
	
    struct v4l2_standard vid_std;
    struct v4l2_input vid_input;
    v4l2_std_id current_stdid = 0;
    gboolean GotPAL = FALSE, GotNTSC = FALSE, GotSECAM = FALSE;
    
	memset (&vid_std, 0, sizeof (vid_std));
	aug->vid_dat.vid_std.num = 0;
	if (!v4l2ioctl (aug->fd, VIDIOC_G_STD, &current_stdid))/* Store current id*/
		L_print ("- Video standards -\n");
	do {
		if (v4l2ioctl (aug->fd, VIDIOC_ENUMSTD, &vid_std))
			break;
			
		/* For each occurrence of a PAL, NTSC or SECAM standard, set the
		 * standard and see what frame sizes are available.  There's no need
		 * to do it for every subvariant of each standard - the frame sizes
		 * should be the same and stepping through each one takes all night.
		 */

		if (vid_std.id & V4L2_STD_PAL && !GotPAL) {
			L_print ("PAL standards...\n");
			/* Set standard, then see what frame rates are available */
			if (v4l2ioctl (aug->fd, VIDIOC_S_STD, &vid_std.id))
				return show_error (__func__, "Unable to set video standard");
			V4L_list_frame_sizes (pixelformat);
			GotPAL = TRUE;
		}
		if (vid_std.id & V4L2_STD_NTSC && !GotNTSC) {
			L_print ("NTSC standards...\n");
			/* Set standard, then see what frame rates are available */
			if (v4l2ioctl (aug->fd, VIDIOC_S_STD, &vid_std.id))
				return show_error (__func__, "Unable to set video standard");
			V4L_list_frame_sizes (pixelformat);
			GotNTSC = TRUE;
		}
		if (vid_std.id & V4L2_STD_SECAM && !GotSECAM) {
			L_print ("SECAM standards...\n");
			/* Set standard, then see what frame rates are available */
			if (v4l2ioctl (aug->fd, VIDIOC_S_STD, &vid_std.id))
				return show_error (__func__, "Unable to set video standard");
			V4L_list_frame_sizes (pixelformat);
			GotSECAM = TRUE;
		}
		
		/* List all the variants of each standard */
			
		L_print ("%s\n", vid_std.name);
		strncpy (aug->vid_dat.vid_std.name[aug->vid_dat.vid_std.num],
				 (const gchar *)vid_std.name, 127);
		aug->vid_dat.vid_std.id[aug->vid_dat.vid_std.num++] = vid_std.id;
	} while (++vid_std.index < MAX_VIDITEMS);
	if (!vid_std.index) {
		V4L_list_frame_sizes (pixelformat);
	}
	/* Restore current standard */
	if (current_stdid && v4l2ioctl (aug->fd, VIDIOC_S_STD, &current_stdid))
		return show_error (__func__, "Unable to set video standard");
			
	L_print ("- Video inputs -\n");
	memset (&vid_input, 0, sizeof (vid_input));
	aug->vid_dat.vid_input.num = 0;
	do {
		if (v4l2ioctl (aug->fd, VIDIOC_ENUMINPUT, &vid_input))
			break;
		L_print ("%i: %s\n", vid_input.index, vid_input.name);
		strncpy (aug->vid_dat.vid_input.name[aug->vid_dat.vid_input.num++],
		         (const gchar *)vid_input.name, 127);
	} while (++vid_input.index < MAX_VIDITEMS);
   
	return TRUE;
}
#endif

#ifdef HAVE_LIBV4L2
static void V4L_list_frame_sizes (guint32 pixelformat)
{
	/* List available frame sizes */
	
    struct v4l2_frmsizeenum vid_frmsizeenum;
    
	memset (&vid_frmsizeenum, 0, sizeof (vid_frmsizeenum));
	vid_frmsizeenum.pixel_format = pixelformat; 
	do {
		if (v4l2ioctl (aug->fd, VIDIOC_ENUM_FRAMESIZES, &vid_frmsizeenum))
			break;
		if (vid_frmsizeenum.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			L_print ("Minimum width: %d, step: %d, maximum width: %d\n", 
			                               vid_frmsizeenum.stepwise.min_width,
			                               vid_frmsizeenum.stepwise.step_width,
			                               vid_frmsizeenum.stepwise.max_width);
			L_print ("Minimum height: %d, step: %d, maximum height: %d", 
			                               vid_frmsizeenum.stepwise.min_height,
			                               vid_frmsizeenum.stepwise.step_height,
			                               vid_frmsizeenum.stepwise.max_height);
			V4L_list_frame_rates (pixelformat,
			                      vid_frmsizeenum.stepwise.max_width,
			                      vid_frmsizeenum.stepwise.max_height);
			break;
		} else if (vid_frmsizeenum.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
			L_print ("Continuously variable between "
			         "minimum width: %d and maximum width: %d\n", 
			                               vid_frmsizeenum.stepwise.min_width,
			                               vid_frmsizeenum.stepwise.max_width);
			L_print ("Continuously variable between "
			         "minimum height: %d and maximum height: %d", 
			                               vid_frmsizeenum.stepwise.min_height,
			                               vid_frmsizeenum.stepwise.max_height);
			V4L_list_frame_rates (pixelformat, 
			                      vid_frmsizeenum.stepwise.max_width,
			                      vid_frmsizeenum.stepwise.max_height);
			break;
		} else { /* Discrete */
			L_print ("%d x %d pixels", vid_frmsizeenum.discrete.width,
			                           vid_frmsizeenum.discrete.height);
			V4L_list_frame_rates (pixelformat, 
			                      vid_frmsizeenum.discrete.width, 
			                      vid_frmsizeenum.discrete.height);
		}
	} while (++vid_frmsizeenum.index);
}
#endif

#ifdef HAVE_LIBV4L2
static void V4L_list_frame_rates (guint32 pixelformat, gint width, gint height)
{
	/* List available frame rates */
	
    struct v4l2_frmivalenum vid_frmivalenum;
    GString *rates = g_string_new (" at ");
    
	memset (&vid_frmivalenum, 0, sizeof (vid_frmivalenum));
	vid_frmivalenum.pixel_format = pixelformat; 
	vid_frmivalenum.width = width;
	vid_frmivalenum.height = height;
	do {
		if (v4l2ioctl (aug->fd, VIDIOC_ENUM_FRAMEINTERVALS, &vid_frmivalenum))
			break;
		if (vid_frmivalenum.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
			/* Remember rate is 1 / interval... */
			L_print ("\nMinimum rate: %d, step: %d, maximum rate: %d "
			         "frames per second\n", 
			          vid_frmivalenum.stepwise.max.denominator /
			          vid_frmivalenum.stepwise.max.numerator,
			          vid_frmivalenum.stepwise.step.denominator /
			          vid_frmivalenum.stepwise.step.numerator,
			          vid_frmivalenum.stepwise.min.denominator / 
			          vid_frmivalenum.stepwise.min.numerator);
			break;
		} else if (vid_frmivalenum.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
			L_print ("\nContinuously variable between minimum rate: %d "
			         "and maximum rate: %d frames per second\n", 
			          vid_frmivalenum.stepwise.max.denominator /
			          vid_frmivalenum.stepwise.max.numerator,
			          vid_frmivalenum.stepwise.min.denominator /
			          vid_frmivalenum.stepwise.min.numerator);
			break;
		} else { /* Discrete */
			g_string_append_printf (rates, "%d : ", 
			          vid_frmivalenum.discrete.denominator /
			          vid_frmivalenum.discrete.numerator);
		}
	} while (++vid_frmivalenum.index);
	if (vid_frmivalenum.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
		g_string_append_printf (rates, "frames per second");
		L_print ("{.}%s", rates->str);
	}
	L_print ("{.}\n");
	g_string_free (rates, TRUE);
}
#endif

#ifdef HAVE_LIBV4L2
static void V4L_list_current_settings (struct v4l2_format *vid_fmt)
{
	/* Store and list the current settings */
	
	struct v4l2_streamparm vid_sparm;
	v4l2_std_id vid_stdid;
    GString *settings = g_string_new ("");
    
 	aug->vid_dat.width = vid_fmt->fmt.pix.width;
	aug->vid_dat.height = vid_fmt->fmt.pix.height;
	aug->vid_dat.size = vid_fmt->fmt.pix.sizeimage;
	aug->cam_cap.max_h = vid_fmt->fmt.pix.width;
	aug->cam_cap.max_v = vid_fmt->fmt.pix.height;

	memset (&vid_stdid, 0, sizeof (vid_stdid));
	v4l2ioctl (aug->fd, VIDIOC_G_STD, &vid_stdid);
	v4l2ioctl (aug->fd, VIDIOC_G_INPUT, &aug->vid_dat.vid_input.selected);
	
	memset (&vid_sparm, 0, sizeof (vid_sparm));
	vid_sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2ioctl (aug->fd, VIDIOC_G_PARM, &vid_sparm);
	if (vid_sparm.parm.capture.timeperframe.numerator)
		aug->vid_dat.fps = vid_sparm.parm.capture.timeperframe.denominator / 
	                       vid_sparm.parm.capture.timeperframe.numerator;
	else
		aug->vid_dat.fps = 0.0;
	
	L_print ("- Current settings -\n");
	for (aug->vid_dat.vid_std.selected = 0; 
	     aug->vid_dat.vid_std.selected < aug->vid_dat.vid_std.num; 
	     aug->vid_dat.vid_std.selected++) {
		if (vid_stdid ==aug->vid_dat.vid_std.id[aug->vid_dat.vid_std.selected]){
			g_string_append_printf (settings, "Video standard: %s, ", 
				      aug->vid_dat.vid_std.name[aug->vid_dat.vid_std.selected]);
			break;
		}
	}
	if (aug->vid_dat.vid_input.num > 0)
		g_string_append_printf (settings, "Video input: %s, ", 
		          aug->vid_dat.vid_input.name[aug->vid_dat.vid_input.selected]);
	g_string_append_printf (settings, "%d x %d, ", 
	                                   aug->vid_dat.width, aug->vid_dat.height);
	if (aug->vid_dat.fps > 0)
		g_string_append_printf (settings, "%5.2f frames per second", 
		                                                      aug->vid_dat.fps);
	g_string_append_printf (settings, "\n");
	L_print (settings->str);
	g_string_free (settings, TRUE);
}
#endif

#ifdef HAVE_LIBV4L2
static gint v4l2ioctl (gint fh, gint request, void *arg)
{
	/* Wrapper for v4l ioctl calls */
	
	gint r;
	
	r = v4l2_ioctl (fh, request, arg);
	return (r == -1 ? errno : 0);
}
#endif

#ifdef HAVE_SX_CAM
gboolean augcam_get_sx_cameras (void)
{
	/* Fill the device selection structure with a list of detected sx cameras */
	
	sx_error_func (ccdcam_sx_error_func);/* Use same error handler as CCD code*/
	
	ds.num = MAX_DEVICES;
	if (!sxc_get_cameras (&a_cam, ds.id, ds.desc, &ds.num))
		return show_error (__func__, "Error searching for cameras!");
	if (!ds.num) {
	    L_print ("{o}Didn't find any cameras\n");
	    return FALSE;
	}
	    
	return TRUE;	
}
#endif

#ifdef HAVE_SX_CAM
static gboolean OpenSXDevice (void)
{
	/* Open and initialise a Starlight Xpress camera */
	
	gint num;

	strcpy (aug->cam_cap.camera_manf, "SX");
	
	sx_error_func (ccdcam_sx_error_func);/* Use same error handler as CCD code*/
	
	num = MAX_DEVICES;
	if (!sxc_get_cameras (&a_cam, ds.id, ds.desc, &num))
		return show_error (__func__, "Error searching for cameras!");
	if (num == 0) {
	    L_print ("{o}Didn't find any cameras\n");
		return FALSE;
	} else if (num == 1) {
		if (!sxc_connect (&a_cam, TRUE, ds.id[0]))
			return show_error (__func__, "Unable to connect to camera!");
		aug->devnum = 0;
	} else if (num > 1 && aug->devnum == -1) {
		L_print ("{o}Found more than one camera!  Please select the one you "
				 "want via the 'Cameras' menu\n");
	    return FALSE;
    } else if (num > 1 && aug->devnum < num) {
		if (!sxc_connect (&a_cam, TRUE, ds.id[aug->devnum]))
			return show_error (__func__, "Unable to connect to camera!");
	} else
		return show_error (__func__, "Error opening camera!");
	
	strncpy (aug->cam_cap.camera_desc, ds.desc[aug->devnum], 256);
	
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

#ifdef HAVE_SX_CAM
static gboolean CloseSXDevice (void)
{
	/* Close a Starlight Xpress camera */
	
	sxc_cancel_exposure (&a_cam);
	if (!sxc_connect (&a_cam, FALSE, NULL))
		return show_error (__func__, "Unable to close Starlight Xpress camera");

	return TRUE;
}
#endif

#ifdef HAVE_SX_CAM
static gboolean OpenSXGuideHead (void)
{
	/* Initialise the guide head attached to the main Starlight Xpress camera */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	if (ccd->device == SX && ccd->Open) {
		
		/* Get a copy of the usbdevice structure for the CCD camera and set the
		 * camera ID to 1 to indicate that any calls made to the device are for
		 * the guide head rather than the main camera.
		 */
		
		a_cam.usbd = sx_get_ccdcam_usbd ();
		a_cam.usbd.id = 1;
		
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

#ifdef HAVE_SX_CAM
static gboolean CloseSXGuideHead (void)
{
	/* Tidy up after using the Starlight Xpress guide head */
	
	sxc_cancel_exposure (&a_cam);
	if (!sxc_connect (&a_cam, FALSE, NULL))
		return show_error (__func__, "Unable to close Starlight Xpress camera");
		
	return TRUE;
}
#endif

#ifdef HAVE_SX_CAM
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

static void initialise_image_decoding (void)
{
	/* Initialise bytes per pixel and image data pointer */
	
	switch (aug->vid_dat.pixfmt) {
		case V4L2_PIX_FMT_Y10:    /* Up to 16-bit greyscale */
		case V4L2_PIX_FMT_Y12:
		case V4L2_PIX_FMT_Y16:
			aug->vid_dat.type = VIDBUF_GREY2;
			aug->vid_dat.byte_incr = 2; /* Increment by 2 bytes per pixel */
			aug->vid_dat.byte_offset = 0;
			break;
			
		case V4L2_PIX_FMT_GREY:   /* Up to 8-bit greyscale */
		case FOURCC_Y8:
		case FOURCC_Y800:
			aug->vid_dat.type = VIDBUF_GREY1;
			aug->vid_dat.byte_incr = 1; /* Increment by 1 byte per pixel */
			aug->vid_dat.byte_offset = 0;
			break;
		
		/* Note: these two 4CC's are numerically identical to YU12 and YV12
		 * respectively.
		 */	
		case V4L2_PIX_FMT_YUV420: /* Select just the Y-plane */
		case V4L2_PIX_FMT_YVU420:
			aug->vid_dat.type = VIDBUF_GREY1;
			aug->vid_dat.byte_incr = 1; /* Increment by 1 byte per pixel */
			aug->vid_dat.byte_offset = 0;
			break;
			
		case V4L2_PIX_FMT_YUYV:   /* Select just the Y-plane */
		case V4L2_PIX_FMT_YVYU:
		case FOURCC_YUY2:
			aug->vid_dat.type = VIDBUF_GREY1;
			aug->vid_dat.byte_incr = 2; /* Increment by 2 bytes per pixel */
			aug->vid_dat.byte_offset = 0;
			break;
			
		case V4L2_PIX_FMT_UYVY:   /* Select just the Y-plane */
			aug->vid_dat.type = VIDBUF_GREY1;
			aug->vid_dat.byte_incr = 2; /* Increment by 2 bytes per pixel */
			aug->vid_dat.byte_offset = 1;/* Start at second byte of image data*/
			break;
			
		case V4L2_PIX_FMT_RGB24:  /* RGB/BGR - converted in convert_to_grey */
		case V4L2_PIX_FMT_BGR24:
			aug->vid_dat.type = VIDBUF_COL3;
			aug->vid_dat.byte_incr = 3; /* Increment by 3 bytes per pixel */
			aug->vid_dat.byte_offset = 0;
			break;
			
		default:
			L_print ("{o}Unknown video format - decoding as 8-bit grey\n");
			aug->vid_dat.type = VIDBUF_GREY1;
			aug->vid_dat.byte_incr = 1; /* Increment by 1 byte per pixel */
			aug->vid_dat.byte_offset = 0;
			break;	
	}
}

static gboolean convert_to_grey (void)
{
	/* Convert image data to greyscale suitable for display on the image canvas,
	 * dark-subtracting, background-subtracting and gamma-correcting as 
	 * appropriate.  Also derive some image statistics.
	 *
	 * On entering this routine:-
	 * 
	 * 		For V4L and Unicap devices:
	 *        'r083'    contains the camera data in a variety of possible 
	 *                  formats, at a maximum of 3 bytes per pixel.
	 * 	    For CCD cameras used as autoguider cameras:
	 *        'r161'    contains the camera data as greyscale at maximum of 2 
	 *                  bytes per pixel.
	 *  
	 * On leaving this routine:
	 * 
	 *        'ff161'   contains (possibly) dark-subtracted or background-
	 *                  subtracted data at a maximum of 16-bit per pixel
	 *                  greyscale.
	 *        'disp083' contains (possibly) dark-subtracted, background-
	 *                  subtracted or gamma-adjusted data in three bytes per 
	 *                  pixel greyscale format (i.e. actually in RGB format, but
	 *                  with the R, G, and B bytes for each pixel set to the 
	 *                  same value).
	 */
	
	gushort row, col;
	gushort xo1, xo2, yo1, yo2;
	guint size, count, reject;
	gint val = 0;
    gint i, j, k;
    gdouble gamma_scale, diff, rms;
	guchar *iptr = NULL;
	
	/* Return if no image */
	
	if (aug->device == UNICAP || aug->device == V4L) {
		if (!aug->r083)
			return TRUE;
	} else if (aug->device == SX || aug->device == SX_GH) {
		if (!aug->r161)
			return TRUE;
	}

	/* Read stored dark frame from file if capturing and averaging */
	
	if (aug->dark.Capture)
		augcam_read_dark_frame ();    /* Fills aug->dk161 */
	
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
	
	gamma_scale = pow (aug->imdisp.W, aug->imdisp.gamma) + 0.01;
	
	iptr = aug->r083+aug->vid_dat.byte_offset;
	for (i = 0; i < aug->exd.h_pix * aug->exd.v_pix; i++) {
		
		/* Loop over entire image and convert to greyscale (if not already) */
		
		if (aug->device == UNICAP || aug->device == V4L) {
			if (aug->vid_dat.type == VIDBUF_GREY1)
				val = *iptr;
			else if (aug->vid_dat.type == VIDBUF_GREY2)
				val = *(iptr+1) * 256 + *iptr; /* Swap bytes and combine */
			else if (aug->vid_dat.type == VIDBUF_COL3) {
				switch (aug->imdisp.greyscale) {
					case LUMIN:
						if (aug->vid_dat.pixfmt == V4L2_PIX_FMT_RGB24)
							val = (gushort) (0.11 * (gfloat) *iptr +
											 0.59 * (gfloat) *(iptr+1) +
											 0.30 * (gfloat) *(iptr+2));
						else /* BGR24 */
							val = (gushort) (0.11 * (gfloat) *(iptr+2) +
											 0.59 * (gfloat) *(iptr+1) +
											 0.30 * (gfloat) *iptr);
						break;
					case DESAT:
						val = (MAX (*iptr, MAX (*(iptr+1), *(iptr+2)) +
							   MIN (*iptr, MAX (*(iptr+1), *(iptr+2)))))/2;							
						break;
					case MAXIM:
						val = MAX (*iptr, MAX (*(iptr+1), *(iptr+2)));
						break;
					case MONO:  /* Keeps the compiler quiet! */
						break;
				}
			}
			iptr += aug->vid_dat.byte_incr;/* Set in initialise_image_decoding*/
		} else if (aug->device == SX || aug->device == SX_GH) {
			val = aug->r161[i];
		}
		
		/* If this is a dark frame, average with the previous stored one(s) */
		
		if (aug->dark.Capture)
			aug->dark.dk161[i] = (aug->dark.dk161[i] * ndark + val)/(ndark + 1);		
	
		/* Subtract dark frame, if required */		

		if (aug->dark.Subtract)
			val = MAX (val - aug->dark.dk161[i], 0);
	
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
										 aug->r161[row * aug->exd.h_pix + col]);
			aug->img.max[GREY].val = MAX (aug->img.max[GREY].val, 
										 aug->r161[row * aug->exd.h_pix + col]);
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
		 
		if (aug->device == UNICAP || aug->device == V4L) {
			for (k = 0; k < 3; k++)
				aug->disp083[i*3 + k] = val;
		} else if (aug->device == SX || aug->device == SX_GH) {
			if (aug->imdisp.gamma >= 0.995) { /* Gamma value essentially == 1 */
				for (k = 0; k < 3; k++)
					aug->disp083[i*3 + k] = val >> (aug->cam_cap.bitspp - 8);
			} else {
				for (k = 0; k < 3; k++)
					aug->disp083[i*3 + k] = (guchar) 255 * 
								     pow (val, aug->imdisp.gamma) / gamma_scale;
			}
		}
		aug->ff161[i] = val;                            /* For centroiding    */
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
	 * Atempt to reject stars as far as possible by ignoring values that
	 * deviate from the median by more than 50%.
	 */
	
	rms = 0.0;
	reject = 0;
	for (i = yo1; i <= yo2; i++) {
		for (j = xo1; j <= xo2; j++) {	
			val = aug->ff161[aug->exd.h_pix * i + j];
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
