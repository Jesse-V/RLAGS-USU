/******************************************************************************/
/*              VIDEO RECORDING/PLAYBACK AND PHOTOMETRY ROUTINES              */
/*                                                                            */
/* All the routines for recording/playing back video are contained in this    */
/* module.  Also contains the routines for video photometry.                  */
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
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define GOQAT_VIDEO
#include "interface.h"

#define VID_BLACK     0          /* Black level                               */
#define VID_WHITE   255          /* White level                               */

#define ID "<GQVID>"             /* GoQat video file 'magic' identifier       */
#define FRAME_HEADER_SIZE 100    /* 100 bytes header for each video frame     */

#ifdef HAVE___AUTOGEN_SH
#undef GOQAT_SEXTRACTOR_PL
#define GOQAT_SEXTRACTOR_PL "../data/GoQat_SExtractor.pl"
#undef GOQAT_SEXTRACTOR_CONF
#define GOQAT_SEXTRACTOR_CONF "../data/goqat_vid_photom_sextractor.conf"
#undef GOQAT_SEXTRACTOR_PARAM
#define GOQAT_SEXTRACTOR_PARAM "../data/goqat_vid_photom_sextractor.param"
#undef GOQAT_SEXTRACTOR_FILTER
#define GOQAT_SEXTRACTOR_FILTER "../data/goqat_vid_photom_sextractor_filter.conv"
#endif

enum VideoActions {              /* Possible actions for video_iter_frames    */
	VA_NONE = 0,
	VA_PB = 1,                   /* Play back video frames                    */
	VA_FT = 2,                   /* Set frame time stamps                     */
	VA_SF = 4,                   /* Save frames as FITS files                 */
	VA_SV = 8,                   /* Save frames in GoQat video format         */
	VA_PS = 16,                  /* Perform photometry on single frame        */
};

struct video_header {            /* Header for entire video file              */
	gchar id[10];
	gushort h;
	gushort v;
	gchar comment[80];
};

struct frame_times {             /* Time values (absolute or increments)      */
	gint sec;                    /* Seconds                                   */
	gint usec;                   /* Microseconds                              */
};

static struct rec_play_buffer {  /* Record/playback buffer                    */
	FILE *fp;                    /* File pointer for video file               */
	struct video_header vh;      /* File header for video file                */
	struct timeval tv;           /* Timeval (time stamp) for current frame    */
	struct frame_times first_time;/* Absolute time for first frame of sequence*/
	struct frame_times incr_time;/* Time increment between 1st & 2nd frames   */
	gushort fps;                 /* Frame rate in fps                         */
	gushort buf;                 /* Buffer counter (0 or 1)                   */
	gushort nbf;                 /* Number of frames per buffer               */
	gushort h;                   /* Horizontal size of video frame            */
	gushort v;                   /* Vertical size of video frame              */
	guint hd_size;               /* Size of header for each video frame       */
	guint fr_size;               /* Video frame size (h * v)                  */
	guint num_frames;            /* Number of buffered frames/frames in file  */
	guint start;                 /* Start time of recording/video playback    */
	guint latest;                /* Time of most recent buffered frame        */
	guint cur_frame;             /* Current frame number for playback sequence*/
	guint init_frame;            /* Initial frame number for playback sequence*/
    guint disp_frame;            /* Currently displayed frame number          */
	guint mark_first;            /* First marked frame                        */
	guint mark_last;             /* Last marked frame                         */
	guint mark_step;             /* Increment for marked frames               */
	guchar *vid_buf;             /* Buffer pointer for video data             */
	guchar *vid_bufptr[2];       /* Pointers into vid_buf area for next frame */
	gchar *comment;              /* Video file comment                        */
	gchar *savefile;             /* Name of video/FITS file to be saved       */
	gchar *vidfile;              /* Name of video file opened for playback    */
	gboolean FirstFrame;         /* TRUE if this is the first recorded frame  */
	gboolean vid_buf_flushed[2]; /* Flags to indicate if buffers are flushed  */
} rp;

static struct photom {           /* Photometry parameters for video images    */
	gfloat minarea;              /* SExtractor DETECT_MINAREA                 */
	gfloat thresh;               /* SExtractor DETECT_THRESH                  */
	gfloat aperture;             /* SExtractor PHOT_APERTURES                 */
	gfloat shift;                /* Max. shift in star position between frames*/
	gchar *type;                 /* "Single" to do photometry on the current  */
                                 /*  frame, "InitRange" to initialise         */
                                 /*  photometry on a range of frames, "Range" */
                                 /*  to do photometry on a frame within a     */
                                 /*  range of frames.                         */
} phot;

