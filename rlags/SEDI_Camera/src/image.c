/******************************************************************************/
/*                         IMAGE HANDLING ROUTINES                            */
/*                                                                            */
/* Some generic image related routines are bundled in this file.  This module */
/* also handles all the communication with DS9 via the XPA mechanism, and     */
/* contains the generic Grace plotting routines.                              */
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
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <math.h>
#ifdef HAVE_LIBGRACE_NP
#include <grace_np.h>
#endif
#ifdef HAVE_LIBXPA
#include <xpa.h>
#define NXPA 1
static XPA xpa;
#endif

#define GOQAT_IMAGE
#include "interface.h"

#define DS9_CCD "CCD_Image"          /* DS9 window title for CCD image        */
#define DS9_AUG "Autoguider_Image"   /* DS9 window title for autoguider image */

#ifdef HAVE_LIBGRACE_NP
static gboolean G_Error = FALSE;     /* TRUE if Grace plotting error occurs   */
#endif
#ifdef HAVE_LIBXPA
static gboolean DS9NewFrame = FALSE; /* TRUE if a new frame is created in DS9 */
static gboolean DS9IsGrey = TRUE;    /* TRUE if current DS9 frame is greyscale*/
#endif

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void xpa_open (void);
void xpa_close (void);
static gboolean xpa_check_ds9_open (struct cam_img *img);
gboolean xpa_display_image (struct cam_img *img, enum Colour colour);
gboolean xpa_get_rect_coords (gfloat *x, gfloat *y, gfloat *w, gfloat *h);
static gboolean xpa_messages (gint i, gchar *bufs[], gchar *names[], 
							  gchar *messages[]);
gboolean image_calc_hist_and_flux (void);
gboolean is_in_image (struct cam_img *img, gushort *xoff1, gushort *yoff1,
	                                       gushort *xoff2, gushort *yoff2);
void image_get_stats (struct cam_img *img, enum ColsPix c);
gboolean image_embed_data (struct cam_img *img);
gboolean image_save_as_fits (struct cam_img *img, gchar *savefile, 
	                         enum Colour colour, gboolean display);
#ifdef HAVE_LIBGRACE_NP
gboolean Grace_Open (gchar *plot, gboolean *AlreadyOpen);
void Grace_SetXAxis (gushort graph, gfloat xmin, gfloat xmax);
void Grace_SetYAxis (gushort graph, gfloat ymin, gfloat ymax);
void Grace_XAxisMajorTick (gushort graph, gint tick);
void Grace_YAxisMajorTick (gushort graph, gint tick);
void Grace_PlotPoints (gushort graph, gushort set, gfloat x, gfloat y);
void Grace_ErasePlot (gushort graph, gushort set);
void Grace_SaveToFile (gchar *FileName);
gboolean Grace_Update (void);
static void Grace_Error (const char *msg);
#endif


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void xpa_open (void)
{
	#ifdef HAVE_LIBXPA
	/* Open a persistent xpa struct */
	
	xpa = XPAOpen (NULL);
	#endif
}

void xpa_close (void)
{
	#ifdef HAVE_LIBXPA
	/* Close persistent xpa connections */
	
	XPAClose (xpa);
	
	#endif
}

static gboolean xpa_check_ds9_open (struct cam_img *img)
{
	#ifdef HAVE_LIBXPA
	/* Check that DS9 is running to display an image - if not, open it.
	 * Retry for up to 10 seconds after attempting to open DS9 before bailing
	 * out.
	 */
	
	gint j, got;
	gint lens[NXPA];
	gchar *bufs[NXPA];
	gchar *names[NXPA];
	gchar *messages[NXPA];
	gchar *ds9_id = NULL, *ds9_cmd = NULL;	

	if (img->id != CCD && img->id != AUG)
		return show_error (__func__, "Unrecognised image type");
	
	ds9_id = (img->id == CCD ? (gchar *) DS9_CCD : (gchar *) DS9_AUG);
	
	/* Check if DS9 is there with a request for the zoom factor (anything else
	 * would do just as well).
	 */
	
	got = XPAGet (xpa, ds9_id, "zoom", NULL, bufs, lens, names, messages, NXPA);
	xpa_messages (got, bufs, names, messages);
    
	/* If DS9 is not there, then start it */

	if (!got) {
		ds9_cmd = g_strdup_printf ("ds9 -title %s -port %d", ds9_id,
                                                        img->id == CCD ? 0 : 1);
        if (img->ds9.stream)
            pclose (img->ds9.stream);
        if (!(img->ds9.stream = popen (ds9_cmd, "r"))) {
			perror ("xpa_check_ds9_open: ");
            g_free (ds9_cmd);
            img->ds9.window = NULL;
            return show_error (__func__, "Unable to open DS9 image viewer");
        }
		g_free (ds9_cmd);

		/* Now check for DS9 up to ten times */	
		
		got = 0;
		for (j = 0; j < 10; j++) {
			got = XPAGet (xpa, ds9_id, "zoom", NULL, bufs, lens, names,
																messages, NXPA);
			xpa_messages (got, bufs, names, messages);
			if (!got)    /* DS9 not started yet?  Wait a second and try again */
				sleep (1);
			else {
				img->ds9.window = ds9_id;
				DS9NewFrame = TRUE;
				DS9IsGrey = TRUE;
				return TRUE; /* DS9 started?  Then return */
			}
		}
		img->ds9.window = NULL;
		return show_error (__func__, "DS9 image viewer has not started");		
	} else {  /* In case user had already opened it before starting GoQat */
		img->ds9.window = ds9_id;
		DS9NewFrame = TRUE;
		DS9IsGrey = TRUE;
	}
	#endif
	return TRUE;
}