static struct cam_img vid_cam_obj, *vid;
gushort video_action = VA_NONE;
static gchar *GOQAT_SEXTRACTOR_RESULTS = NULL;

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void video_init (void);
#ifdef HAVE_UNICAP
gboolean video_record_start (gchar *dirname);
void video_record_stop (void);
void video_buffer_frame (guchar *buffer, struct timeval *fill_time, guint now);
void video_flush_buffer (gushort buf);
#endif
gboolean video_open_file (gchar *file);
gint video_get_frame_number (void);
void video_set_start_time (void);
void video_set_frame_rate (gushort fps);
gboolean video_show_frame (guint frame_num);
gboolean video_update_timestamp (gchar *stamp);
void video_set_timestamps (void);
void video_play_frames (gboolean Play);
void video_show_prev (void);
void video_show_next (void);
void video_close_file (void);
gboolean video_save_frames (gchar *file, gint filetype, gboolean Range);
gboolean video_iter_frames (gboolean Final);
void video_photom_frames (gfloat aperture, gfloat minarea, gfloat thresh,
						  gfloat shift, gboolean Range);
struct cam_img *get_vid_image_struct (void);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void video_init (void)
{
	/* Initialise video recording/playback (called by the window configure 
	 * event and when the window is "shown").
	 */
	
	vid = &vid_cam_obj;

    vid->canv.cviImage = NULL;
	rp.fp = NULL;
    rp.disp_frame = 0;
	rp.vid_buf = NULL;
	rp.savefile = NULL;
	rp.vidfile = NULL;
}

#ifdef HAVE_UNICAP
gboolean video_record_start (gchar *dirname)
{
	/* Allocate memory and open the file for recording */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gchar *file, *comment;
	
	/* Close any previously open file and free memory */
	
	video_close_file ();

	/* Initialise buffer structure */
	
	rp.fp = NULL;
	rp.vid_buf = NULL;
	rp.buf = 0;
	rp.nbf = get_video_framebufsize ();
	rp.hd_size = FRAME_HEADER_SIZE;  /* Space for each frame header */
	rp.fr_size = aug->exd.h_pix * aug->exd.v_pix * sizeof (guchar);
	rp.num_frames = 0;
	rp.FirstFrame = TRUE;
	rp.vid_buf_flushed[0] = TRUE;
	rp.vid_buf_flushed[1] = TRUE;
	
	/* Allocate memory */
	
	if (!(rp.vid_buf = (guchar *) g_malloc0 (2 * rp.nbf * 
											(rp.hd_size + rp.fr_size)))) {
		aug->Record = FALSE;
		return show_error (__func__, "Can't allocate buffer for recording!");
	}
	rp.vid_bufptr[0] = rp.vid_buf;
	rp.vid_bufptr[1] = rp.vid_buf + rp.nbf * (rp.hd_size + rp.fr_size);
	
	/* Open the file */
	
	set_fits_data (aug, NULL, FALSE, FALSE);  /* Get date/time in handy format*/
	file = g_strconcat (dirname, "/", aug->fits.date_obs, ".vid", NULL);
	if (!(rp.fp = fopen (file, "wb"))) {
		g_free (file);
		aug->Record = FALSE;
		return show_error (__func__, "Can't open video file for recording!");
	}
	L_print ("{b}Opened %s for video recording...\n", file);
	g_free (file);
	
	/* Write header info */
	
	strcpy (rp.vh.id, ID);
	rp.vh.h = aug->exd.h_pix;
	rp.vh.v = aug->exd.v_pix;
	comment = get_entry_string ("txtLVComment");
	strcpy (rp.vh.comment, comment);
	g_free (comment);
	if ((fwrite (&rp.vh, sizeof (struct video_header), 1, rp.fp) != 1)) {
		aug->Record = FALSE;
		return show_error (__func__, "Can't write video file header!");
	}
	return TRUE;
}
#endif

#ifdef HAVE_UNICAP
void video_record_stop (void)
{
    /* Free memory, close recording file and display statistics */

	gdouble seconds;
	
	video_flush_buffer (rp.buf);
	video_close_file ();
	
	seconds = (gdouble) (rp.latest - rp.start) / 1000.0;
	L_print ("{b}Buffered %d frames in %8.3f seconds at %6.3f fps\n",
			 rp.num_frames,
			 seconds, 
			 (gfloat) rp.num_frames / seconds);
	L_print ("{b}Expected %d frames at 25.000 fps\n",(guint) (seconds * 25.0));
	L_print ("{b}Expected %d frames at 30.000 fps\n",(guint) (seconds * 30.0));
	L_print ("{b}Expected %d frames at 60.000 fps\n",(guint) (seconds * 60.0));
}
#endif

#ifdef HAVE_UNICAP
void video_buffer_frame (guchar *buffer, struct timeval *fill_time, guint now)
{
	/* Buffer every 'save_every' video frames */
	
	/*struct cam_img *aug = get_aug_image_struct ();*/
		
	static gushort f_num = 0;
	static gint save_every = 0;
	static gint save_count = 0;
	
	if (!rp.vid_buf)
		return;
	
	if (rp.FirstFrame) {
		f_num = 0;
		save_count = 0;
		get_entry_int ("txtLVSaveFrames", 1, 99999, 1, NO_PAGE, &save_every);
		rp.start = now;
		rp.FirstFrame = FALSE;
		return;
	}
	
	if (++save_count < save_every)
		return;
	else
		save_count = 0;
	
	/* Increment frame counter and set latest time */
	
	rp.num_frames++;
	rp.latest = now;
	
	/* Buffer the memory */
	
	memcpy (rp.vid_bufptr[rp.buf], fill_time, sizeof (struct timeval));
	rp.vid_bufptr[rp.buf] += rp.hd_size;
	memcpy (rp.vid_bufptr[rp.buf], buffer, rp.fr_size);
	rp.vid_bufptr[rp.buf] += rp.fr_size;
	
	/* Flush to disk and swap buffers every rp.nbf frames */
	
	if (++f_num == rp.nbf) {
		f_num = 0;
		loop_LiveView_flush_to_disk (rp.buf);  /* Flush this buffer       */
		rp.buf = (rp.buf == 0) ? 1 : 0;        /* Swap buffers            */
		if (rp.vid_buf_flushed[rp.buf]) {      /* Check it's been flushed */
			rp.vid_bufptr[rp.buf] = rp.vid_buf + rp.buf * rp.nbf * 
			                                          (rp.hd_size + rp.fr_size);
			rp.vid_buf_flushed[rp.buf] = FALSE;
		} else {
			L_print ("{r}Disk writes too slow for video recording!\n");
			/*aug->Record = FALSE;*/
			/*return;*/
		}
	}
}
#endif

#ifdef HAVE_UNICAP
void video_flush_buffer (gushort buf)
{
	/* Flush the video buffer to disk */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gushort frames_to_buffer;
    guchar *bufptr;
	
	if (!rp.vid_buf)
		return;
	
	bufptr = rp.vid_buf + buf * rp.nbf * (rp.hd_size + rp.fr_size);
	
	if (!(frames_to_buffer = rp.num_frames%rp.nbf))
		frames_to_buffer = rp.nbf;
	
	if (rp.fp) {
		if ((fwrite (bufptr, (rp.hd_size + rp.fr_size) * sizeof (guchar), 
				                frames_to_buffer, rp.fp) != frames_to_buffer)) {
		    L_print ("{r}Error flushing recording buffer to disk!\n");
			aug->Record = FALSE;
		} else
	        rp.vid_buf_flushed[buf] = TRUE;
	}
}
#endif

gboolean video_open_file (gchar *file)
{
	/* Open the video file, get the header information and display the first
	 * frame.
	 */
	
	struct stat fil;
    gint i, j;
    guchar *disp, *val;
	
	/* Close any previously open file and free memory */
	
	video_close_file ();
	
	/* Initialise buffer structure */
	
	rp.fp = NULL;
	rp.vid_buf = NULL;
	rp.cur_frame = 1;
    rp.disp_frame = 0;
	
	/* Open the file */

	if (!(rp.fp = fopen (file, "r+b")))
		return show_error (__func__, "Can't open video file for playback!");
	
	/* Get the video file header information */
	
	if (fread (&rp.vh, sizeof (struct video_header), 1, rp.fp) != 1) {
        video_close_file ();
		return show_error (__func__, "Error reading header information from "
						             "video file");
    }
	
	/* Check that this is a valid video file */
	
	if (strcmp (rp.vh.id, ID)) {
		msg ("Warning - Not a valid video file!");
		video_close_file ();
		return FALSE;
	}
	
	rp.h = rp.vh.h;
	rp.v = rp.vh.v;
	rp.hd_size = FRAME_HEADER_SIZE;
	rp.fr_size = rp.h * rp.v;
	rp.comment = rp.vh.comment;
	set_entry_string ("txtPBComment", rp.comment);
	rp.vidfile = g_strdup (file);
    GOQAT_SEXTRACTOR_RESULTS = g_strconcat (file, "_catalog.txt", NULL);
	
	/* Initialise remaining values */
	
	vid->exd.h_top_l = 0;
	vid->exd.v_top_l = 0;
	vid->exd.h_bot_r = rp.h - 1;
	vid->exd.v_bot_r = rp.v - 1;
	vid->exd.h_bin = 1;
	vid->exd.v_bin = 1;
	vid->exd.h_pix = rp.h;
	vid->exd.v_pix = rp.v;
	vid->canv.r.htl = 0;
	vid->canv.r.vtl = 0;
	vid->canv.r.hbr = rp.h - 1;
	vid->canv.r.vbr = rp.v - 1;
	vid->canv.NewRect = FALSE;
	vid->vadr = NULL;
    vid->disp_8_3 = NULL;
    
	/* Allocate buffer memory, read and display the first frame */
	
	if (!(vid->disp_8_3 = (guchar *) g_malloc0 (vid->exd.h_pix * 
	                                   vid->exd.v_pix * 3 * sizeof (guchar)))) {
        video_close_file ();
		return show_error (__func__, "Unable to allocate buffer memory");
	}
    
	if (!(rp.vid_buf = (guchar *) g_malloc0 ((rp.hd_size + rp.fr_size) * 
                                                            sizeof (guchar)))) {
        video_close_file ();
		return show_error (__func__, "Unable to allocate buffer memory");
    }
	
	if (fread (rp.vid_buf, (rp.hd_size + rp.fr_size) * sizeof (guchar), 1, 
                                                                  rp.fp) != 1) {
        video_close_file ();
		return show_error (__func__, "Error reading frame from video file");
    }
        
	memcpy (&rp.tv, rp.vid_buf, sizeof (struct timeval));
	set_fits_data (vid, &rp.tv, TRUE, FALSE);
    disp = vid->disp_8_3;
    val = rp.vid_buf + rp.hd_size;
    for (i = 0; i < rp.fr_size; i++) { /* Expand data to 3 x 8 bits per pixel */
        for (j = 0; j < 3; j++)
			*disp++ = *val;
        val++;
    }
	ui_show_video_frame (rp.vid_buf + rp.hd_size, vid->fits.date_obs, 
					                                  rp.cur_frame, rp.h, rp.v);
                                                      
	/* Calculate and display number of frames in file */
	
	stat (file, &fil);
	rp.num_frames = (fil.st_size - sizeof (struct video_header)) / 
	                                                  (rp.hd_size + rp.fr_size);
	
	L_print ("{b}Opened video file %s: contains %d frames of size %dx%d\n", 
			                                   file, rp.num_frames, rp.h, rp.v);
	L_print("{b}Comment: %s\n", get_entry_string ("txtPBComment"));
	set_video_range_adjustment (rp.num_frames);
	vid->Open = TRUE;
	
	return TRUE;
}