gboolean xpa_display_image (struct cam_img *img, enum Colour colour)
{
	#ifdef HAVE_LIBXPA
	/* Display the latest CCD or autoguider image in DS9 via the 
	 * XPA mechanism.  This routine is called three times for a
	 * colour image; once each for the R, G and B components.
	 */

	gint i, got, len = 0;
	gint lens[NXPA];
	gchar *pany, *buf = NULL;
	gchar *bufs[NXPA];
	gchar *names[NXPA];
	gchar *messages[NXPA];
	gchar *ds9_id, *ds9_cmd;
	static gdouble DS9_zoom = 0.0, DS9_panx = 0.0, DS9_pany = 0.0;
	
	/* Store the current zoom and pan */
    
	ds9_id = (img->id == CCD ? (gchar *) DS9_CCD : (gchar *) DS9_AUG);
	
	got = XPAGet (xpa, ds9_id, "zoom", NULL, bufs, lens, names, messages, NXPA);
	for (i = 0; i < got; i++) {
		if (messages[i] == NULL) {
			DS9_zoom = strtod (bufs[i], (gchar **) NULL);
			g_free (bufs[i]);
		} else
			L_print ("{r}XPA error: %s (%s)\n", messages[i], names[i]);
		if (names[i])
			g_free (names[i]);
		if (messages[i])
			g_free (messages[i]);
	}
	
	got = XPAGet (xpa, ds9_id, "pan", NULL, bufs, lens, names, messages, NXPA);
	for (i = 0; i < got; i++) {
		if (messages[i] == NULL) {
			DS9_panx = strtod (bufs[i], &pany);
			DS9_pany = strtod (++pany, (gchar **) NULL);
			g_free (bufs[i]);
		} else
			L_print ("{r}XPA error: %s (%s)\n", messages[i], names[i]);
		if (names[i])
			g_free (names[i]);
		if (messages[i])
			g_free (messages[i]);
	}

	/* (Re)open DS9 */
	
	if (!xpa_check_ds9_open (img))
		return FALSE;
	
	/* Create new frame if required */
	
	switch (colour) {
		case GREY:
			if (!DS9IsGrey) {
				
				/* Delete the current frame if it's colour (DS9 starts with a
				 * greyscale frame by default) and create a greyscale one.
				 */
				
				if (!(got = XPASet (xpa, img->ds9.window, "frame delete", NULL, 
								   buf, len, names, messages, NXPA))) {				
					xpa_messages (got, (gchar **) buf, names, messages);
					return show_error (__func__, 
									   "Error deleting current frame in DS9");
				}
				if (!(got = XPASet (xpa, img->ds9.window, "frame new", NULL, 
								   buf, len, names, messages, NXPA))) {
					xpa_messages (got, (gchar **) buf, names, messages);
					return show_error (__func__, 
								   "Error creating new greyscale frame in DS9");
				}
				DS9NewFrame = TRUE;
				DS9IsGrey = TRUE;  /* Current image is greyscale */
			}
			break;
		case R:
			if (DS9IsGrey) {
				
				/* Delete the current frame if it's greyscale (DS9 starts with a
				 * greyscale frame by default) and create a colour one.
				 */
				
				if (!(got = XPASet (xpa, img->ds9.window, "frame delete", NULL, 
								   buf, len, names, messages, NXPA))) {
					xpa_messages (got, (gchar **) buf, names, messages);
					return show_error (__func__, 
									   "Error deleting current frame in DS9");
				}
				if (!(got = XPASet (xpa, img->ds9.window, "frame new rgb", NULL, 
								   buf, len, names, messages, NXPA))) {
					xpa_messages (got, (gchar **) buf, names, messages);
					return show_error (__func__, 
									   "Error creating new RGB frame in DS9");
				}
				DS9NewFrame = TRUE;
				DS9IsGrey = FALSE;  /* Current image is colour */
			}			
			
			/* Set to red channel */
			
			if (!(got = XPASet (xpa, img->ds9.window, "rgb red", NULL, buf, len, 
												      names, messages, NXPA))) {
				xpa_messages (got, (gchar **) buf, names, messages);
				return show_error (__func__,"Error setting red channel in DS9");
			}
			break;
		case G:
			
			/* Set to green channel */
			
			if (!(got = XPASet (xpa, img->ds9.window, "rgb green", NULL,buf,len, 
													  names, messages, NXPA))) {
				xpa_messages (got, (gchar **) buf, names, messages);
				return show_error(__func__,"Error setting green channel in DS9");
			}
			break;
		case B:
			
			/* Set to blue channel */
			
			if (!(got = XPASet (xpa, img->ds9.window, "rgb blue", NULL, buf,len, 
													  names, messages, NXPA))) {
				xpa_messages (got, (gchar **) buf, names, messages);
				return show_error(__func__,"Error setting blue channel in DS9");
			}
			break;
	}
	
	/* Now display the image */
	
	ds9_cmd = g_strdup_printf ("file %s", img->ds9.display);
	if ((got = XPASet (xpa, img->ds9.window, ds9_cmd, NULL, buf, len,  
		                                              names, messages, NXPA))) {
		xpa_messages (got, (gchar **) buf, names, messages);
		g_free (ds9_cmd);
														  
		/* If we need a new frame, restore pan and zoom to previous values, 
		 * unless this is the first time that DS9 has been started (in which 
		 * case zoom == 0.0).
         */
														  
		if (DS9NewFrame) {
			if (img->FullFrame) {
				if (DS9_zoom == 0.0) /* Pan and zoom not initialised yet */
					ds9_cmd = g_strdup_printf ("pan to %d %d", 
							    img->cam_cap.max_h / 2, img->cam_cap.max_v / 2);
				else
					ds9_cmd = g_strdup_printf("pan to %f %f",DS9_panx,DS9_pany);
			} else {
				if (DS9_zoom == 0.0) /* Pan and zoom not initialised yet */
					ds9_cmd = g_strdup_printf ("pan to %d %d", 
							    img->exd.h_pix / 2, img->exd.v_pix / 2);
				else
					ds9_cmd = g_strdup_printf("pan to %f %f",DS9_panx,DS9_pany);
			}
			got = XPASet (xpa, img->ds9.window, ds9_cmd, NULL, buf,len, 
													     names, messages, NXPA);
			xpa_messages (got, (gchar **) buf, names, messages);
			g_free (ds9_cmd);
			
			if (DS9_zoom != 0.0) { /* Pan and zoom initialised */
				ds9_cmd = g_strdup_printf ("zoom to %f", DS9_zoom);
				got = XPASet (xpa, img->ds9.window, ds9_cmd, NULL, buf,len, 
													     names, messages, NXPA);
				xpa_messages (got, (gchar **) buf, names, messages);
				g_free (ds9_cmd);
			}			
			
			DS9NewFrame = FALSE;
		}
	} else {
		g_free (ds9_cmd);
		return show_error (__func__, "Can't find DS9 to display image!");
	}
	
	return TRUE;
	#else
		return show_error (__func__, "GoQat was compiled without xpa support "
						                              "- can't display image!");
	#endif
}

gboolean xpa_get_rect_coords (gfloat *x, gfloat *y, gfloat *w, gfloat *h)
{
	#ifdef HAVE_LIBXPA
	/* Return the dimensions and location of the rectangular region in DS9.
	 * The centre is at (x, y), the width is w and the height h.
	 */
	

	gint i, got;
	gint lens[NXPA];
	gchar *bufs[NXPA];
	gchar *names[NXPA];
	gchar *messages[NXPA];
	gchar *box;
	gchar *endy, *endw, *endh;	
	
	/* Get the region(s) */
	
	if (!(got = XPAGet (xpa, DS9_CCD, "regions", NULL, bufs, lens, names, 
		                                                       messages, NXPA)))
		goto no_region;

	for (i = 0; i < got; i++) {
		if (messages[i] == NULL) {
			bufs[i][lens[i]] = '\0';
			L_print ("DS9 regions:\n%s\n", bufs[i]);
			box = g_strstr_len (bufs[i], lens[i], "box(");/* First defined*/
			if (box == NULL)                              /*  box         */
				goto no_region;
			*x = strtod (box+4, &endy);
        	*y = strtod (++endy, &endw);
			*w = strtod (++endw, &endh);	
			*h = strtod (++endh, (gchar **) NULL);
			g_free (bufs[i]);
		} else
			L_print ("{r}XPA error: %s (%s)\n", messages[i], names[i]);
		if (names[i])
			g_free (names[i]);
		if (messages[i])
			g_free (messages[i]);
	}
	return TRUE;
	
no_region:
	msg ("Warning - No imaging regions defined in DS9!");
	return FALSE;
	#else
	msg ("Warning - Can't find imaging region - GoQat was compiled "
			         "without xpa support\n so the image cannot be displayed!");
	return FALSE;	    
	#endif
}

static gboolean xpa_messages (gint i, gchar *bufs[], gchar *names[], 
							  gchar *messages[])
{
	/* Display XPA error messages */
	
	gint j;
	gboolean xpa_error = FALSE;
	
	for (j = 0; j < i; j++) {
		if (messages[j] == NULL) {
			if (bufs) {
				if (bufs[j])
					g_free (bufs[j]);
			}
		} else {
			L_print ("{r}XPA error: %s (%s)\n", messages[j], names[j]);
			xpa_error = TRUE;
		}
		if (names[j])
			g_free (names[j]);
		if (messages[j])
			g_free (messages[j]);
	}
	return xpa_error;
}