gint video_get_frame_number (void)
{
	/* Return the current frame number */
	
	if (rp.fp)
		return (gint) rp.cur_frame;
	else
		return 0;
}

void video_set_start_time (void)
{
	/* Set the start time for playback.  This is modified whenever the user
	 * clicks the 'Play' button or alters the frame rate.
	 */
	
	rp.start = loop_elapsed_since_first_iteration ();
	rp.init_frame = rp.cur_frame;
}

void video_set_frame_rate (gushort fps)
{
	/* Set the playback rate in fps */
	
	rp.fps = fps;
}

gboolean video_show_frame (guint frame_num)
{
	/* Read and show the requested video frame */
	
    gint i, j;
    guchar *disp, *val;
    
	if (frame_num < 1 || frame_num > rp.num_frames) {
		loop_video_iter_frames (FALSE);
		return FALSE;
	}
	
	if (!rp.fp) {
		loop_video_iter_frames (FALSE);
		return FALSE;
	}
	
	if (!rp.vid_buf) {
		loop_video_iter_frames (FALSE);
		return FALSE;
	}
    
    if (frame_num == rp.disp_frame)
        return TRUE;
    
	if (frame_num != rp.cur_frame + 1) {  /* frame no. is not the next frame */
		if (fseeko (rp.fp, (off_t) (sizeof (struct video_header) + 
		    ((off_t) (frame_num - 1) * (rp.hd_size + rp.fr_size))), SEEK_SET)) {
			perror ("fseeko");
			loop_video_iter_frames (FALSE);
			return show_error (__func__, "Error seeking frame in video file");
		}
	}
		
	if (fread (rp.vid_buf, (rp.hd_size + rp.fr_size) * sizeof (guchar), 1, 
			                                                      rp.fp) != 1) {
		loop_video_iter_frames (FALSE);
		return show_error (__func__, "Error reading frame from video file");
	}
    
	memcpy (&rp.tv, rp.vid_buf, sizeof (struct timeval));
	set_fits_data (vid, &rp.tv, TRUE, FALSE);
    disp = vid->disp_8_3;
    val = rp.vid_buf + rp.hd_size;
    for (i = 0; i < rp.fr_size; i++) { /* Expand data to 3 x 8 bits per pixel */
        for (j = 0; j < 3; j++)
			*disp++ = *val;
        val++;
    }
    ui_show_video_frame (rp.vid_buf + rp.hd_size, vid->fits.date_obs, 
					                                     frame_num, rp.h, rp.v);
    
    rp.disp_frame = frame_num;
    rp.cur_frame = frame_num;                                                     
	set_video_range_value (frame_num);
	
	return TRUE;
}