gboolean image_calc_hist_and_flux (void)
{
	/* Generate histogram plot of the data within the selection rectangle and x 
	 * and y flux plots of the data within the centroid area.  Each point in the
	 * x flux plot is the sum of each column, and each point in the y flux plot 
	 * is the sum of each row.  Also calculate the mean x and y positions 
	 * (centroid) of the data in the centroid area.  Note that the raw camera 
	 * data may have had the background level globally subtracted using the 
	 * slider in the autoguider window, and/or may be dark-subtracted.  The 
	 * Image window is shown/hidden after being created, so pointers are freed 
	 * when the application ends.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	GooCanvasPoints *cvpHist;
	GooCanvasPoints *cvpHFlux, *cvpVFlux;
	static GooCanvasItem *cviHist = NULL;
	static GooCanvasItem *cviHistScale = NULL;	
	static GooCanvasItem *cviHPlot = NULL;
	static GooCanvasItem *cviVPlot = NULL;
	static GooCanvasItem *cviHPlotMax = NULL;
	static GooCanvasItem *cviVPlotMax = NULL;
	static GooCanvasItem *cviHLabel = NULL;
	static GooCanvasItem *cviVLabel = NULL;
	gushort xo1, xo2, yo1, yo2;
	gushort hc1, hc2, vc1, vc2, csemi;
	gushort h, v, val, hp, vp;
	static guint numframes, reset_time = 0;
	guint i, j, k;
	guint counts[BOXSIZE], peak = 0;
	static gfloat init_h, init_v, shift_h, shift_v;
	static gfloat rms_h = 0, rms_v = 0;
	gdouble maxh = 0.0, maxv = 0.0;
	gdouble sumh = 0.0, sigmah = 0.0, sumv = 0.0, sigmav = 0.0;

	/* Return if there's no image presently displayed */
	
	if (!aug->canv.cviImage)
		return TRUE;
	
	/* Get the coordinates of the area of overlap between selection rectangle
	 * and image.  It's not a fatal error if there's no overlap, so return
	 * without doing anything.
	 */
	
	if (!is_in_image (aug, &xo1, &yo1, &xo2, &yo2))
		return TRUE;
	
	/* Set up histogram points structure and scale the points values so that it
	 * displays in the right place on the canvas.  This is un-ref'd in the call 
	 * to ui_show_augcanv_plot later - no need to un-ref it here. The 'x' values
     * for the plot are in the even-numbered indices and the 'y' values are in 
     * the odd-numbered indices.
	 */

	cvpHist = goo_canvas_points_new (BOXSIZE);
	
	/* Bin the histogram data into BOXSIZE bins first.  Note in the following 
	 * that (aug->imdisp.W + 1) / BOXSIZE is assumed to be an integer.
	 */
	
	for (i = 0, k = 0; i < BOXSIZE; i++) {
		counts[i] = 0;
		for (j = 0; j < (aug->imdisp.W + 1) / BOXSIZE; j++)
			counts[i] += aug->rect.mode[GREY].hist[k++];
		peak = MAX (counts[i], peak);
	}
		
	for (i = 0; i <  BOXSIZE * 2; i++) {
		cvpHist->coords[i] = (i%2) ? (gdouble) counts[(i - 1) / 2] : 0;
		cvpHist->coords[i] = (i%2) ? (gdouble) YHIST + (gdouble) BOXSIZE * 
				   (1.0 - cvpHist->coords[i] / peak) :
			       (gdouble) (XPLOT + i / 2);
	}

	/* Display the histogram */		
			
	cviHist = ui_show_augcanv_plot (cvpHist, cviHist);
	
	/* Set the scale value under the histogram plot */

	cviHistScale = ui_show_augcanv_text ((gdouble) XPLOT, 
	                                     (gdouble) (YHIST + BOXSIZE + TGAP),
	                                     "Peak at:",
	                                     aug->rect.mode[GREY].peakbin,
	                                     1,
							             3,
	                                     "red",
	                                     cviHistScale);
	
	/* Get the coordinates of the centroid box, constraining it to within
	 * the boundaries of the image.
	 */
	
	csemi = (aug->canv.csize - 1) / 2;
	hc1 = aug->rect.max[GREY].h - csemi > 0 ? aug->rect.max[GREY].h - csemi : 0;
	hc2 = aug->rect.max[GREY].h + csemi < aug->exd.h_pix ? 
								 aug->rect.max[GREY].h + csemi : aug->exd.h_pix;
	vc1 = aug->rect.max[GREY].v - csemi > 0 ? aug->rect.max[GREY].v - csemi : 0;
	vc2 = aug->rect.max[GREY].v + csemi < aug->exd.v_pix ? 
								 aug->rect.max[GREY].v + csemi : aug->exd.v_pix;
	hp = hc2 - hc1 + 1;
	vp = vc2 - vc1 + 1;
	
	/* Set up the flux plot points structures.  They are un-ref'd in the call to 
	 * ui_show_augcanv_plot later - no need to un-ref them here.
	 */
	
	cvpHFlux = goo_canvas_points_new (hp * 2);
	for (i = 0; i <  hp * 4; i++)
		cvpHFlux->coords[i] = 0;	

	cvpVFlux = goo_canvas_points_new (vp * 2);
	for (i = 0; i <  vp * 4; i++)
		cvpVFlux->coords[i] = 0;
	
	/* Now do flux plots... The coords arrays contain the 'x' values for the
     * plot in the even-numbered indices and the 'y' values in the odd-numbered 
	 * indices.  Here we are storing just the 'y' values for the time being, but
	 * since the flux plots are drawn as a series of horizontal bars for each
	 * 'y' value, we have to store each 'y' value twice (in successive slots).
	 * The contents of the flux coords arrays are used for the centroid 
	 * calculation, so we subtract the sky background estimate first.
	 */
	
	for (v = vc1, i = 1; v <= vc2; v++, i += 4)
		for (h = hc1, j = 1; h <= hc2; h++, j += 4) {		
			val = aug->disp_16_1[aug->exd.h_pix * v + h];
			val = MAX ((gint) val - (aug->rect.stdev[GREY].median +
				  (gushort)(aug->imdisp.stdev * aug->rect.stdev[GREY].val)), 0);
			cvpHFlux->coords[j] += val;
			cvpHFlux->coords[j + 2] += val;
			cvpVFlux->coords[i] += val;      /* Could reverse this too, to    */
			cvpVFlux->coords[i + 2] += val;  /* match reversed v-coordinate!  */
		}
				
	/* Determine maximum value for each flux plot.  Also find weighted h and v
	 * positions of maximum flux.  Then set bin contents so that the flux plots 
	 * display in the correct place.
	 */
		
	/* X flux plot ... */

	for (h = 1, j = 1; j < 4 * hp; j += 4) {
		maxh = MAX (cvpHFlux->coords[j], maxh);
		sigmah += h++ * cvpHFlux->coords[j];
		sumh += cvpHFlux->coords[j];
	}
	aug->rect.mean[GREY].h = hc1 + (gfloat) (sigmah / sumh) - 1; /* Mean h pos*/
	
	for (j = 1; j < 4 * hp; j += 2)                              /* 'y' values*/
		cvpHFlux->coords[j] = (gdouble) (YHIST + 2 * (BOXSIZE + YGAP)) + 
     	                      (gdouble) BOXSIZE * (1.0 - cvpHFlux->coords[j] /
	                           maxh);
			
	cvpHFlux->coords[0] = (gdouble) XPLOT;                       /* 'x' values*/
	for (j = 2; j < 4 * hp - 2; j += 4) {
		cvpHFlux->coords[j] = (gdouble) XPLOT + ((j / 4) + 1) * 
                              ((gdouble) BOXSIZE / (gdouble) hp);
		cvpHFlux->coords[j + 2] = cvpHFlux->coords[j];
	}
	cvpHFlux->coords[4 * hp - 2] = (gdouble) (XPLOT + BOXSIZE);
			
	/* Display the x-flux plot */			
	
	cviHPlot = ui_show_augcanv_plot (cvpHFlux, cviHPlot);
	
	/* Set the present maximum under the plot */
	
	cviHPlotMax = ui_show_augcanv_text ((gdouble) XPLOT,
	                           (gdouble)(YHIST + 3 * BOXSIZE + 2 * YGAP + TGAP),
	                           "Max:  ",
	                           maxh,
	                           1,
							   6,
						       "red",
	                           cviHPlotMax);

	/* Y flux plot ... */
	
	for (v = 1, i = 1; i < 4 * vp; i += 4) {
		maxv = MAX (cvpVFlux->coords[i], maxv);
		sigmav += v++ * cvpVFlux->coords[i];
		sumv += cvpVFlux->coords[i];
	}
	aug->rect.mean[GREY].v = vc1 + (gfloat) (sigmav / sumv) - 1; /* Mean v pos*/
	
	for (i = 1; i < 4 * vp; i += 2)                              /* 'y' values*/
		cvpVFlux->coords[i] = (gdouble) (YHIST + BOXSIZE + YGAP) + 
     	                      (gdouble) BOXSIZE * (1.0 - cvpVFlux->coords[i] /
	                           maxv);
			
	cvpVFlux->coords[0] = (gdouble) XPLOT;                       /* 'x' values*/
	for (i = 2; i < 4 * vp - 2; i += 4) {
		cvpVFlux->coords[i] = (gdouble) XPLOT + ((i / 4) + 1) * 
                              ((gdouble) BOXSIZE / (gdouble) vp);
		cvpVFlux->coords[i + 2] = cvpVFlux->coords[i];
	}
	cvpVFlux->coords[4 * vp - 2] = (gdouble) (XPLOT + BOXSIZE);
			
	/* Display the y-flux plot */			
	
	cviVPlot = ui_show_augcanv_plot (cvpVFlux, cviVPlot);
	
	/* Set the present maximum under the plot */
	
	cviVPlotMax = ui_show_augcanv_text ((gdouble) XPLOT,
	                               (gdouble)(YHIST + 2 * BOXSIZE + YGAP + TGAP),
	                               "Max:  ",
	                               maxv,
	                               1,
							       6,
							       "red",
	                               cviVPlotMax);
							 
	/* Optionally show the box-and-dot marker for the centroid of the star */
	
	ui_show_augcanv_centroid (aug->canv.Centroid,
				              (aug->img.max[GREY].val >= aug->imdisp.satlevel),
                              aug->exd.h_top_l + aug->rect.mean[GREY].h,
				              aug->exd.v_top_l + aug->rect.mean[GREY].v, 
				              hc1 + aug->exd.h_top_l,
				              hc2 + aug->exd.h_top_l,
				              vc1 + aug->exd.v_top_l,
				              vc2 + aug->exd.v_top_l);
	
	/* Calculate and display the rms shift in the centroid position */
		
	if (aug->canv.NewRect || isnan (rms_h) || isnan (rms_v)) {
		numframes = 0;
		rms_h = 0.0;
		rms_v = 0.0;
		aug->exd.frame_rate = 0;
		init_h = aug->rect.mean[GREY].h;
		init_v = aug->rect.mean[GREY].v;
		reset_time = loop_elapsed_since_first_iteration ();
	}
	
	numframes++;
	shift_h = aug->rect.mean[GREY].h - init_h;
	shift_v = aug->rect.mean[GREY].v - init_v;
	rms_h = sqrt((rms_h * rms_h * (numframes-1) + shift_h * shift_h)/numframes);
	rms_v = sqrt((rms_v * rms_v * (numframes-1) + shift_v * shift_v)/numframes);
		
	cviHLabel = ui_show_augcanv_text ((gdouble) XPLOT,
	                         (gdouble)(YHIST + 3 * (BOXSIZE + TGAP) + 2 * YGAP),
						     "RMS:",
						     rms_h,
						     2,
						     3,
		  			         "orange",
						     cviHLabel);
							   
	cviVLabel = ui_show_augcanv_text ((gdouble) XPLOT,
	                         (gdouble)(YHIST + 2 * BOXSIZE + YGAP + 3 * TGAP),
						     "RMS:",
						     rms_v,
						     2,
						     3,
					         "orange",
						     cviVLabel);
	
	/* Calculate the shift North/South and East/West since the selection
	 * rectangle was last re-drawn.
	 */
	
	aug->rect.shift_ns = shift_h * aug->autog.s.Uvec_N[0] + 
	                     shift_v * aug->autog.s.Uvec_N[1];
	/* Want a shift east to be negative by default on the trace display and in*/
	/* the star positions file (i.e. for north up and east left, star moves to*/
	/* lower pixel numbers on the display when going east (left).             */ 
	aug->rect.shift_ew = - (shift_h * aug->autog.s.Uvec_E[0] + 
	                        shift_v * aug->autog.s.Uvec_E[1]);
	                        
	/* Calculate average frame rate */
	
	aug->exd.frame_rate = (numframes - 1) * 1000.0 / 
			       (loop_elapsed_since_first_iteration () - reset_time);
	G_print ("Average autoguider frame rate: %f\n", aug->exd.frame_rate);
			       
	aug->canv.NewRect = FALSE;
	return TRUE;
}