gboolean video_update_timestamp (gchar *stamp)
{
	/* Update the time stamp of the current frame (always interpreted as UTC) */
	
	struct tm dt;
	GDate *now, *epoch;
		
	if (!rp.fp)
		return FALSE;

	strptime (stamp, "%Y-%m-%dT%T", &dt);
	now = g_date_new_dmy (dt.tm_mday, 1 + dt.tm_mon, 1900 + dt.tm_year);
	epoch = g_date_new_dmy (1, 1, 1970);
	rp.tv.tv_sec = (time_t) (g_date_days_between (epoch, now) * 86400 + 
                                dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec);
	rp.tv.tv_usec = (suseconds_t)(1.e3 * strtol (stamp+20, (gchar **) NULL,10));
	g_date_free (epoch);
	g_date_free (now);

	/* Seek to beginning of current frame */
	
	if (fseeko (rp.fp, (off_t) (sizeof (struct video_header) + 
		 ((off_t) (rp.cur_frame - 1) * (rp.hd_size + rp.fr_size))), SEEK_SET)) {
		perror ("fseeko");
		return show_error (__func__, "Error seeking frame in video file");
	}
	
	/* Write the timeval */	
	
	if (!fwrite (&rp.tv, sizeof (struct timeval), 1, rp.fp))
		return show_error (__func__, "Error updating time stamp");
	
	/* Reset file postion indicator to its previous value */
	
	if (fseeko (rp.fp, (off_t) (rp.hd_size + rp.fr_size), SEEK_CUR)) {
		perror ("fseeko");
		return show_error (__func__, "Error seeking frame in video file");
	}
	
	return TRUE;
}

void video_set_timestamps (void)
{
	/* Set the time stamps within the selected range.  The increment is
	 * defined to be the difference between the first and second time stamps in
	 * the selected range; subsequent time stamps are set so that each differs
	 * from the previous by this increment.
	 */
	
	gint first, last;
	
	/* Get the selected range */
	
	if (!get_entry_int ("txtPBFirst", 1, rp.num_frames, 0, NO_PAGE, &first) ||
	    !get_entry_int ("txtPBLast", first + 1,rp.num_frames,0,NO_PAGE,&last)) {
		L_print ("{o}Please enter valid values for the first and last "
				                                                    "frames\n");
		return;
	}
	rp.mark_first = (guint) first;
	rp.mark_last = (guint) last;
	
	/* Get first and second time stamps and calculate increment */
	
	video_show_frame (first);
	rp.first_time.sec = (gint) rp.tv.tv_sec;
	rp.first_time.usec = (gint) rp.tv.tv_usec;
	video_show_frame (first + 1);
	rp.incr_time.sec = (gint) rp.tv.tv_sec - rp.first_time.sec;
	rp.incr_time.usec = (gint) rp.tv.tv_usec - rp.first_time.usec;
	
	if (rp.incr_time.usec < 0) {
		rp.incr_time.sec--;
		rp.incr_time.usec += 1e6;
	}
	
	rp.cur_frame++;/* This is a bodge so that the loop code in                */
	               /* video_iter_frames works as expected i.e. although THIS  */
	               /* frame has been read and is being displayed, the file    */
	               /* pointer is positioned at the beginning of the NEXT frame*/
	
	video_action |= VA_FT;
	loop_video_iter_frames (TRUE);
}

void video_play_frames (gboolean Play)
{
	/* Start/stop playing video frames */
	
	if (Play) {
		video_action |= VA_PB;
		video_set_start_time ();
		loop_video_iter_frames (TRUE);
	} else
		loop_video_iter_frames (FALSE);
}

void video_show_prev (void)
{
	/* Show previous frame */
	
    if (rp.cur_frame > 1)
        video_show_frame (--rp.cur_frame);
}

void video_show_next (void)
{
	/* Show next frame */
	
	video_show_frame (++rp.cur_frame);
}

void video_close_file (void)
{
	/* Close the video file and release memory */
	
	if (rp.fp) {
		fclose (rp.fp);
		rp.fp = NULL;
	}
    
	if (vid->disp_8_3) {
		g_free (vid->disp_8_3);
		vid->disp_8_3 = NULL;
	}
	
	if (rp.vid_buf) {
		g_free (rp.vid_buf);
		rp.vid_buf = NULL;
	}
	
	if (rp.vidfile) {
		g_free (rp.vidfile);
		rp.vidfile = NULL;
	}
	
	if (GOQAT_SEXTRACTOR_RESULTS) {
		g_free (GOQAT_SEXTRACTOR_RESULTS);
		GOQAT_SEXTRACTOR_RESULTS = NULL;
	}
	
	vid->Open = FALSE;
}