gboolean is_in_image (struct cam_img *img, gushort *xoff1, gushort *yoff1, 
                                           gushort *xoff2, gushort *yoff2)
{
	/* Tests to see whether the selection rectangle intersects the image and
	 * returns TRUE if it does, FALSE if it doesn't.  Passes back the
	 * coordinates of the corners of the overlapping area, as offsets from the
	 * image (not the window) top left corner.  Note that this routine doesn't
	 * assume the pre-existence of an image - the calculation is based on the 
	 * boundary within which the image would lie, if it existed.
	 * NOTE that the selection rectangle is a boundary round all selected pixels
	 * and so for pixels 1 to N, the rectangle has edges at 1 (left of left-most
	 * pixel) and N+1 (left of [right-most pixel+ 1]).  We allow for that here
	 * by subtracting 1 from the right-most (and bottom-most) coordinates of the
	 * selection rectangle.  Since we are returning coordinates relative to the
	 * data array in memory, the coordinates are zero-offset, so the offset of
	 * the top left pixel is (0, 0).
	 */

	gushort xa1, xa2, ya1, ya2;
	
	xa1 = (gushort) MAX (img->exd.h_top_l, img->canv.r.htl);
	xa2 = (gushort) MIN (img->exd.h_bot_r, img->canv.r.hbr - 1);
	ya1 = (gushort) MAX (img->exd.v_top_l, img->canv.r.vtl);
	ya2 = (gushort) MIN (img->exd.v_bot_r, img->canv.r.vbr - 1);
	
	/* Check that the selected area actually contains any image at all */
		
		if ((xa2 >= xa1) && (ya2 >= ya1)) {
			*xoff1 = xa1 - img->exd.h_top_l;
			*yoff1 = ya1 - img->exd.v_top_l;
			*xoff2 = xa2 - img->exd.h_top_l;
			*yoff2 = ya2 - img->exd.v_top_l;
		} else
			return FALSE;
		
	return TRUE;
}

void image_get_stats (struct cam_img *img, enum ColsPix c)
{
	/* Get some image statistics */
	
	gushort row, col;
	guint counts;
	gint i, j, totpix;
	gfloat stdev_new;
	
	totpix = img->exd.h_pix * img->exd.v_pix;  /* Total pixels in image */
	
	if (c == C_GREY) {  /* Greyscale */
		
		img->img.min[GREY].val = img->imdisp.W;          /* Initialise values */
		img->img.max[GREY].val = img->imdisp.B;
		img->img.mean[GREY].val = 0;
		memset (img->img.mode[GREY].hist,0,(img->imdisp.W + 1) * sizeof(guint));
		
		for (i = 0; i < totpix; i++) {
			col = i%img->exd.h_pix;
			row = i / img->exd.h_pix;		                     /* Minimum */
			img->img.min[GREY].val = MIN (img->img.min[GREY].val, img->vadr[i]);
			if (img->vadr[i] > img->img.max[GREY].val) {  /* Max. and location*/
				img->img.max[GREY].val = img->vadr[i];
				img->img.max[GREY].h = col;
				img->img.max[GREY].v = row;
			}
			img->img.mean[GREY].val += img->vadr[i];
			++img->img.mode[GREY].hist[img->vadr[i]];            /* Histogram */
		}
		img->img.mean[GREY].val /= totpix;                       /* Mean */
		img->img.mode[GREY].peakcount = 0;
		img->img.stdev[GREY].val = 0;
	    for (i = 0; i <= img->imdisp.W; i++) {
		    if (img->img.mode[GREY].hist[i] > img->img.mode[GREY].peakcount) {
				img->img.mode[GREY].peakcount = img->img.mode[GREY].hist[i];
				img->img.mode[GREY].peakbin = i;                 /* Mode */
			}
			img->img.stdev[GREY].val += pow ((i - img->img.mean[GREY].val), 2) *
			                                        img->img.mode[GREY].hist[i];
		}                                                        /* Stdev */
		img->img.stdev[GREY].val = sqrt (img->img.stdev[GREY].val / totpix);
		
		stdev_new = counts = 0;   /* Calc. 3-sigma clipped standard deviation */
  	    for (i = (gint) 
			 MAX ((img->img.mode[GREY].peakbin - 3 * 
				   img->img.stdev[GREY].val), 0);
		     i <= (gint) MIN ((img->img.mode[GREY].peakbin + 3 * 
							   img->img.stdev[GREY].val), img->imdisp.W);
		     i++) {
		    stdev_new += pow ((i - img->img.mean[GREY].val), 2) * 
				                                    img->img.mode[GREY].hist[i];
  		    counts += img->img.mode[GREY].hist[i];
	    }                                             /* 3-sig. clipped stdev */
  	    img->img.stdev[GREY].val = sqrt ((stdev_new / counts));		
		
	} else {  /* Calculate simple stats for all colour components in one pass */
		for (i = 0; i < c; i++) {
			img->img.min[c].val = img->imdisp.W;
			img->img.max[c].val = img->imdisp.B;
		}
		
		for (i = 0; i < totpix; i ++) {
			col = i%img->exd.h_pix;
			row = i / img->exd.h_pix;
			for (j = 0; j < c; j++) {
				img->img.min[j].val = MIN (img->img.min[j].val, 
										                 img->bvadr[i * 3 + j]);
				img->img.max[j].val = MAX (img->img.max[j].val, 
										                 img->bvadr[i * 3 + j]);
			}
		}
	}
}

gboolean image_embed_data (struct cam_img *img)
{
	/* Embed the image data in an image the size of the full imaging area, 
	 * expanding back to actual area of coverage, if binned.  The value in the
	 * blank areas of the full image is set to the minimum value in the exposed
	 * part.
	 */
	
	gushort val;
	gushort m, n, h, v;
	guint i, k;
	
	if (img->vadr == NULL)
		return FALSE;
	if (img->disp_16_1 == NULL)
		return FALSE;
	
	i = img->cam_cap.max_h * img->cam_cap.max_v;
	while (i)
		img->disp_16_1[--i] = img->img.min[GREY].val;

	k = img->exd.h_top_l + img->exd.v_top_l * img->cam_cap.max_h;
	for (v = 0; v < img->exd.v_pix * img->exd.v_bin; v += img->exd.v_bin)
    	for (m = 0; m < img->exd.v_bin; m++) {
      		for (h = 0; h < img->exd.h_pix*img->exd.h_bin; h += img->exd.h_bin){
        		for (n = 0; n < img->exd.h_bin; n++) {
					val = img->vadr[img->exd.h_pix * 
					                   v / img->exd.v_bin + h / img->exd.h_bin];

					/* Set the image pixel data */	

					img->disp_16_1[k++] = val;
				}
			}
			k += img->cam_cap.max_h - img->exd.h_pix * img->exd.h_bin;
		}
	
	return TRUE;
}

gboolean image_save_as_fits (struct cam_img *img, gchar *savefile, 
	                         enum Colour colour, gboolean display)
{
	/* Save the image in standard FITS format as a primary header and data unit
	 * (HDU).  Primary header is a single 2880 byte record in this case,
	 * comprising a series of 80 byte card images.  Then follows the data, 
	 * adjusted to be in the range -32768 to 32767 and in big-endian format.
	 * The overall file size is set to be a multiple of 2880 bytes.
	 * See FITS standard, NOST 100-2.0
	 * If display == TRUE then we are saving a copy for display in DS9;
	 * otherwise we are saving the raw image data for use elsewhere.
	 * This routine is called three times for a colour image; once each for the 
	 * R, G and B components.
	 */

	const gint REC_LEN = 2880;
	const gint HEAD_LEN = 1 * 2880;
	const gint CARD_LEN = 80;
	const gint OFFSET = 32768;	
	
	gshort *data;
	gushort	bytes_per_pixel;
	gint h, i = 0, j, k, numpix, numbytes;
	gchar *header, *string;
	void *hdu;
	
	FILE *fp;
	
	/* Allocate memory for the HDU (header plus data plus padding) */

	if (display && img->id == CCD && img->FullFrame)
		numpix = img->cam_cap.max_h * img->cam_cap.max_v;
	else
		numpix = img->exd.h_pix * img->exd.v_pix;
	numbytes = (gint) (HEAD_LEN + numpix * sizeof (gshort) +
	                  (REC_LEN - fmod (numpix * sizeof (gshort), REC_LEN)));
	if (!(hdu = g_malloc (numbytes)))
		return show_error (__func__, "Unable to allocate memory for HDU");
	header = (char *) memset (hdu, ' ', HEAD_LEN);
	
	/* Set C locale to ensure correct format of FITS entries with 
	 * decimal point.
	 */
	 
	setlocale (LC_NUMERIC, "C");
	
	/* Write the header records to the HDU */
	
	h = -CARD_LEN;
	
	h += CARD_LEN;
	sprintf (&header[h], "SIMPLE  =                    T /"
	                  "   Standard conforming file");
	header[strlen (header)] = ' ';     /* Overwrite closing '\0' from sprintf */