gboolean video_save_frames (gchar *file, gint filetype, gboolean Range)
{
	/* Save the requested range of video frames as FITS files or in VID format.
	 * If Range is FALSE, then save just the current frame.
	 */
	
	FILE *fp;
	gint first, last, save_every;
	
	/* Check that there's an open video file */
	
	if (!rp.fp) {
		msg ("Warning - No video frames to save!");
		return FALSE;
	}
	
	if (Range) {
	
		/* Get the range to save */
	
		if (!get_entry_int("txtPBFirst", 1, rp.num_frames, 0, NO_PAGE, &first)||
	        !get_entry_int("txtPBLast", first, rp.num_frames, 0,NO_PAGE,&last)){
		    L_print ("{o}Please enter valid values for the first and last "
				                                                    "frames\n");
		    return FALSE;
	    }
	    rp.mark_first = (guint) first;
	    rp.mark_last = (guint) last;
	    get_entry_int ("txtPBSaveFrames", 1, 99999, 1, NO_PAGE, &save_every);
	    rp.mark_step = (guint) save_every;
	    rp.cur_frame = rp.mark_first;
	
    } else {
		
		/* Set range to current frame */
		
		rp.mark_first = rp.cur_frame;
		rp.mark_last = rp.cur_frame;
		rp.mark_step = 1;		
	}

	/* Save the data */
	
	if (filetype == SVF_FITS) {              /* Can save subset of each frame */
		
		/* Initialise the data structure */
	
		vid->id = VID;
		vid->imdisp.W = VID_WHITE;
		vid->imdisp.B = VID_BLACK;
		vid->fits.tm_start = 0.0;
		vid->fits.epoch = 0.0;
		strcpy (vid->fits.date_obs, "");
		strcpy (vid->fits.utstart, "");
		strcpy (vid->fits.RA, "");
		strcpy (vid->fits.Dec, "");
		vid->exd.req_len = 0;
		
		if (!(vid->vadr = (gushort *) g_malloc0 (vid->exd.h_pix * 
                                                 vid->exd.v_pix * 
                                                 sizeof (gushort))))
			return show_error (__func__, "Unable to allocate buffer memory "
							                              "for writing frames");
		
		/* Set the base file name to be used for saving individual frames in
		 * video_iter_frames.
		 */
		
		if (file)				
			rp.savefile = g_strdup (file);
		else
			rp.savefile = g_strdup (rp.vidfile);
		video_action |= VA_SF;
	} else if (filetype == SVF_VID) {        /* Saves full frame only */
		
		/* Set the file name to be used for writing the video header in this
		 * routine, and for writing each frame in video_iter_frames.
		 */
		
		rp.savefile = g_strconcat (file, ".vid", NULL);
		
		/* Open video file and write header */
		
	    if (!(fp = fopen (rp.savefile, "wb"))) {
			g_free (rp.savefile);
			rp.savefile = NULL;
			return show_error (__func__, "Can't save to video file!");
		}
		if (!fwrite (&rp.vh, sizeof (struct video_header), 1, fp)) {
			fclose (fp);
			g_free (rp.savefile);
			rp.savefile = NULL;
			return show_error (__func__, "Error writing video header");
		}
		fclose (fp);
		video_action |= VA_SV;
	}
	loop_video_iter_frames (TRUE);
	
	return TRUE;
}

gboolean video_iter_frames (gboolean Final)
{
	/* Iterate over selected frames performing desired action until done.  Note
	 * that this routine is called repeatedly by the main event loop code until
     * loop_video_iter_frames (FALSE) is called from here or elsewhere.  This is
     * because the selected range of frames is processed in batches rather than
     * all in one go, thus permitting the user to cancel the process before the 
     * end of the range is reached.
	 */
	
	static FILE *fp = NULL;
	gushort xo1, xo2, yo1, yo2;
	guint next;
	gint j, r, c, sec, usec;
	gint exit;
	static gchar *filename = NULL;
	gchar *strnum;
	gchar *s = NULL;
	
	if (!rp.fp) {
		loop_video_iter_frames (FALSE);
		video_action = VA_NONE;
		return FALSE;
	}
	
	if (video_action & VA_PB) {                  /* Play back video */
		if (Final) {
			video_action = VA_NONE;
			return TRUE;
		}
		
		/* Play the next video frame according to the amount of time that 
		 * has elapsed since playback started.  This routine is called by 
		 * the main loop timer once every 25ms in theory, but probably less 
		 * than this in practice.
		 */

		rp.cur_frame = rp.init_frame + (guint) (0.5 + rp.fps * (gdouble) 
			       (loop_elapsed_since_first_iteration () - rp.start) / 1000.0);
		video_show_frame (rp.cur_frame);  /* video_show_frame calls         */
		    			                  /* loop_video_iter_frames (FALSE) */
									      /* when end of video is reached   */

	} else if (video_action & VA_FT) {           /* Set frame time stamps */
		if (Final) {
			video_show_frame (rp.cur_frame);
			L_print ("Set time stamps for frames %d to %d\n", 
											       rp.mark_first, rp.mark_last);
			video_action = VA_NONE;
			return TRUE;
		}
		
		/* Set frame time stamps according to time stamp of first frame
		 * and the increment between the first and second frames.  Do this
		 * in batches of 100 frames, allowing the user to cancel between 
		 * batches.
		 */
		
		next = rp.cur_frame;
		for (rp.cur_frame = next; rp.cur_frame < next + 100; rp.cur_frame++) {
												 
			if (rp.cur_frame > rp.mark_last) {   /* Got to end of sequence */
				rp.cur_frame--;
				loop_video_iter_frames (FALSE);
				break;
			}
			
			/* Calculate times for current frame */
			
			sec = rp.first_time.sec + (rp.cur_frame - rp.mark_first) * 
														       rp.incr_time.sec;
			usec = rp.first_time.usec + (rp.cur_frame - rp.mark_first) * 
														      rp.incr_time.usec;
			
			sec += (gint) (usec / 1000000);
			usec = usec % 1000000;
			
			rp.tv.tv_sec = (time_t) sec;
			rp.tv.tv_usec = (suseconds_t) usec;
			
			/* Write the timeval */	

			if (!fwrite (&rp.tv, sizeof (struct timeval), 1, rp.fp))
				return show_error (__func__, "Error updating time stamp");
			
			/* Seek to beginning of next frame */

			if (rp.cur_frame < rp.mark_last)
				if (fseeko (rp.fp, (off_t) (rp.hd_size + rp.fr_size - 
									      sizeof (struct timeval)), SEEK_CUR)) {
					perror ("fseeko");
					return show_error (__func__, "Error seeking frame in "
															      "video file");
				}
		}
			
	} else if (video_action & VA_SF) {           /* Save frames as FITS files */
		if (Final) {
            video_show_frame (rp.cur_frame);
			if (video_action & VA_PS) {
				s = g_strdup_printf("%s --%s "
									"--hoffset %d "
									"--voffset %d "
									"--vheight %d "
									"--timestamp %f "
									"--maxshift %f "
                                    "--results_catalog %s " 
									"--sextractor %s -c %s "
									"-CATALOG_NAME %s "
									"-PARAMETERS_NAME %s "
									"-DETECT_MINAREA %f "
									"-DETECT_THRESH %f "
									"-FILTER_NAME %s "
									"-PHOT_APERTURES %f ",
									GOQAT_SEXTRACTOR_PL,
									"Final",
									vid->canv.r.htl,
									vid->canv.r.vtl,
									vid->exd.v_pix,
									vid->fits.tm_start,
									phot.shift,
									GOQAT_SEXTRACTOR_RESULTS,
									rp.savefile,
									GOQAT_SEXTRACTOR_CONF,
                                    rp.vidfile,
									GOQAT_SEXTRACTOR_PARAM,
									phot.minarea,
									phot.thresh,
									GOQAT_SEXTRACTOR_FILTER,
									phot.aperture);
				g_spawn_command_line_sync (s, NULL, NULL, &exit, NULL);
				g_free (s);
				ui_show_photom_points (GOQAT_SEXTRACTOR_RESULTS, phot.aperture);
			}
            vid->exd.h_pix = rp.h;
            vid->exd.v_pix = rp.v;
            
			if (filename) {
				g_free (filename);
				filename = NULL;
			}
			if (rp.savefile) {
				g_free (rp.savefile);
				rp.savefile = NULL;
			}
			if (vid->vadr) {
				g_free (vid->vadr);
				vid->vadr = NULL;
			}
			video_action = VA_NONE;
			return TRUE;
		}

		/* Save selected frames as FITS files in batches of 10, allowing the
		 * user to cancel between batches.  Note that vadr is gushort and  
		 * vid_buf is guchar, so we can't do a memcpy.  Perform photometry on
		 * saved frames if requested.
		 */

		if (!filename && rp.savefile) {         /* Save base part of filename */
			filename = g_strdup (rp.savefile); 
			g_free (rp.savefile);
			rp.savefile = NULL;
		}
			
		next = rp.cur_frame;
		for (rp.cur_frame = next; rp.cur_frame < next + 10 * rp.mark_step; 
											     rp.cur_frame += rp.mark_step) {
												 
			if (rp.cur_frame > rp.mark_last) {  /* Got to end of sequence */
				rp.cur_frame -= rp.mark_step;
				loop_video_iter_frames (FALSE);
				break;
			}
			
			if (is_in_image (vid, &xo1, &yo1, &xo2, &yo2)) {
				vid->exd.h_pix = xo2 - xo1 + 1;/*reset to rp.h on 'Final' call*/
				vid->exd.v_pix = yo2 - yo1 + 1;/*reset to rp.v on 'Final' call*/
				strnum = g_strdup_printf ("%06i", rp.cur_frame);
				rp.savefile = g_strconcat (filename, "_", strnum, ".fit", NULL);
				g_free (strnum);
				video_show_frame (rp.cur_frame);
				for (j = 0, r = yo1; r <= yo2; r++)
					for (c = xo1; c <= xo2; c++)
						vid->vadr[j++] =rp.vid_buf[(r * rp.h) + c + rp.hd_size];
                image_save_as_fits (vid, rp.savefile, GREY, FALSE);
                if (!(video_action & VA_PS))
                    L_print ("Saved %s\n", rp.savefile);
                if (video_action & VA_PS) {   /* Performing photometry... */
                    s = g_strdup_printf("%s --%s "
                                        "--hoffset %d "
                                        "--voffset %d "
                                        "--vheight %d "
                                        "--timestamp %f "
                                        "--maxshift %f "
                                        "--results_catalog %s " 
                                        "--sextractor %s -c %s "
                                        "-CATALOG_NAME %s "
                                        "-PARAMETERS_NAME %s "
                                        "-DETECT_MINAREA %f "
                                        "-DETECT_THRESH %f "
                                        "-FILTER_NAME %s "
                                        "-PHOT_APERTURES %f ", 
                                        GOQAT_SEXTRACTOR_PL,
                                        phot.type,
                                        vid->canv.r.htl,
                                        vid->canv.r.vtl,
                                        vid->exd.v_pix,
                                        vid->fits.tm_start,
                                        phot.shift,
                                        GOQAT_SEXTRACTOR_RESULTS,
                                        rp.savefile,
                                        GOQAT_SEXTRACTOR_CONF,
                                        rp.vidfile,
                                        GOQAT_SEXTRACTOR_PARAM,
                                        phot.minarea,
                                        phot.thresh,
                                        GOQAT_SEXTRACTOR_FILTER,
                                        phot.aperture);
                    g_spawn_command_line_sync (s, NULL, NULL, &exit, NULL);
                    g_free (s);
                    /* If called with phot.type = "InitRange", this must next */
                    /*  be called with phot.type = "Range".  If it was called */
                    /*  with phot.type = "Single", then it will be called only*/
                    /*  once anyway, so either way we can just set phot.type  */
                    /*  to "Range" here.                                      */
                    phot.type = "Range";
                }
			} else {
				L_print ("{o}No video frame within selection rectangle!\n");
				loop_video_iter_frames (FALSE);
				return FALSE;
			}
		}
		
	} else if (video_action & VA_SV) {          /* Save frames as VID file */
		if (Final) {
			video_show_frame (rp.cur_frame);
			if (fp) {
				fclose (fp);
				fp = NULL;
			}
			if (rp.savefile) {
				L_print ("Saved %s\n", rp.savefile);
				g_free (rp.savefile);
				rp.savefile = NULL;
			}
			video_action = VA_NONE;
			return TRUE;
		}
		
		/* Save selected frames as VID file in batches of 100, allowing the 
		 * user to cancel between batches.
		 */

		if (!(fp = fopen (rp.savefile, "ab"))) {
			loop_video_iter_frames (FALSE);
			return show_error (__func__, "Can't save to video file!");
		}
		video_show_frame (rp.cur_frame);
		next = rp.cur_frame;
		for (rp.cur_frame = next; rp.cur_frame < next + 100; rp.cur_frame++) {  
												 
			if (rp.cur_frame > rp.mark_last) {  /* Got to end of sequence */
				rp.cur_frame--;
				loop_video_iter_frames (FALSE);
				break;
			}
			
			/* This is potentially quite inefficient, but buffering the
			 * data to read and write bigger chunks in one go didn't seem
			 * to make much difference.  Probably the operating system does that
             * anyway.
			 */
			
			if (!fwrite (rp.vid_buf, (rp.hd_size + rp.fr_size) *
												  sizeof (guchar), 1, fp)) {
				loop_video_iter_frames (FALSE);
				return show_error (__func__, "Error writing video frame");
			}
			if (rp.cur_frame < rp.mark_last)
				if (!fread (rp.vid_buf, (rp.hd_size + rp.fr_size) *
											   sizeof (guchar), 1, rp.fp)) {
				loop_video_iter_frames (FALSE);
				return show_error (__func__, "Error reading video frame");
			}
			
		}
		fclose (fp);
		fp = NULL;

	}
	return TRUE;
}

void video_photom_frames (gfloat aperture, gfloat minarea, gfloat thresh,
						  gfloat shift, gboolean Range)
{
	/* Perform photometry on video frame(s) */
	
	/* Save photometry parameters */
	
	phot.minarea = minarea;
	phot.thresh = thresh;
	phot.aperture = aperture;
	phot.shift = shift;
	phot.type = Range ? "InitRange" : "Single";
	
	/* Set flag to perform photometry after saving frame */
	
	video_action |= VA_PS;
	
	/* Save frame(s) */
	
	video_save_frames (NULL, SVF_FITS, Range);
}

struct cam_img *get_vid_image_struct (void)
{
	return vid;
}