	h += CARD_LEN;
	sprintf (&header[h], "BITPIX  =                   16 /"
	                  "   16 bits per pixel");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "NAXIS   =                    2 /"
	                  "   2 image axes");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;	
	sprintf (&header[h], "NAXIS1  = %20i /   no. pixels on horizontal axis", 
		    (display && img->id == CCD && img->FullFrame) ? 
			 img->cam_cap.max_h : img->exd.h_pix);
	header[strlen (header)] = ' ';	
	
	h += CARD_LEN;	
	sprintf (&header[h], "NAXIS2  = %20i /   no. pixels on vertical axis",
	        (display && img->id == CCD && img->FullFrame) ? 
			 img->cam_cap.max_v : img->exd.v_pix);
	header[strlen (header)] = ' ';

	h += CARD_LEN;
	sprintf (&header[h], "CTYPE1  = ' '                  /"
	                  "   axis 1 data type");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CRPIX1  =                  1.0 /"
	                  "   reference point at first pixel on axis 1");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CRVAL1  = %20.1f /"
	                  "   pixel offset from start of frame on axis 1",
		              (img->id != CCD ? 
			           0 : display && img->id == CCD && img->FullFrame ?
			           0 : (gfloat) (img->exd.h_top_l)));
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CDELT1  = %20i /"
	                     "   rate of increase of pixel count ", img->exd.h_bin);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CTYPE2  = ' '                  /"
	                  "   axis 2 data type");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CRPIX2  =                  1.0 /"
	                  "   reference point at first pixel on axis 2");
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CRVAL2  = %20.1f /"
	                  "   pixel offset from start of frame on axis 2",
			          (img->id != CCD ? 
					   0 :  display && img->id == CCD && img->FullFrame ?
			           0 : (gfloat) (img->cam_cap.max_v - 
		              (img->exd.v_top_l + img->exd.v_pix * img->exd.v_bin))));					  
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CDELT2  = %20i /"
	                     "   rate of increase of pixel count ", img->exd.v_bin);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "BINX1   = %20i /   pixel binning on X1 axis",
																img->exd.h_bin);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "BINX2   = %20i /   pixel binning on X2 axis",
																img->exd.v_bin);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "BZERO   = %20i /"
	                  "   offset to add back on for unsigned integers", OFFSET);
	header[strlen (header)] = ' ';

	h += CARD_LEN;
	sprintf (&header[h], "DATAMAX = %20i /   maximum data value", 
	    (img->id == AUG) ? img->pic.max[colour].val : img->img.max[colour].val);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "DATAMIN = %20i /   minimum data value", 
	    (img->id == AUG) ? img->pic.min[colour].val : img->img.min[colour].val);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "CCDTEMP = %20.1f /   CCD temperature (C)", 
	                                                          img->state.c_ccd);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "FOCUSPOS= %20i /   focuser position", 
			                                               img->fits.focus_pos);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "FOCUSTMP= %20.1f /   focuser temperature (C)",
			                                              img->fits.focus_temp);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;	
	sprintf (&header[h], "DATE-OBS= '%-23s'/"
	                  "   date of start of observation (UTC)", 
					                                        img->fits.date_obs);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;	
	sprintf (&header[h], "UTSTART = '%-12s'       /"
	                  "   time of start of observation (UTC)", 
					                                         img->fits.utstart);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;	
	sprintf (&header[h], "TM-START= %20.3f /"
	                  "   seconds since midnight (UTC) at start of obs.",
					                                        img->fits.tm_start);
	header[strlen (header)] = ' ';	
	
	h += CARD_LEN;
	sprintf (&header[h], "EXPTIME = %20.3f /"
	                  "   exposure length (seconds)",(gdouble)img->exd.act_len);
	header[strlen (header)] = ' ';	
	
	h += CARD_LEN;
	string = get_entry_string ("txtTelescop");
	sprintf (&header[h], "TELESCOP= '%s'", (img->id != VID) ? string : "");
	header[strlen (header)] = ' ';
	g_free (string);

	h += CARD_LEN;
	string = get_entry_string ("txtInstrume");
	sprintf (&header[h], "INSTRUME= '%s'", (img->id != VID) ? string : "");
	header[strlen (header)] = ' ';
	g_free (string);
	
	h += CARD_LEN;
	string = get_entry_string ("txtObserver");
	sprintf (&header[h], "OBSERVER= '%s'", (img->id != VID) ? string : "");
	header[strlen (header)] = ' ';
	g_free (string);
	
	h += CARD_LEN;
	string = get_entry_string ("txtObject");
	sprintf (&header[h], "OBJECT  = '%s'", (img->id != VID) ? string : "");
	header[strlen (header)] = ' ';
	g_free (string);
	
	h += CARD_LEN;	
	sprintf (&header[h], "EQUINOX = %20.4f /"
	                  "   Julian epoch of coordinates", img->fits.epoch);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "RA      = '%8s'           /"
	                  "   Right Ascension of center of image", img->fits.RA);
	header[strlen (header)] = ' ';
	
	h += CARD_LEN;
	sprintf (&header[h], "DEC     = '%9s'          /"
	                  "   declination of center of image", img->fits.Dec);
	header[strlen (header)] = ' ';
	
	if (img->id == CCD && !img->Debayer && img->bayer_pattern >= 0 && 
		img->exd.h_bin == 1 && img->exd.v_bin == 1) {
		gint tile;
		gchar *bp;
		tile = debayer_get_tile (img->bayer_pattern, img->exd.h_top_l, 
								 img->exd.v_top_l);
		/* See debayer.c for the following constants.  Note that we flip the
		 * pattern here to allow for the fact that we reverse the order of
		 * the rows when we save the data.
		 */
		switch (tile) {
			case 512:
				/*bp = "RG";*/
				bp = "GB";
			    break;
			case 513:
				/*bp = "GB";*/
				bp = "RG";
			    break;
			case 514:
				/*bp = "GR";*/
				bp = "BG";
			    break;
			case 515:
				/*bp = "BG";*/
				bp = "GR";
			    break;
			default:
				bp = "??";
		}
		h += CARD_LEN;
		sprintf (&header[h], "BAYERPAT= '%2s'                 /"
						  "   bayer pattern at start of image", bp);
		header[strlen (header)] = ' ';
	}

	h += CARD_LEN;	
	sprintf (&header[h], "END");
	header[strlen (header)] = ' ';
	
	/* Restore locale setting to local value */
	 
	setlocale (LC_NUMERIC, "");
	
	/* Write the data to the HDU, swapping the rows so that other software
	 * (e.g. DS9, Starlink's Gaia and Kappa routines or the GIMP) display an 
	 * inverted image on the chip the right way up.  (Hence a picture taken with
	 * a camera lens attached to the chip would appear correctly).
	 */

	data = (gshort *) hdu;
	if (img->id == CCD) {
		if (colour == GREY) {
			if (display && img->FullFrame) {
				for (i = HEAD_LEN / sizeof (gshort),
					 k = numpix - img->cam_cap.max_h;
					 k >= 0; k -= img->cam_cap.max_h)
					for (j = k; j < (k + img->cam_cap.max_h); j++)
						data[i++] = GINT16_TO_BE (img->disp_16_1[j] - OFFSET);
			} else {
				for (i = HEAD_LEN / sizeof (gshort),
					 k = numpix - img->exd.h_pix;
					 k >= 0; k -= img->exd.h_pix)
					for (j = k; j < (k + img->exd.h_pix); j++)
						data[i++] = GINT16_TO_BE (img->vadr[j] - OFFSET);
			}
		} else {
			if (display && img->FullFrame) {
				for (i = HEAD_LEN / sizeof (gshort),
					 k = numpix - img->cam_cap.max_h;
					 k >= 0; k -= img->cam_cap.max_h)
					for (j = k; j < (k + img->cam_cap.max_h); j++)
						data[i++] = GINT16_TO_BE (
									   img->disp_16_3[colour + j * 3] - OFFSET);
			} else {
				for (i = HEAD_LEN / sizeof (gshort),
					 k = numpix - img->exd.h_pix;
					 k >= 0; k -= img->exd.h_pix)
					for (j = k; j < (k + img->exd.h_pix); j++)
						data[i++] = GINT16_TO_BE (
										img->bvadr[colour + j * 3] - OFFSET);
			}
		}
	} else if (img->id == AUG) {
		bytes_per_pixel = 3;  /* Always assumed to equal 3 at this stage! */
		for (i = HEAD_LEN / sizeof (gshort),
			 k = numpix - img->exd.h_pix; 
			 k >= 0; k -= img->exd.h_pix)
			for (j = k; j < (k + img->exd.h_pix); j++)
				data[i++] = GINT16_TO_BE (
								   img->disp_8_3[j * bytes_per_pixel] - OFFSET);
	} else if (img->id == VID) {
		for (i = HEAD_LEN / sizeof (gshort),
			 k = numpix - img->exd.h_pix; 
		     k >= 0; k -= img->exd.h_pix)
			for (j = k; j < (k + img->exd.h_pix); j++)
				data[i++] = GINT16_TO_BE (img->vadr[j] - OFFSET);
	}
	
	/* Fill any remaining space with zeros */
	
	while (i < numbytes / sizeof (gshort))
		data[i++] = 0;
	
	/* Write the HDU to a disk file */
	
    if (!(fp = fopen (savefile, "wb"))) {
		g_free (hdu);
		return show_error (__func__, "Error writing image file (fopen)");
	}
	
    if (fwrite (hdu, sizeof (gchar), numbytes, fp) < numbytes) {
		g_free (hdu);
		fclose (fp);
		return show_error (__func__, "Error writing image file (fwrite)");
	}
	fclose (fp);

	g_free (hdu);
	
	return TRUE;
}

#ifdef HAVE_LIBGRACE_NP
gboolean Grace_Open (gchar *plot, gboolean *AlreadyOpen)
{
	/* Open the given plot in Grace.  Set AlreadyOpen to TRUE if the plot
	 * is already open, FALSE otherwise.  Return FALSE if there is an error
	 * opening Grace, or if Grace is already open with a different plot from
	 * the one given.
	 */
	
	gushort i;
	static gchar *orig_plot = NULL;
	
	/* GraceIsOpen only recognises a broken pipe from a previous invocation 
	 * when an attempt is made to write to Grace.  So attempting a write here
	 * will force GraceIsOpen to give the correct response.
	 */	
	
	GracePrintf (" "); 
	                  
	if (!(GraceIsOpen ())) { /* Open Grace if not already open */
		if (orig_plot) {
			g_free (orig_plot);
			orig_plot = NULL;
		}
		if (GraceOpenVA ("xmgrace", 2048, "-nosafe", "-geometry", 
						 "875x700+10+10", plot, NULL) == -1)
			return show_error (__func__, "Error opening Grace for plotting");
		for (i = 0; i < 5; i++) {  /* Wait up to 5 seconds for Grace to open */
			if (!(GraceIsOpen ()))
				sleep (1);
		}
		if (!(GraceIsOpen ())) {
			return show_error (__func__, "Error opening Grace for plotting");
		}
		GraceRegisterErrorFunction (Grace_Error);
		orig_plot = g_strdup (plot);
		G_Error = FALSE;
	} else {
		if (!strcmp (plot, orig_plot))
			*AlreadyOpen = TRUE;
		else {
			*AlreadyOpen = FALSE;
		    L_print ("{o}Grace can't open %s; already plotting to %s!\n", plot,
					                                                 orig_plot);
		    return FALSE;
		}
	}
	
	return TRUE;
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_SetXAxis (gushort graph, gfloat xmin, gfloat xmax)
{
	/* Set the values for the x-axis range for graph 'graph' */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	g_string_printf (s, "with g%i", graph);
	GracePrintf (s->str);
	setlocale (LC_NUMERIC, "C");
	GracePrintf ("world xmin %f", xmin);
	GracePrintf ("world xmax %f", xmax);
	setlocale (LC_NUMERIC, "");
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_SetYAxis (gushort graph, gfloat ymin, gfloat ymax)
{
	/* Set the values for the y-axis range for graph 'graph' */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
		
	g_string_printf (s, "with g%i", graph);
	GracePrintf (s->str);
	setlocale (LC_NUMERIC, "C");
	GracePrintf ("world ymin %f", ymin);
	GracePrintf ("world ymax %f", ymax);
	setlocale (LC_NUMERIC, "");
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_XAxisMajorTick (gushort graph, gint tick)
{
	/* Set the major tick spacing for the x-axis for graph 'graph' */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	g_string_printf (s, "with g%i", graph);
	GracePrintf (s->str);
	setlocale (LC_NUMERIC, "C");
	GracePrintf ("xaxis tick major %d", tick);
	setlocale (LC_NUMERIC, "");
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_YAxisMajorTick (gushort graph, gint tick)
{
	/* Set the major tick spacing for the y-axis for graph 'graph' */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	g_string_printf (s, "with g%i", graph);
	GracePrintf (s->str);
	setlocale (LC_NUMERIC, "C");
	GracePrintf ("yaxis tick major %d", tick);
	setlocale (LC_NUMERIC, "");
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_PlotPoints (gushort graph, gushort set, gfloat x, gfloat y)
{
	/* Plot the specified points */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	setlocale (LC_NUMERIC, "C");
	g_string_printf (s, "g%d.s%d point %f, %f", graph, set, x, y);
	setlocale (LC_NUMERIC, "");
	GracePrintf (s->str);
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_ErasePlot (gushort graph, gushort set)
{
	/* Erase the specified plot, keeping its attributes */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	g_string_printf (s, "kill g%d.s%d saveall", graph, set);
	GracePrintf (s->str);
}
#endif

#ifdef HAVE_LIBGRACE_NP
void Grace_SaveToFile (gchar *FileName)
{
	/* Save the current Grace graphs to the given file */
	
	static GString *s = NULL;
	
	if (!s)  /* This GString is freed when the app. quits */
		s = g_string_new (NULL);
	
	g_string_printf (s, "saveall \"%s\"", FileName);
	GracePrintf (s->str);
}
#endif


#ifdef HAVE_LIBGRACE_NP
gboolean Grace_Update (void)
{
	/* Refresh the Grace display and return FALSE if a Grace plotting error
	 * has occurred.
	 */
	
	GracePrintf ("redraw");
	return !G_Error;
}
#endif

#ifdef HAVE_LIBGRACE_NP
static void Grace_Error (const char *msg)
{
    L_print ("{o}Grace plotting message: \"%s\"\n", msg);
	G_Error = TRUE;
}
#endif
