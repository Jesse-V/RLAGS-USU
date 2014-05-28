/******************************************************************************/
/*                      TELESCOPE INTERFACE ROUTINES                          */
/*                                                                            */
/* All the routines for interfacing with the telescope are contained in here. */
/* This is designed to interface with the Losmandy Gemini System L4 1.04 or   */
/* higher, and LX200-compatibles.                                             */
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
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#ifdef HAVE_LIBPARAPIN
#include <parapin.h>
#endif

#define GOQAT_TELESCOPE
#include "interface.h"

#define PEC_RECLEN 16                    /* Record length for PEC data file   */
#define DEF_RECLEN 40                    /* Record length for defaults file   */

enum GeminiSpeed {                       /* Gemini speed setting command ID's */
	CMD_SPEED_GUIDE = 150,
	CMD_SPEED_CENTER = 170
};

enum GeminiParms {                       /* Gemini modelling parameter ID's   */
	A = 201,
	E, 
	NP, 
	NE, 
	IH, 
	ID, 
	FR, 
	FD, 
	CF, 
	TF = 211
};
                                         /* Gemini PEC data command ID's      */
enum GeminiPEC {
	PEC_RA_STEPS = 27, 
	PEC_RA_COUNTER = 501, 
	PEC_STATUS = 509, 
	PEC_CURRENT_DATA = 511
};

enum GeminiPECStatus {				     /* Gemini PEC status ID's            */

	PEC_ACTIVE = 1, 
	PEC_AVAILABLE = 32
};

enum GeminiQuery {                       /* Gemini status inquiry ID's        */
	TEL_START_WAIT = -1, 
	TEL_ALIGNED = 1, 
	MOD_USE = 2, 
	OBJ_SEL = 4,
	GOTO_ACTIVE = 8, 
	RA_LIM = 16, 
	PRECESS = 32, 
	TEL_UNKNOWN = 64,
	INQUIRY = 99
};

enum GuideCorrection {  		  		 /* Type and direction of guide corr. */
	SD_NONE = 0,
	SHIFT_H = 1, 
	DRIFT_H = 2,
	SHIFT_V = 4, 
	DRIFT_V = 8
};

struct position {                /* Guide star times and positions            */
	gushort maxval;              /* Initial max. value of guide star (counts) */
	guint NumImg;                /* Total no. images analysed for autoguiding */
	guint NumGuide;              /* Total no. images requiring guide correc'n */
	guint min_num;			     /* Minimum no. movements before calc. drift  */
	guint elapsed_h;             /* Elapsed time at end of h correction       */ 
	guint elapsed_v;             /* Elapsed time at end of v correction       */ 	
	guint MOVE_H;                /* Required shift (telescope east or west)   */
	guint MOVE_V;                /* Required shift (telescope north or south) */
	guint GuideCorr;             /* Flags for shift/drift in h/v directions   */
	guint e;                     /* Number of corrections to the east         */
	guint w;                     /* Number of corrections to the west         */
	guint n;                     /* Number of corrections to the north        */
	guint s;                     /* Number of corrections to the south        */
	gfloat init_h;               /* Initial guide-star h and v positions      */
	gfloat init_v;               /*  (used for autoguiding and calibration)   */
	gfloat shift_h;			     /* Shift from initial to present posn.       */
	gfloat shift_v;			     /* Shift from initial to present posn.       */
	gfloat drift_h;			     /* Drift from initial to present posn.       */
	gfloat drift_v;			     /* Drift from initial to present posn.       */
	gfloat offset_h;		     /* Equal to either shift or drift            */
	gfloat offset_v;		     /* Equal to either shift or drift            */
	gfloat min_offset;		     /* Equal to either MaxShift or MaxDrift      */
	gfloat *pos_h;			     /* Latest min_num h positions                */
	gfloat *pos_v;			     /* Latest min_num v positions                */		
	gfloat shift_rate_h;         /* Pixels per second shift at sidereal rate  */
	gfloat shift_rate_v;         /* Pixels per second shift at sidereal rate  */
	gfloat shift_duration_h;     /* Duration of shift (msec) for guide corrn. */
	gfloat shift_duration_v;     /* Duration of shift (msec) for guide corrn. */
	gboolean reset_h;		     /* TRUE to reset drift_h calculation         */
	gboolean reset_v;		     /* TRUE to reset drift_v calculation         */
	gboolean wait_h;             /* TRUE if waiting for h update period       */
	gboolean wait_v;             /* TRUE if waiting for v update period       */
	gboolean active_h;           /* TRUE if an h guide correction is active   */
	gboolean active_v;           /* TRUE if a  v guide correction is active   */
};

static struct cam_img *ccd;       /* Pointer to CCD camera data               */
static struct cam_img *aug;       /* Pointer to autoguider camera data        */
static struct position pos;       /* Guide star time and position data        */
	
/* Losmandy Gemini/Autostar compatible telescope interface commands */
	
static gchar ACK           = 0x06;
static gchar STARTED[]     = "G#";     /* Gemini-specific command             */
static gchar START_WAIT[]  = "b#";     /* Gemini-specific command             */
static gchar RESTART[]     = "bR#";    /* Gemini-specific command             */

static gchar SET_RA[]      = ":Sr";
static gchar SET_DEC[]     = ":Sd";

static gchar GET_RA[]      = ":GR#";
static gchar GET_DEC[]     = ":GD#";

static gchar MOVE_EAST[]   = ":Me#";
static gchar MOVE_WEST[]   = ":Mw#";
static gchar MOVE_NORTH[]  = ":Mn#";
static gchar MOVE_SOUTH[]  = ":Ms#";

static gchar PULSE_EAST[]   = ":Mge";  /* These commands are compatible with  */
static gchar PULSE_WEST[]   = ":Mgw";  /*  Losmandy Gemini L4 and the internal*/
static gchar PULSE_NORTH[]  = ":Mgn";  /*  timer of the Astro-Electronic relay*/
static gchar PULSE_SOUTH[]  = ":Mgs";  /*  box.                               */

static gchar STOP_EAST[]   = ":Qe#";
static gchar STOP_WEST[]   = ":Qw#";
static gchar STOP_NORTH[]  = ":Qn#";
static gchar STOP_SOUTH[]  = ":Qs#";
static gchar STOP_ALL[]    = ":Q#";

static gchar GOTO[]        = ":MS#";

static gchar PARK[]		   = ":hP#";
static gchar QUERY_PARK[]  = ":h?#";

static gchar RATE_CENTER[] = ":RC#";
static gchar RATE_GUIDE[]  = ":RG#";

static gchar SET_DATE[]    = ":SC";
static gchar GET_DATE[]    = ":GC#";
static gchar SET_LTM[]     = ":SL";
static gchar GET_LTM[]     = ":GL#";
static gchar SET_UTCOFF[]  = ":SG";
static gchar GET_UTCOFF[]  = ":GG#";

/* Variable declarations */

static GThreadPool *thread_pool_guide_timing = NULL; /* Guide timing pool     */
static gint orig_rate_center;            /* Original centering rate in Gemini */
static gfloat orig_rate_guide;           /* Original guiding rate in Gemini   */
static gboolean SavedRates = FALSE; /* TRUE when telescope motion rates saved */
static FILE *fg = NULL;                  /* Guide corrections file            */

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void telescope_init (void);
gboolean telescope_open_comms_port (void);
void telescope_close_comms_port (void);
gboolean telescope_open_autog_port (void);
void telescope_close_autog_port (void);
#ifdef HAVE_LIBPARAPIN
gboolean telescope_open_parallel_port (void);
#endif
gboolean telescope_close_parallel_port (void);
void telescope_move_start (enum TelMotion direction);
void telescope_move_stop (enum TelMotion direction);
void telescope_guide_start (enum TelMotion direction);
void telescope_guide_stop (enum TelMotion direction);
void telescope_guide_pulse (enum TelMotion direction, gint duration);
void telescope_d_start (enum TelMotion direction);
void telescope_d_stop (enum TelMotion direction);
void telescope_d_pulse (enum TelMotion direction, gint duration);
void telescope_s_start (enum TelMotion direction);
void telescope_s_stop (enum TelMotion direction);
void telescope_s_pulse (enum TelMotion direction, gint duration);
#ifdef HAVE_LIBPARAPIN
void telescope_p_start (enum TelMotion direction);
void telescope_p_stop (enum TelMotion direction);
#endif
static void thread_pool_guide_timing_func (gpointer data, gpointer user_data);
void telescope_goto (gchar *sRA, gchar *sDec);
gboolean telescope_goto_done (void);
void telescope_move_by (gdouble RA, gdouble Dec);
void telescope_warm_restart (void);
gboolean telescope_warm_restart_done (void);
void telescope_park_mount (void);
gboolean telescope_park_mount_done (void);
gboolean telescope_yellow_button (void);
void telescope_set_center_speed (gfloat speed);
void telescope_set_guide_speed (gfloat speed);
gboolean telescope_autog_calib (gfloat *calib_coords);
gboolean telescope_guide (enum Guide status, guint elapsed);
gboolean telescope_write_guidecorr_time (void);
static gboolean telescope_write_guide_corr (gboolean move, guint now,
									   enum TelMotion direction, gint duration);
gboolean telescope_get_RA_Dec (gboolean precess, gdouble *epoch,
	                           gchar *sRA, gchar *sDec,
                               gdouble *fRA, gdouble *fDec,
							   gboolean *OK_RA, gboolean *OK_Dec);
gushort telescope_get_RA_worm_pos (void);
void telescope_get_guide_corrs (guint *l, guint *r, guint *u, guint *d,
                                gfloat *percent);
gboolean telescope_get_gemini_model (struct GemModel *gm);
gboolean telescope_set_gemini_model (struct GemModel *gm);
gboolean telescope_get_gemini_PEC (gchar *filename);
gboolean telescope_set_gemini_PEC (gchar *filename);
gboolean telescope_set_gemini_defaults (gchar *filename);
gboolean telescope_PEC_on (gboolean On);
void telescope_set_time (void);
guint telescope_query_status (gboolean list);
void telescope_save_motion_rates (void);
void telescope_restore_motion_rates (void);
static gboolean gemini_get (gushort id, gfloat *val, gfloat *val1);
static gboolean gemini_set (gushort id, gfloat val, gfloat val1, gfloat val2);
static gchar *chksum (gchar *string, gchar *c);
gdouble stof_RA (gchar *RA);
static gdouble stof_Dec (gchar *Dec);
static gchar *ftos_RA (gchar *string, gdouble RA);
static gchar *ftos_Dec (gchar *string, gdouble Dec);
static void mean_pos (gboolean *reset, gfloat pos[], guint size, 
					  enum MotionDirection dirn, gfloat val, gfloat *mean);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void telescope_init (void)
{
	/* Set up the pointer to the ccd and autoguider image structures - we need 
	 * these for guiding (and other things) later.
	 */
	
	ccd = get_ccd_image_struct ();
	aug = get_aug_image_struct ();
}

gboolean telescope_open_comms_port (void)
{
	/* Open the requested port for telescope communications */
	
	guint tel_status;
	
	L_print ("{b}****---->>>> Opening telescope comms link on "
	                                                "%s...\n", tel_comms->name);

	/* Check if port is already open - perhaps previously opened by something
	 * else.
	 */
	
	if (tel_comms->ref_count && tel_comms->user & PU_TEL) {
		L_print ("{b}Port already open for telescope communications!\n");
		L_print ("{b}****---->>>> Opened telescope link\n");
		return TRUE;
	}
	
	/* Open if not already open */
	
	if (!tel_comms->ref_count) {
		if (!serial_open_port (tel_comms, PU_TEL))
			return show_error (__func__, "Unable to open telescope "
							             "communications link");
	} else {
		tel_comms->ref_count++;
		tel_comms->user |= PU_TEL;
		L_print ("{b}Selected port %s is already open - sending telescope "
			                        "commands on same port\n", tel_comms->name);
	}
			
	/* Check to see if there is any response from the telescope */
	
	if (GeminiCmds) {
		tel_status = telescope_query_status (FALSE);
		if (tel_status == TEL_UNKNOWN) {
			telescope_close_comms_port ();
			return show_error (__func__, "No response from telescope - "
										"check port and serial cable!");
		} else {
			L_print("{b}Established communications with Gemini unit\n");
			
			/* Save the current telescope guiding and centering rates */
		
			if ((tel_status != TEL_START_WAIT) && SavedRates == FALSE) {
				telescope_save_motion_rates ();
				SavedRates = TRUE;
			}
		}
	}
			
	L_print ("{b}****---->>>> Opened telescope link\n");
	
	return TRUE;
}

void telescope_close_comms_port (void)
{
	/* Close down the communications link with the telescope if it's open and
	 * nothing else (e.g. autoguider commands) is using this link.
	 */
	 
	if (!tel_comms->ref_count)  /* Return if not open */
		return;
	
	if (tel_comms->user & PU_TEL) {
		tel_comms->ref_count--;   /* Decrease ref. count */
		tel_comms->user &= ~PU_TEL;
		L_print ("{b}****---->>>> Communications with telescope closed\n");
	}
	if (tel_comms->ref_count)    /* If still ref'd by someone else, return */
		return;
	
	serial_close_port (tel_comms);
}

gboolean telescope_open_autog_port (void)
{
	/* Open the requested serial port for autoguiding */
	
	L_print ("{b}****---->>>> Opening autoguider comms link on "
	                                              "%s...\n", autog_comms->name);
	
	/* Return if already open */
	
	if (autog_comms->ref_count && autog_comms->user & PU_AUTOG) {
		L_print ("{b}Port already open for autoguider communications!\n");
		L_print ("{b}****---->>>> Opened autoguider link\n");
		return TRUE;
	} 
	
	/* Open if not already open */
	
	if (!autog_comms->ref_count) {
		if (!serial_open_port (autog_comms, PU_AUTOG))
			return show_error (__func__, "Unable to open autoguider "
							             "communications link");
	} else {
		autog_comms->ref_count++;
		autog_comms->user |= PU_AUTOG;
		L_print ("{b}Selected port %s is already open - sending guide "
			                      "commands on same port\n", autog_comms->name);
	}
	
	/* Assign telescope guiding functions for this port */
	
	ports[autog_comms->pnum].guide_start = telescope_s_start;
	ports[autog_comms->pnum].guide_stop = telescope_s_stop;
	ports[autog_comms->pnum].guide_pulse = telescope_s_pulse;
	
	L_print ("{b}****---->>>> Opened autoguider link\n");

	return TRUE;
}

void telescope_close_autog_port (void)
{
	/* Close down the communications link with the autoguider if it's open and
	 * nothing else (e.g. telescope commands) is using this link.
	 */
	 
	if (!autog_comms->ref_count)  /* Return if not open */
		return;
	
	if (autog_comms->user & PU_AUTOG) {
		autog_comms->ref_count--;   /* Decrease ref. count */
		autog_comms->user &= ~PU_AUTOG;
		
		/* Reset telescope guiding function pointers */
	
		ports[autog_comms->pnum].guide_start = telescope_d_start;
		ports[autog_comms->pnum].guide_stop = telescope_d_stop;
		ports[autog_comms->pnum].guide_pulse = telescope_d_pulse;
		
		L_print ("{b}****---->>>> Communications with autoguider closed\n");
	}
	if (autog_comms->ref_count)    /* If still ref'd by someone else, return */
		return;
	
	serial_close_port (autog_comms);
}	

#ifdef HAVE_LIBPARAPIN
gboolean telescope_open_parallel_port (void)
{
	/* 'Open' the parallel port.  In practice, this means initialising the
	 * parapin library and setting the function pointers for autoguiding
	 * commands for this port.
	 */
	
	gint address;
	
	address = strtol (ports[LPT].address, NULL, 16);
	L_print ("{b}Initialising parallel port at address %x...\n", address);
	if (pin_init_user (address) < 0)
		return show_error (__func__, "Error initialising parallel port");
	L_print ("{b}Parallel port initialised OK\n");
	pin_output_mode (LP_DATA_PINS);
	
	ports[LPT].guide_start = telescope_p_start;
	ports[LPT].guide_stop = telescope_p_stop;
	ports[LPT].guide_pulse = telescope_d_pulse; /* No remotely-timed pulse */
	
	return TRUE;
}
#endif

gboolean telescope_close_parallel_port (void)
{
	/* 'Close' the parallel port.  In practice, there isn't anything to close
	 * when using the parapin library, so we just reset the autoguiding function
	 * pointers to their default state.
	 */
	
	ports[LPT].guide_start = telescope_d_start;
	ports[LPT].guide_stop = telescope_d_stop;
	ports[LPT].guide_pulse = telescope_d_pulse;
	
	return TRUE;
}

void telescope_move_start (enum TelMotion direction)
{
	/* Move the telescope in the desired direction, via LX200-compatible serial 
	 * commands to the telescope serial/USB port.  (For Gemini controllers at 
	 * least, the telescope will move at the presently selected rate).
	 */
	
	gchar *s = NULL;
	
	if (direction & TM_EAST)
		s = MOVE_EAST;
	else if (direction & TM_WEST)
		s = MOVE_WEST;
	else if (direction & TM_NORTH)
		s = MOVE_NORTH;
	else if (direction & TM_SOUTH)
		s = MOVE_SOUTH;
		
	s_write (tel_comms->f, s, 4);
}

void telescope_move_stop (enum TelMotion direction)
{
	/* Stop the telescope motion, via LX200-compatible serial commands to the 
	 * telescope serial/USB port.
	 */ 
	
	gchar *s = NULL;
	
	if (direction == TM_ALL)
		s = STOP_ALL;
	else if (direction & TM_EAST)
		s = STOP_EAST;
	else if (direction & TM_WEST)
		s = STOP_WEST;
    else if (direction & TM_NORTH)
		s = STOP_NORTH;
    else if (direction & TM_SOUTH)
		s = STOP_SOUTH;
		
	s_write (tel_comms->f, s, direction == TM_ALL ? 3 : 4);
}
	
void telescope_guide_start (enum TelMotion direction)
{
	/* Move the telescope in the desired direction at the guide speed */
	
	autog_comms->guide_start (direction);
}

void telescope_guide_stop (enum TelMotion direction)
{
	/* Stop the telescope motion */
	
	autog_comms->guide_stop (direction);
}

void telescope_guide_pulse (enum TelMotion direction, gint duration)
{
	/* Pulse-guide the telescope in the desired direction for the given duration
	 * with the pulse being timed remotely by hardware.
	 */
	 
	autog_comms->guide_pulse (direction, duration);
}

void telescope_d_start (enum TelMotion direction)
{
	/* Default empty function for starting telesope motion (this function is
	 * called if no other function pointers have been assigned).
	 */
	
	if (autog_comms->pnum == LPT)
		L_print ("{r}Parallel port not open or not available for "
				 "sending 'start' and 'stop' guiding commands\n");
	else if (autog_comms->pnum == USBAUG)
		L_print ("{r}Autoguider camera guide port not open, not available or "
				 "does not support 'start' and 'stop' guiding commands\n");
	else if (autog_comms->pnum == USBCCD)
		L_print ("{r}CCD camera guide port not open, not available or "
				 "does not support 'start' and 'stop' guiding commands\n");
	else
		L_print ("{r}Serial/USB port not open or not available for "
				 "sending 'start' and 'stop' guiding commands\n");
}

void telescope_d_stop (enum TelMotion direction)
{
	/* Default empty function for stopping telesope motion (this function is
	 * called if no other function pointers have been assigned).
	 */
	 
	/* No need to do anything in particular here */	
}

void telescope_d_pulse (enum TelMotion direction, gint duration)
{
	/* Default empty function for remotely-timed guide pulse (this function is
	 * called if no other function pointers have been assigned).
	 */
	
	if (autog_comms->pnum == LPT)
		L_print ("{r}Parallel port not open, not available or "
				 "does not support remotely-timed guide pulses\n");
	else if (autog_comms->pnum == USBAUG)
		L_print ("{r}Autoguider camera guide port not open, not available or "
				 "does not support remotely-timed guide pulses\n");
	else if (autog_comms->pnum == USBCCD)
		L_print ("{r}CCD camera guide port not open, not available or "
				 "does not support remotely-timed guide pulses\n");
	else
		L_print ("{r}Serial/USB port not open or not available for "
				 "sending remotely-timed guide pulses\n");
}

void telescope_s_start (enum TelMotion direction)
{
	/* Move the telescope in the desired direction, via LX200-compatible serial 
	 * commands to the autoguider serial/USB port.  (For Gemini controllers at 
	 * least, the telescope will move at the presently selected rate).
	 */
	
	gchar *s = NULL;
	
	if (direction & TM_EAST)
		s = MOVE_EAST;
	else if (direction & TM_WEST)
		s = MOVE_WEST;
	else if (direction & TM_NORTH)
		s = MOVE_NORTH;
	else if (direction & TM_SOUTH)
		s = MOVE_SOUTH;
		
	s_write (autog_comms->f, s, 4);
}

void telescope_s_stop (enum TelMotion direction)
{
	/* Stop the telescope motion, via LX200-compatible serial commands to the 
	 * autoguider serial/USB port.
	 */ 
	
	gchar *s = NULL;

	if (direction == TM_ALL)
		s = STOP_ALL;
	else if (direction & TM_EAST)
		s = STOP_EAST;
	else if (direction & TM_WEST)
		s = STOP_WEST;
    else if (direction & TM_NORTH)
		s = STOP_NORTH;
    else if (direction & TM_SOUTH)
		s = STOP_SOUTH;
	
	s_write (autog_comms->f, s, direction == TM_ALL ? 3 : 4);
}

void telescope_s_pulse (enum TelMotion direction, gint duration)
{
	/* Function for remotely-timed guide pulses via serial/USB port using the
	 * format :MgxNNNN# where x is the direction (e, w, n or s) and NNNN is the
	 * duration in milliseconds.
	 */
	
	gchar *s = NULL, *msec = NULL, *str = NULL;
	
	if (direction & TM_EAST)
		s = PULSE_EAST;
	else if (direction & TM_WEST)
		s = PULSE_WEST;
	else if (direction & TM_NORTH)
		s = PULSE_NORTH;
	else if (direction & TM_SOUTH)
		s = PULSE_SOUTH;
	
	msec = g_strdup_printf ("%i", duration);
	str = g_strconcat (s, msec, "#", NULL);
	s_write (autog_comms->f, str, (gint) g_utf8_strlen (str, -1));
	g_free (str);
	g_free (msec);
}

#ifdef HAVE_LIBPARAPIN
void telescope_p_start (enum TelMotion direction)
{
	/* Function for starting telescope motion via parallel port */
	
	if (direction & TM_EAST)
		set_pin (LP_PIN[ports[LPT].RAp]);
	else if (direction & TM_WEST)
		set_pin (LP_PIN[ports[LPT].RAm]);
    else if (direction & TM_NORTH)
		set_pin (LP_PIN[ports[LPT].Decp]);
	else if (direction & TM_SOUTH)
		set_pin (LP_PIN[ports[LPT].Decm]);
}
#endif

#ifdef HAVE_LIBPARAPIN
void telescope_p_stop (enum TelMotion direction)
{
	/* Function for stopping telescope motion via parallel port.  Note that if
	 * this function is called with direction == TM_ALL (i.e. all bits set),
	 * then each pin will be cleared in turn.
	 */
	
	if (direction & TM_EAST)
		clear_pin (LP_PIN[ports[LPT].RAp]);
	if (direction & TM_WEST)
		clear_pin (LP_PIN[ports[LPT].RAm]);
	if (direction & TM_NORTH)
		clear_pin (LP_PIN[ports[LPT].Decp]);
	if (direction & TM_SOUTH)
		clear_pin (LP_PIN[ports[LPT].Decm]);
}
#endif

static void thread_pool_guide_timing_func (gpointer data, gpointer user_data)
{
	/* Guide timing function.  If the guiding commands are issued via the main
	 * CCD camera, such commands may block for a significant time while the 
	 * camera is downloading an image.  Consequently, they will block in this 
	 * thread rather than the main gtk thread, where they would hang the 
	 * application while the image was being downloaded.
	 */
	 
	struct timespec length;
	guint now;
	
	switch GPOINTER_TO_INT (data) {
		case H:
			if (aug->autog.Write)
				telescope_write_guide_corr(TRUE, 
										  loop_elapsed_since_first_iteration (), 
										  pos.MOVE_H, pos.shift_duration_h);
			pos.elapsed_h = G_MAXUINT32; /*Artificially high till guiding done*/
			length.tv_sec = (time_t) pos.shift_duration_h / 1000.0;
			length.tv_nsec = 1e+06 * (pos.shift_duration_h - 1000.0 * 
																 length.tv_sec);
			if (aug->autog.s.RemoteTiming) {
				G_print ("Issuing guide correction in E/W direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				now = loop_elapsed_since_first_iteration ();
				autog_comms->guide_pulse (pos.MOVE_H, 
										 (gint) rintf (pos.shift_duration_h));
				if (loop_elapsed_since_first_iteration () - now < 
														   pos.shift_duration_h)
				/* Assume guide_pulse returned immediately; sleep here instead*/
					nanosleep (&length, NULL);
			} else {
				G_print ("Issuing guide start in E/W direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				autog_comms->guide_start (pos.MOVE_H);
				nanosleep (&length, NULL);
				G_print ("Issuing guide stop in E/W direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				autog_comms->guide_stop (pos.MOVE_H);
			}
			pos.elapsed_h = loop_elapsed_since_first_iteration ();
			if (aug->autog.Write)
				telescope_write_guide_corr (FALSE, pos.elapsed_h, pos.MOVE_H,0);
			if (!aug->exd.FreeRunning)
				loop_autog_exposure_wait (FALSE, H);
			pos.active_h = FALSE;
			break;
		case V:
			if (!aug->autog.s.SimulGuide) {/* Can't guide E/W,N/S at same time*/
				length.tv_sec = 0;
				length.tv_nsec = 1000;
				while (pos.active_h)      /* Spin while h correction is active*/
					nanosleep (&length, NULL);
			}
			if (aug->autog.Write)
				telescope_write_guide_corr(TRUE, 
										  loop_elapsed_since_first_iteration (), 
										  pos.MOVE_V, pos.shift_duration_v);
			pos.elapsed_v = G_MAXUINT32; /*Artificially high till guiding done*/
			length.tv_sec = (time_t) (pos.shift_duration_v / 1000.0);
			length.tv_nsec = 1e+06 * (pos.shift_duration_v - 1000.0 * 
																 length.tv_sec);
			if (aug->autog.s.RemoteTiming) {
				G_print ("Issuing guide correction in N/S direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				now = loop_elapsed_since_first_iteration ();
				autog_comms->guide_pulse (pos.MOVE_V, 
										 (gint) rintf (pos.shift_duration_v));
				if (loop_elapsed_since_first_iteration () - now < 
														   pos.shift_duration_v)
				/* Assume guide_pulse returned immediately; sleep here instead*/
					nanosleep (&length, NULL);
			} else {
				G_print ("Issuing guide start in N/S direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				autog_comms->guide_start (pos.MOVE_V);
				nanosleep (&length, NULL);
				G_print ("Issuing guide stop in N/S direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				autog_comms->guide_stop (pos.MOVE_V);
			}
			pos.elapsed_v = loop_elapsed_since_first_iteration ();
			if (aug->autog.Write)
				telescope_write_guide_corr (FALSE, pos.elapsed_v, pos.MOVE_V,0);
			if (!aug->exd.FreeRunning)
				loop_autog_exposure_wait (FALSE, V);
			pos.active_v = FALSE;
			break;
	}
}

void telescope_goto (gchar *RA, gchar *Dec)
{
	/* Issue GoTo command to the telescope */
	
	gchar *sRA, *sDec;
	gchar s[SER_BUFSIZ];
	
	sRA = g_strconcat (SET_RA, RA, "#", NULL);
	Dec[3] = '*';  /* Replace ':' with '*' */
	sDec = g_strconcat (SET_DEC, Dec, "#", NULL);
	
	s_write (tel_comms->f, sRA, 12);
	s_read (tel_comms->f, s, 1);
	L_print ("Set RA: %s", (!strncmp (s, "1", 1) ? "OK\n" : "failed\n"));
	g_free (sRA);	
	
	s_write (tel_comms->f, sDec, 13);
	s_read (tel_comms->f, s, 1);
	L_print ("Set Dec: %s", (!strncmp (s, "1", 1) ? "OK\n" : "failed\n"));
	g_free (sDec);	
	
	s_write (tel_comms->f, GOTO, 4);
	s_read (tel_comms->f, s, SER_BUFSIZ);
	if (strncmp (s, "0", 1))
		L_print ("{r}telescope_goto: Issued GoTo; controller returned %s\n", s);
}

gboolean telescope_goto_done (void)
{
	/* Test to see if the GoTo operation has completed:
	 * For Gemini mounts we query the status from the controller.
	 * For non-Gemini mounts, we compare positions from one 5-second period to 
	 * the next.  If the mount has moved less than a given number of arcsec over
	 * a 5-second period in both RA and Dec, we conclude that the mount has 
	 * finished slewing.  If an error occurs reading the RA or Dec, we switch to
	 * a timer mode and assume that slewing has stopped after 60s.
	 * NOTE: The timing of the 5s intervals is done in loop.c; this routine is
	 * called once every 5s, beginning 5s after the GoTo command has been
	 * issued.
	 */
	
	guint flags, now;
	static guint start_time = 0;
	gfloat arcsec;
	gdouble epoch, fRA, fDec;
	static gdouble RA = 9999.0, Dec = 9999.0;
	gchar sRA[9], sDec[10];
	gboolean OK_RA, OK_Dec;
	
	if (GeminiCmds) {
		flags = telescope_query_status (FALSE);
		if (flags & GOTO_ACTIVE)
			return FALSE;              /* Return if GoTo still active */
	} else {
		now = loop_elapsed_since_first_iteration (); /* milliseconds */
		if (!start_time) { /* Get RA and Dec coordinates */
			telescope_get_RA_Dec (FALSE, &epoch, sRA, sDec, 
								  &fRA, &fDec, &OK_RA, &OK_Dec);
			if (OK_RA && OK_Dec) {
				arcsec = get_goto_motion_limit ();
				G_print ("Motion limit is %.2f arcsec\n", arcsec);
				if (ABS (fRA - RA) > (arcsec / 15.0) / 3600.0 || 
									   ABS (fDec - Dec) > arcsec / 3600.0) {
					G_print ("RA is %f, RA difference is %.2f arc sec\n", 
							 RA, ABS (fRA - RA) * 3600.0 / 15.0);
					RA = fRA;
					G_print ("Dec is %f, Dec difference is %.2f arc sec\n",
							 Dec, ABS (fDec - Dec) * 3600.0);
					Dec = fDec;
					return FALSE;  /* RA or Dec motion too large */
				}
			} else {
				L_print ("{o}Error reading RA/Dec - using timer for GoTo "
						 " completion\n");
				start_time = now;
				return FALSE;
			}
		} else { /* Use timer instead */
			G_print("Using timer - elapsed: %ds\n",(now - start_time)/1000);
			if (now - start_time < 60 * 1000)  /* 60s */
				return FALSE;
		}
	}
	
	RA = 9999.0;
	Dec = 9999.0;
	start_time = 0;
	tasks_task_done (T_GTO);
	L_print ("{b}GoTo completed!\n");
	return TRUE;
}

void telescope_move_by (gdouble RA, gdouble Dec)
{
	/* Move the telescope by the given number of arcminutes in the RA and Dec
	 * directions.  We do this by finding the current pointing position, 
	 * adjusting the RA and Dec accordingly and then issuing a GoTo command to
	 * the new position.
	 * Note that we have to convert arcminutes in RA appropriate to the present 
	 * declination.
	 */
	
	gdouble epoch, fRA, fDec;
	gchar sRA[9], sDec[10];
	gboolean OK_RA, OK_Dec;
	
	if (telescope_get_RA_Dec (FALSE, &epoch, sRA, sDec, &fRA, &fDec, 
							  &OK_RA, &OK_Dec)) {
		if (OK_RA && OK_Dec) {
			fRA = fRA + RA / (60.0 * cos (fDec)) * M_PI / 180.0;
			fRA = fRA > 2.0 * M_PI ? fRA - 2.0 * M_PI : fRA;
			fRA = fRA < 0.0 ? fRA + 2.0 * M_PI : fRA;
			fDec = fDec + Dec / 60.0 * M_PI / 180.0;
			fDec = fDec > 90.0 * M_PI / 180.0 ? M_PI - fDec : fDec;
			fDec = fDec < -90.0 * M_PI / 180.0 ? -M_PI - fDec : fDec;
			telescope_goto (ftos_RA (sRA, fRA), ftos_Dec (sDec, fDec));
		} else {
			L_print ("{o}Error reading RA or Dec - can't move telescope!\n");
			tasks_task_done (T_GTO); /* Pretend it's worked in case task list */
		}                            /* has another go at some point!         */
	} else {
		L_print ("{o}Telescope link not open - can't execute Move task!\n");
	}
}

void telescope_warm_restart (void)
{
	/* Initiate a warm restart in the telescope controller.  Note that
	 * we need to close and then re-open the comms link to re-establish
	 * communications with the Gemini controller if it is in the 'warm restart' 
	 * state as a result of being powered off and then on again.
	 */
	
	guint response;
	gchar s[4];

	if (GeminiCmds) {
		telescope_close_comms_port ();
		if (telescope_open_comms_port ()) {
			response = telescope_query_status (FALSE);
			switch (response) {
				case TEL_START_WAIT:
					s_write (tel_comms->f, RESTART, 3);
					s_read (tel_comms->f, s, 1); /* Discard return value */
					break;
				case TEL_UNKNOWN:
					L_print ("{r}telescope_warm_restart: Can't restart "
							 "telescope - telescope status unknown\n");
					break;
				default:
					L_print ("{r}telescope_warm_restart: Can't restart "
							 "telescope - telescope already started?\n");
					tasks_task_done (T_WRT);
					break;
			}
		} else {
			reset_checkbox_state (RCS_OPEN_COMMS_LINK, FALSE);
			L_print ("{r}telescope_warm_restart: Can't warm restart - unable "
														"to open comms link\n");
		}
	}
}

gboolean telescope_warm_restart_done (void)
{
	/* Check if the telescope has been restarted */
	
	if (TEL_START_WAIT != telescope_query_status (FALSE)) {
		tasks_task_done (T_WRT);
		return TRUE;
	} else
		return FALSE;
}

void telescope_park_mount (void)
{
	/* Park the telescope mount */
	
	s_write (tel_comms->f, PARK, 4);
}

gboolean telescope_park_mount_done (void)
{
	/* Test to see if the park mount command has completed */
	
	gchar s[4];
	
	s_write (tel_comms->f, QUERY_PARK, 4);
	s_read (tel_comms->f, s, 1);
	
	if (!(tel_comms->user & PU_TEL)) { /* Don't loop forever if link not open */
		tasks_task_done (T_PMT);
		return TRUE;   /* Home move done */
	} else if (!strncmp (s, "0", 1)) {
		L_print ("{o}No telescope Home command received!\n");
		return FALSE;
	} else if (!strncmp (s, "1", 1)) {
		tasks_task_done (T_PMT);
		return TRUE;   /* Home move done */
	} else if (!strncmp (s, "2", 1)) {
		return FALSE;  /* Home move in progress */
	}
	return FALSE;
}

gboolean telescope_yellow_button (void)
{
	/* 'Press' the yellow button on the KIWI-OSD box via the relay box.  We 
	 * close the 'West' relay for 100 msec, with the relay box doing the
	 * timing.
	 */
	
	gchar *dirn;
	 
	/* It isn't a show-stopper if this fails for some reason.  It's a 
	 * nice-to-have but not worth interrupting the task list if there's a 
	 * problem.  So set the 'task completed' flag here.
	 */	 
	
	tasks_task_done (T_YBT);		
	 
	if (autog_comms->user & PU_TEL) {
		L_print ("{o}Autoguider comms port is same as telescope comms "
				 "port; won't be able to send command to relay box\n");
		return FALSE;
	}
	if (autog_comms->user & PU_AUTOG) {
		dirn = g_strconcat (PULSE_WEST, "100#", NULL);			
		s_write (autog_comms->f, dirn, (gint) g_utf8_strlen (dirn, -1));
		g_free (dirn);
		return TRUE;
	} else {
		L_print ("{o}Autoguider comms port not open; can't send command to "
				                                                 "relay box\n");
		return FALSE;
	}
}

void telescope_set_center_speed (gfloat speed)
{
	/* Set the centering speed to be used for the telescope motion commands, and
	 * set the centering speed to be the active speed.
	 */
	
	if (GeminiCmds)
		gemini_set (CMD_SPEED_CENTER, speed, 0, 0);
	s_write (tel_comms->f, RATE_CENTER, 4);
}

void telescope_set_guide_speed (gfloat speed)
{
	/* Set the guide speed to be used for the telescope motion commands, and set
	 * the guide speed to be the active speed.
	 */

	if (GeminiCmds)
		gemini_set (CMD_SPEED_GUIDE, speed, 0, 0);
	s_write (tel_comms->f, RATE_GUIDE, 4);
}

gboolean telescope_autog_calib (gfloat *calib_coords)
{
	/* Calculate the autoguider calibration.  Assume that all the star position 
	 * measurements are 'good'; i.e. not affected by hot pixels or other 
	 * brighter guide stars drifting into the field of view.  This can be made 
	 * more robust at a later date.
	 */
	
	gdouble pos[4][2];
	gdouble drift_ew = 0, drift_ns = 0;
	gdouble drift_n, drift_s, rot;
	gdouble fRA, fDec;
	gdouble epoch;
	gfloat val, ignore;
	gchar sRA[9], sDec[10];
	gboolean OK_RA, OK_Dec;
	
	/* Calculate the number of pixels moved in the east/west and north/south
	 * directions, and get the rate of motion. 
	 *
	 * The first and third positions are averaged because these are in principle
	 * the same (i.e. after the telescope has moved west and then back east 
	 * again by the same amount).  Averaging may reduce the effects of any drift
	 * due to periodic error during the E/W calibration.
	 *
	 * Stellar drift north (when the telescope moves south) or south (when the
	 * telescope moves north) is subject to backlash.  The telescope moves south
	 * initially and the user should ensure that any backlash in the southerly 
	 * direction has already been removed.  We adopt the following:
	 *
	 * 1. If the stellar drift north is greater than 110% of the stellar drift
	 *    south, use the drift north for the N/S calibration.
	 * 2. If the stellar drift south is greater than 110% of the stellar drift
	 *    north, use the drift south for the N/S calibration.
	 * 3. Otherwise, average the third and fifth positions since these are in
	 *    principle the same, after the telescope has moved south and back north
	 *    again.
	 */
	
	/* E/W */
	
	pos[0][0] = (*(calib_coords) + *(calib_coords + 4))/2.0;
	pos[0][1] = (*(calib_coords + 1) + *(calib_coords + 5))/2.0;
	pos[1][0] = *(calib_coords + 2);
	pos[1][1] = *(calib_coords + 3);
	
	drift_ew = sqrt ((pos[1][0] - pos[0][0]) * (pos[1][0] - pos[0][0]) + 
					 (pos[1][1] - pos[0][1]) * (pos[1][1] - pos[0][1]));
	
	/* N/S */	
	
	drift_n = sqrt ((*(calib_coords + 5) - *(calib_coords + 7)) *
	                (*(calib_coords + 5) - *(calib_coords + 7)) +
	                (*(calib_coords + 4) - *(calib_coords + 6)) *
	                (*(calib_coords + 4) - *(calib_coords + 6)));
	drift_s = sqrt ((*(calib_coords + 9) - *(calib_coords + 7)) *
	                (*(calib_coords + 9) - *(calib_coords + 7)) +
	                (*(calib_coords + 8) - *(calib_coords + 6)) *
	                (*(calib_coords + 8) - *(calib_coords + 6)));
	
    if (drift_n / drift_s > 1.1) {
		pos[2][0] = *(calib_coords + 4);
		pos[2][1] = *(calib_coords + 5);
		pos[3][0] = *(calib_coords + 6);
		pos[3][1] = *(calib_coords + 7);
		drift_ns = drift_n;
	} else if (drift_s / drift_n > 1.1) {
		pos[2][0] = *(calib_coords + 6);
		pos[2][1] = *(calib_coords + 7);
		pos[3][0] = *(calib_coords + 8);
		pos[3][1] = *(calib_coords + 9);
		drift_ns = drift_s;
	} else {
		pos[2][0] = (*(calib_coords + 4) + *(calib_coords + 8))/2.0; 
		pos[2][1] = (*(calib_coords + 5) + *(calib_coords + 9))/2.0; 
		pos[3][0] = *(calib_coords + 6);
		pos[3][1] = *(calib_coords + 7);
		drift_ns = sqrt ((pos[3][0] - pos[2][0]) * (pos[3][0] - pos[2][0]) + 
						 (pos[3][1] - pos[2][1]) * (pos[3][1] - pos[2][1]));
	}
	
	aug->autog.s.Rate_EW = drift_ew / aug->autog.s.EWCalibDuration;
	aug->autog.s.Rate_NS = drift_ns / aug->autog.s.NSCalibDuration;
	
	L_print ("{b}Star moved %6.2f pixels E/W at %6.2f pixels/s\n", 
			                                    drift_ew, aug->autog.s.Rate_EW);
	L_print ("{b}Star moved %6.2f pixels N/S at %6.2f pixels/s\n", 
			                                    drift_ns, aug->autog.s.Rate_NS);
	
	/* Ideally, the east/west drift should define the rotation of the camera
	 * since the north/south drift might be influenced by periodic error
	 * during the course of the north/south measurement.  But for observations
	 * close to the pole, the east/west drift might be very small, so assume
	 * the following:
	 * 1. If the east/west drift is greater than half the north/south drift,
	 *    then use the east/west drift to define the rotation of the camera;
	 *    otherwise use the north/south drift.
	 * 2. Ignore the effects of drifts due to periodic error during the north/
	 *    south drift measurement.
	 * 3. Assume that the east/west and north/south drifts are orthogonal.
	 *    Hence the direction of north/south drift is calculated based on the 
	 *    direction of the east/west drift (if the east/west drift is used
	 *    to define the rotation of the camera) and vice versa.
	 */
	
	if (drift_ew > 0.5 * drift_ns) {
		
		/* Calculate the unit length east direction vector.  Note that when
		 * the telescope moves west during the first calibration step, then
		 * the star will appear to move east.
		 */
		
		aug->autog.s.Uvec_E[0] = (pos[1][0] - pos[0][0]) / drift_ew;
		aug->autog.s.Uvec_E[1] = (pos[1][1] - pos[0][1]) / drift_ew;
		
		/* Derive the unit length north vector as being the vector perpendicular
		 * to this one.  There are two choices so we pick the one in the same
		 * direction as the motion of the star when it moved north (i.e. when 
		 * the telescope moved south).
		 */
		
		aug->autog.s.Uvec_N[0] = -aug->autog.s.Uvec_E[1];
		aug->autog.s.Uvec_N[1] = aug->autog.s.Uvec_E[0];
		
		/* Vector dot product to see if vectors are in the same or opposite */
		/* directions.                                                      */
		if (aug->autog.s.Uvec_N[0] * (pos[3][0] - pos[2][0]) +
			aug->autog.s.Uvec_N[1] * (pos[3][1] - pos[2][1]) < 0) {
			aug->autog.s.Uvec_N[0] = aug->autog.s.Uvec_E[1];
			aug->autog.s.Uvec_N[1] = -aug->autog.s.Uvec_E[0];
		}
		
	} else {
		
		/* Calculate the unit length north direction vector.  Note that when
		 * the telescope moves south during the first calibration step, then
		 * the star will appear to move north.
		 */
		
		aug->autog.s.Uvec_N[0] = (pos[3][0] - pos[2][0]) / drift_ns;
		aug->autog.s.Uvec_N[1] = (pos[3][1] - pos[2][1]) / drift_ns;
		
		/* Derive the unit length east vector as being the vector perpendicular
		 * to this one.  There are two choices so we pick the one in the same
		 * direction as the motion of the star when it moved east (i.e. when 
		 * the telescope moved west).
		 */
		
		aug->autog.s.Uvec_E[0] = -aug->autog.s.Uvec_N[1];
		aug->autog.s.Uvec_E[1] = aug->autog.s.Uvec_N[0];
		
		/* Vector dot product to see if vectors are in the same or opposite */
		/* directions.                                                      */
		if (aug->autog.s.Uvec_E[0] * (pos[1][0] - pos[0][0]) +
			aug->autog.s.Uvec_E[1] * (pos[1][1] - pos[0][1]) < 0) {
			aug->autog.s.Uvec_E[0] = aug->autog.s.Uvec_N[1];
			aug->autog.s.Uvec_E[1] = -aug->autog.s.Uvec_N[0];
		}
	}
	
	rot = atan (aug->autog.s.Uvec_E[1] / aug->autog.s.Uvec_E[0]);
	/* Swap sign since v-coord is inverted for display purposes */
	L_print ("{b}Camera is rotated by %7.4f degrees\n", -57.29578 * rot);  

	/* Show the rotated cross-hair */
	
    ui_set_augcanv_crosshair (aug->exd.h_top_l + aug->exd.h_pix / 2.0, 
                              aug->exd.v_top_l + aug->exd.v_pix / 2.0);
	
	/* If a declination correction is to be applied to the E/W rate, store the
	 * declination at which the calibration was done.
	 */
	
	if (aug->autog.s.DecCorr) {
		L_print ("{b}Getting declination at which calibration was done...\n");
		aug->autog.s.CalibDec = 999.0; /* An 'error' value */
		if (telescope_get_RA_Dec (FALSE, &epoch, sRA, sDec, 
								  &fRA, &fDec, &OK_RA, &OK_Dec)) {
			if (OK_Dec)
				aug->autog.s.CalibDec = (gfloat) fDec;
		}
	}
	
	/* If Gemini commands are enabled, store the guide rate that was used */
	
	if (tel_comms->user & PU_TEL && GeminiCmds) {
		L_print ("{b}Getting guide rate used for calibration...\n");
		gemini_get (CMD_SPEED_GUIDE, &val, &ignore);
		if (val > 0.01) {
			aug->autog.s.CalibGuideSpeed = val;
			L_print ("{b}Guide rate was %3.1f\n", aug->autog.s.CalibGuideSpeed);
		} else
			aug->autog.s.CalibGuideSpeed = 999.0; /* An 'error' value */
	}

	/* All done */
	
	set_autog_calibrate_done ();
	
	return TRUE;
}

gboolean telescope_guide (enum Guide status, guint elapsed)
{
	/* Guide the telescope to keep the star centered at the initial position.
	 * When called with status = INIT, returns FALSE if initialisation fails.
	 * When called with status = GUIDE, returns FALSE unless an autoguider
	 * image has been successfully analysed and a guide correction is not
	 * required.
	 */
	
	gfloat Correction = 0.0;
	gdouble fRA, fDec;
	gdouble epoch;
	gchar sRA[9], sDec[10];
	gchar *CorrectionsPath;
	gboolean OK_RA, OK_Dec;
	static gboolean WarnDisappear = TRUE, WarnOffset = TRUE;
	
	switch (status) {
		
		/* Initialise some values */
		
		case INIT:
			L_print ("Beginning autoguiding...\n");
		
			/* Open the file for logging guiding corrections, in case needed */
	
			CorrectionsPath = g_strconcat (PrivatePath, "/guide_corr.csv",NULL);
			if (!(fg = fopen (CorrectionsPath, "a"))) {
				g_free (CorrectionsPath);
				fg = NULL;
				L_print ("{r}Error opening guiding corrections file");
			}
			g_free (CorrectionsPath);
			if (aug->autog.Write)
				telescope_write_guidecorr_time ();
			
			/* Get guiding parameters */
		
			get_autog_guide_params ();
			
			pos.shift_rate_h = aug->autog.s.Rate_EW;
			pos.shift_rate_v = aug->autog.s.Rate_NS;
			
			telescope_set_guide_speed (aug->autog.s.GuideSpeed);
			
			if (aug->autog.s.DecCorr) {
				if (telescope_get_RA_Dec (FALSE, &epoch, sRA, sDec, 
										        &fRA, &fDec, &OK_RA, &OK_Dec)) {
					if (OK_Dec) {
						L_print ("Current declination is %6.2f degrees\n", 
								 57.2958 * fDec);
						pos.shift_rate_h = pos.shift_rate_h * cos (fDec);
					}
				} else {
					msg ("Warning - Unable to get telescope declination.\n"
				         "Guiding may not be accurate.");
				}
				if (aug->autog.s.CalibDec > 90.0) { /* An error value */
					msg ("Warning - Don't know declination used for "
						 "calibration.\nGuiding may not be accurate.");
				} else {
					L_print ("Calibration was done at declination %6.2f "
							 "degrees\n", 57.2958 * aug->autog.s.CalibDec);
					pos.shift_rate_h = pos.shift_rate_h / 
					                                cos (aug->autog.s.CalibDec);
				}
			}
			
			if (tel_comms->user & PU_TEL && GeminiCmds) {
				L_print ("Current guide speed is %3.1f\n",
						                               aug->autog.s.GuideSpeed);
				if (aug->autog.s.CalibGuideSpeed > 1.0) { /* An error value */
					msg ("Warning - Don't know guide speed used for "
						 "calibration.\nGuiding may not be accurate.");
				} else {
					L_print ("Calibration was done at guide speed of %3.1f\n",
						                          aug->autog.s.CalibGuideSpeed);
					pos.shift_rate_h = pos.shift_rate_h * 
				         aug->autog.s.GuideSpeed / aug->autog.s.CalibGuideSpeed;
				}
			}
					
			L_print ("Calibrated E/W guide rate was %6.2f pixels/s; now using "
					 "%6.2f pixels/s\n", aug->autog.s.Rate_EW,pos.shift_rate_h);
					 
			/* Set initial guide star position and peak value */
		
			pos.init_h = aug->rect.mean[GREY].h;
			pos.init_v = aug->rect.mean[GREY].v;
		
			pos.maxval = aug->rect.max[GREY].val;
			
			/* Report the position to the user, remembering to invert the v-axis
			 * coordinate.
			 */
			
			L_print ("Initial position of guide star is h: %f, v: %f; "
			          "peak value is %d\n", pos.init_h + 1, 
			           aug->exd.v_pix - pos.init_v, pos.maxval);			
		
			/* Set remaining initial values */
			
			pos.MOVE_H = TM_NONE;
			pos.MOVE_V = TM_NONE;
			pos.GuideCorr = SD_NONE;
			pos.NumImg = 0;
			pos.NumGuide = 0;
		    /* The conversion from drift sample time to the number of samples */
		    /* is simplistic, but probably achieves what the user wants.      */
			pos.min_num = MAX (1, (guint) (aug->autog.s.DriftSample /
								                             aug->exd.req_len));
			pos.pos_h = (gfloat *) g_malloc0 (pos.min_num * sizeof (gfloat));
			pos.pos_v = (gfloat *) g_malloc0 (pos.min_num * sizeof (gfloat));
			pos.reset_h = TRUE;
			pos.reset_v = TRUE;	
		    pos.wait_h = FALSE;
		    pos.wait_v = FALSE;
		    pos.offset_h = 0.0;
		    pos.offset_v = 0.0;
			pos.e = pos.w = pos.n = pos.s = 0;
			/* +0.5 places cross-hair at centre of centroid marker! */
			ui_set_augcanv_crosshair (pos.init_h + aug->exd.h_top_l + 0.5,
			                          pos.init_v + aug->exd.v_top_l + 0.5);
			aug->autog.Guide = TRUE;
			aug->canv.NewRect = TRUE; /* Reinitialise RMS calculation */
			
			/* Start guide timing thread pool */
			
			thread_pool_guide_timing = g_thread_pool_new (
							                    &thread_pool_guide_timing_func, 
											    NULL,
											    2, 
												TRUE, 
												NULL);
			
			tasks_task_done (T_GST);
			break;
		
		/* Issue guiding commands based on the movement of the star over the
		 * previous interval.
		 */
			
		case GUIDE:
			G_print ("\nGuiding at %d...\n", elapsed);
			
			/* For free-running devices, wait until the guide corrections have
			 * finished, then additionally wait two exposure lengths.
			 * (Suppose a guide correction finishes just after a free-running 
			 * exposure starts.  Then that exposure will be 'contaminated' with
			 * the stellar motion resulting from the guide correction, so we 
			 * want the position of the star during the next exposure, which 
			 * isn't measured until that next exposure has ended).
			 * 
			 * For timed exposures, we just have to wait until the most recent
			 * image start time is later than the finish time of the last guide 
			 * correction.  This works for timed exposures because this function
			 * isn't called while a timed exposure is in progress, so any 
			 * exposure that started after the last guide correction must also 
			 * have finished by this point.
		     */
		
		    if (aug->exd.FreeRunning) {
				if (elapsed < MAX (pos.elapsed_h, pos.elapsed_v) +
								   2000.0 * aug->exd.req_len)
				return FALSE;
			} else {
				if (aug->exd.exp_start < MAX (pos.elapsed_h, pos.elapsed_v))
					return FALSE;
			}
		
			/* Reject frames where the star has disappeared */

			if (isnan(aug->rect.mean[GREY].h) || isnan(aug->rect.mean[GREY].v)){
				if (WarnDisappear) {
					L_print ("{o}Warning - guide star signal is too weak for "
							 "guiding\n");
					loop_display_blinkrect (TRUE);
					WarnDisappear = FALSE;
				}
				return FALSE;
			}
			WarnDisappear = TRUE;

			/* Calculate shifts of guide star from initial position */
			
			pos.shift_h = (aug->rect.mean[GREY].h - pos.init_h) * 
						   aug->autog.s.Uvec_E[0] +
						  (aug->rect.mean[GREY].v - pos.init_v) * 
						   aug->autog.s.Uvec_E[1];
			pos.shift_v = (aug->rect.mean[GREY].h - pos.init_h) *
						   aug->autog.s.Uvec_N[0] +
						  (aug->rect.mean[GREY].v - pos.init_v) *
						   aug->autog.s.Uvec_N[1];
			
			/* Reject frames where the star has moved more than maximum 
			 * permitted amount.
			 */
			
			if (ABS (pos.shift_h) > aug->autog.s.MaxOffset ||
				ABS (pos.shift_v) > aug->autog.s.MaxOffset) {
				if (ABS (pos.shift_h) > aug->autog.s.MaxOffset)	
					G_print ("Ignoring E/W offset of %10.4f - more than %5.2f "
							 "pixels\n", pos.shift_h, aug->autog.s.MaxOffset);
			    if (ABS (pos.shift_v) > aug->autog.s.MaxOffset)
					G_print ("Ignoring N/S offset of %10.4f - more than %5.2f "
						     "pixels\n", pos.shift_v, aug->autog.s.MaxOffset);
				if (WarnOffset) {
					L_print ("{o}Warning - guide star has moved further than "
							 "maximum offset\n");
					loop_display_blinkrect (TRUE);
					WarnOffset = FALSE;
				}
				return FALSE;
			}
			WarnOffset = TRUE;
			loop_display_blinkrect (FALSE);
			
			/* Now sort out which shift/drift corrections need to be done and 
			 * for how long.
			 */
			
			pos.GuideCorr = SD_NONE;
			pos.MOVE_H = TM_NONE;
			pos.MOVE_V = TM_NONE;
			pos.wait_h = FALSE;
			pos.wait_v = FALSE;
			pos.offset_h = 0.0;
			pos.offset_v = 0.0;
			Correction = aug->autog.s.CorrFac;	
			pos.NumImg++;  /* No. images for which we might make a correction */
			
			if ((gfloat)(elapsed - pos.elapsed_h)<aug->autog.s.Update * 1000.0){
				G_print ("Waiting for E/W Update period to elapse\n");
				pos.wait_h = TRUE;
			} else {
				if (ABS (pos.shift_h) > aug->autog.s.MaxShift) { /* Shifts    */
					pos.offset_h = pos.shift_h;              /* take priority */
					pos.min_offset = aug->autog.s.MaxShift;
					pos.GuideCorr |= SHIFT_H;
				} else {
					mean_pos (&pos.reset_h, pos.pos_h, pos.min_num, 
											      H, pos.shift_h, &pos.drift_h);
					if (ABS (pos.drift_h) > aug->autog.s.MaxDrift) {
						pos.offset_h = pos.drift_h;
						pos.min_offset = aug->autog.s.MaxDrift;
						pos.GuideCorr |= DRIFT_H;
					}
				}
				G_print ("E/W shift from initial position is: %10.4f pixels\n", 
																   pos.shift_h);
				G_print ("E/W drift from initial position is: %10.4f pixels\n", 
																   pos.drift_h);
				if (pos.GuideCorr & (SHIFT_H | DRIFT_H)) {
					
					G_print ("Required correction is %10.4f x %5.2f = %10.4f "
							 "pixels\n", ABS (pos.offset_h), Correction,
							 ABS (pos.offset_h) * Correction);
					if (ABS (pos.offset_h) * Correction > aug->autog.s.MaxMove){
						G_print ("Restricting to maximum permitted value of "
										"%5.2f pixels\n", aug->autog.s.MaxMove);
						pos.offset_h = pos.offset_h > 0 ? 
								  aug->autog.s.MaxMove : -aug->autog.s.MaxMove;
						pos.offset_h /= Correction;  /* Gets multiplied by    */
					}                                /* Correction below!     */
				
					pos.MOVE_H = pos.offset_h > 0.0 ? TM_EAST : TM_WEST;
					if (pos.MOVE_H & aug->autog.s.GuideDirn) {
						pos.shift_duration_h = ABS (pos.offset_h) * Correction * 
													  1000.0 / pos.shift_rate_h;
						G_print ("E/W offset is greater than %5.2f pixels; "
								 "making %s correction %s for %10.4f "
								 "milliseconds\n", 
								 pos.min_offset,
								 pos.GuideCorr & SHIFT_H ? "shift" : "drift",
								 pos.MOVE_H & TM_EAST ? "East" : "West",
								 pos.shift_duration_h);
					} else {
						G_print ("*** Moving telescope %s disabled ***\n",
								 pos.MOVE_H & TM_EAST ? "East" : "West");
						pos.GuideCorr &= ~(SHIFT_H | DRIFT_H);
					}
				}
			}				
		
			if ((gfloat)(elapsed - pos.elapsed_v)<aug->autog.s.Update * 1000.0){
				G_print ("Waiting for N/S Update period to elapse\n");
				pos.wait_v = TRUE;
			} else {
				if (ABS (pos.shift_v) > aug->autog.s.MaxShift &&
									!aug->autog.s.DriftNSOnly) { /* Shifts    */
					pos.offset_v = pos.shift_v;              /* take priority */
					pos.min_offset = aug->autog.s.MaxShift;
					pos.GuideCorr |= SHIFT_V;
				} else {
					mean_pos (&pos.reset_v, pos.pos_v, pos.min_num, 
												  V, pos.shift_v, &pos.drift_v);
					if (ABS (pos.drift_v) > aug->autog.s.MaxDrift) {
						pos.offset_v = pos.drift_v;
						pos.min_offset = aug->autog.s.MaxDrift;
						pos.GuideCorr |= DRIFT_V;
					}
				}
				if (!aug->autog.s.DriftNSOnly) {
					G_print ("N/S shift from initial position is: "
												"%10.4f pixels\n", pos.shift_v);
				} else {
					G_print ("N/S shift ignored - making drift "
														  "corrections only\n");
				}
				G_print ("N/S drift from initial position is: "
												"%10.4f pixels\n", pos.drift_v);
												
				if (pos.GuideCorr & (SHIFT_V | DRIFT_V)) {
					
					G_print("Required correction is %10.4f x %5.2f = %10.4f "
							"pixels\n", ABS (pos.offset_v), Correction,
							ABS (pos.offset_v) * Correction);
					if (ABS (pos.offset_v) * Correction > aug->autog.s.MaxMove){
						G_print ("Restricting to maximum permitted value of "
										"%5.2f pixels\n", aug->autog.s.MaxMove);
						pos.offset_v = pos.offset_v > 0 ? 
								  aug->autog.s.MaxMove : -aug->autog.s.MaxMove;
						pos.offset_v /= Correction;  /* Gets multiplied by    */
					}                                /* Correction below!     */
				
					pos.MOVE_V = pos.offset_v > 0.0 ? TM_NORTH : TM_SOUTH;
				    if (pos.MOVE_V & aug->autog.s.GuideDirn) {
						pos.shift_duration_v = ABS (pos.offset_v) * Correction * 
				                                      1000.0 / pos.shift_rate_v;
						G_print ("N/S offset is greater than %5.2f pixels; "
								 "making %s correction %s for %10.4f "
								 "milliseconds\n", 
								 pos.min_offset,
								 pos.GuideCorr & SHIFT_V ? "shift" : "drift",
								 pos.MOVE_V & TM_NORTH ? "North" : "South", 
								 pos.shift_duration_v);
					} else {
						G_print ("*** Moving telescope %s disabled ***\n",
								 pos.MOVE_V & TM_NORTH ? "North" : "South");
						pos.GuideCorr &= ~(SHIFT_V | DRIFT_V);
					}
				}				
			}				
			
			/* Set selection rectangle colour */
			
			if (pos.GuideCorr & (SHIFT_H | SHIFT_V))
				ui_set_augcanv_rect_colour ("red");
			else if (pos.GuideCorr & (DRIFT_H | DRIFT_V))
				ui_set_augcanv_rect_colour ("magenta");
			else {
				if (pos.wait_h || pos.wait_v) {
					ui_set_augcanv_rect_colour ("orange"); 
					return FALSE;  /* Wait till end of update period */
				} else {
					G_print ("No correction required: autoguider idle\n");
					ui_set_augcanv_rect_colour ("green"); 
					return TRUE;   /* No corrections needed, so return */
				} 
			} 
			
			/* Issue guide corrections */
			
			if (pos.GuideCorr & (SHIFT_H | DRIFT_H)) {
				G_print ("Requesting guide correction in E/W direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				if (!aug->exd.FreeRunning)
					loop_autog_exposure_wait (TRUE, H);
				pos.active_h = TRUE;
				g_thread_pool_push (thread_pool_guide_timing, 
									GINT_TO_POINTER (H), NULL);
				pos.MOVE_H & TM_EAST ? pos.e++ : pos.w++;
				pos.reset_h = TRUE;
			}
			if (pos.GuideCorr & (SHIFT_V | DRIFT_V)) {
				G_print ("Requesting guide correction in N/S direction at %d\n",
										 loop_elapsed_since_first_iteration ());
				if (!aug->exd.FreeRunning)
					loop_autog_exposure_wait (TRUE, V);
				pos.active_v = TRUE;
				g_thread_pool_push (thread_pool_guide_timing, 
									GINT_TO_POINTER (V), NULL);
				pos.MOVE_V & TM_NORTH ? pos.n++ : pos.s++;
				pos.reset_v = TRUE;
			}
				
			/* Increment the number of images for which a guide correction was
			 * made.
			 */
			
			pos.NumGuide++;
			
			return FALSE;				
			break;
			
		/* Pause issuing guiding commands (but retain initial guide star
		 * position).
		 */
			
		case PAUSE:
			L_print ("Paused autoguiding...\n");
			pos.MOVE_H = TM_NONE;
			pos.MOVE_V = TM_NONE;
			pos.GuideCorr = SD_NONE;
			telescope_guide_stop (TM_ALL);
			aug->autog.Pause = TRUE;
			break;
		
		/* Continue after pause */
		
		case CONT:
			L_print ("Continuing autoguiding...\n");
			aug->autog.Pause = FALSE;
			break;
			
		/* Stop all telescope motion and reset the cross-hairs */
			
		case QUIT:
			telescope_guide_stop (TM_ALL);
            ui_set_augcanv_crosshair (aug->exd.h_top_l + aug->exd.h_pix / 2.0, 
                                      aug->exd.v_top_l + aug->exd.v_pix / 2.0);
		    loop_display_blinkrect (FALSE);
			if (pos.pos_h != NULL) {
				g_free (pos.pos_h);
				pos.pos_h = NULL;
			}
			if (pos.pos_v != NULL) {
				g_free (pos.pos_v);
				pos.pos_v = NULL;
			}
			if (fg) {
				fclose (fg); 	/* Close the guiding corrections file */
				fg = NULL;
			}
			
			if (tel_comms->user & PU_TEL && GeminiCmds) 
				telescope_restore_motion_rates ();
			ui_set_augcanv_rect_colour ("green");
			L_print ("Quitting autoguiding...\n");
			L_print ("Guide corrections were: north [%d], south [%d], "
					 "east [%d], west [%d]\n", pos.n, pos.s, pos.e, pos.w);
			L_print ("Number of images that required guiding was %5.2f "
					 "percent\n\n", 100.0 * (gfloat) pos.NumGuide / 
													   (gfloat) pos.NumImg);
			aug->autog.Guide = FALSE;
			
			/* Free the guide timing thread pool */
			
			g_thread_pool_free (thread_pool_guide_timing, TRUE, TRUE);

			tasks_task_done (T_GSP);		
			break;
	}
	return TRUE;
}

gboolean telescope_write_guidecorr_time (void)
{
	/* Write the date and time to the guide corrections file */
	
	struct tm *dt;
	gchar *date;		
	
	dt = get_time (UseUTC);
	date = g_strdup_printf ("%4i-%02i-%02iT%02i:%02i:%02i", 
	                        1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
	                        dt->tm_hour, dt->tm_min, dt->tm_sec);	
	
	if (fg) {
		if ((fprintf (fg, "\n%f,   %s\n",loop_elapsed_since_first_iteration () / 
	                                                        1000.0, date) == 0))
			goto error;

		if ((fprintf (fg, "     sec.,  E/W (-1/+1),  N/S (+1/-1),  "
				                                         "length (ms)\n") == 0))
			goto error;
	}
	
	g_free (date);
	return TRUE;

error:

	g_free (date);
	return show_error (__func__, "Error writing to guide corrections file");	
}

static gboolean telescope_write_guide_corr (gboolean move, guint now,
										enum TelMotion direction, gint duration)
{
	/* Write guide corrections to a file.  We write the previous state followed
	 * by the existing state so that we get a square-wave when plotted in a
	 * spreadsheet.
	 */
	
	static gint prev_H_direction = 0, prev_V_direction = 0;
	gfloat fnow;
	
	fnow = now / 1000.0;
	
	if (fg) {
		if ((fprintf (fg, "%9.3f,  %11i,  %11i,  %11i\n", 
				      fnow, prev_H_direction, prev_V_direction, duration)) == 0)
			goto error;
	
	    switch (direction) {
		
			case TM_NONE:
				break;
		
			case TM_EAST:
				if ((fprintf (fg, "%9.3f,  %11i,  %11i,  %11i\n", 
						        fnow, move ? -1 : 0, prev_V_direction, 0)) == 0)
					goto error;
				prev_H_direction = move ? -1 : 0;						
				break;
			
			case TM_WEST:
				if ((fprintf (fg, "%9.3f,  %11i,  %11i,  %11i\n", 
						        fnow, move ? 1 : 0, prev_V_direction, 0)) == 0)
					goto error;
				prev_H_direction = move ? 1 : 0;						
				break;
			
				case TM_NORTH:
					if ((fprintf (fg, "%9.3f,  %11i,  %11i,  %11i\n", 
						         fnow, prev_H_direction, move ? 1 : 0, 0)) == 0)
						goto error;
					prev_V_direction = move ? 1 : 0;
					break;
			
		
				case TM_SOUTH:
					if ((fprintf (fg, "%9.3f,  %11i,  %11i,  %11i\n", 
						        fnow, prev_H_direction, move ? -1 : 0, 0)) == 0)
						goto error;
					prev_V_direction = move ? -1 : 0;
					break;
		
				case TM_ALL:
					break;
		}
	}

	return TRUE;
	
error:
	return show_error (__func__, "Error writing to guide corrections file");	
}

gboolean telescope_get_RA_Dec (gboolean precess, gdouble *epoch,
	                           gchar *sRA, gchar *sDec,
                               gdouble *fRA, gdouble *fDec,
							   gboolean *OK_RA, gboolean *OK_Dec)
{
	/* Return the RA and Dec coordinates from the telescope if the link is open,
	 * otherwise return 00:00:00.  Precess coordinates to epoch 2000.0 if
	 * requested.
	 */
	
	struct tm *dt;
	GDate *date1 = g_date_new ();
	GDate *date2 = g_date_new ();
	gushort i;
	gdouble dRA, dDec;
	gdouble now, julian_diff, year_diff;
	
	const gdouble m = 46.125;  /* Precession constants in arcsec */
	const gdouble n = 20.042;
	
	*OK_RA = FALSE;
	*OK_Dec = FALSE;
	sprintf (sRA, "00:00:00");
	sprintf (sDec, "+00:00:00");
	*fRA = 0.0;
	*fDec = 0.0;
	*epoch = 2000.0;
	
	if (!(tel_comms->user & PU_TEL)) {
		L_print ("{o}Telescope link not open; returning 00:00:00 "
		                                                    "for RA and Dec\n");
		return FALSE;
	} else {

		s_write (tel_comms->f, GET_RA, 4);
		s_read (tel_comms->f, sRA, 9);
		
		sRA[8] = '\0'; /* Over-write last character which for Gemini is hash */
		L_print ("Wrote %.4s to controller, read: %s\n", GET_RA, sRA);
		
		*OK_RA = TRUE;
		for (i = 0; i < 8; i++) { /* Check for rubbish */
			if (i == 2 || i == 5) {
				if (sRA[i] != ':')
					*OK_RA = FALSE;
			} else {
				if (!isdigit (sRA[i]))
					*OK_RA = FALSE;
			}
		}
		if (*OK_RA) {
		    *fRA = stof_RA (sRA);
		} else {
			L_print ("{o}Warning - bad response from telescope controller. "
					 "Returning 00:00:00 for RA\n");
			sprintf (sRA, "00:00:00");
		}
		
		s_write (tel_comms->f, GET_DEC, 4);
		s_read (tel_comms->f, sDec, 10);
		
		sDec[9] = '\0'; /* Over-write last character which for Gemini is hash */
		L_print ("Wrote %.4s to controller, read: %s\n", GET_DEC, sDec);
		
		*OK_Dec = TRUE;
		for (i = 0; i < 9; i++) { /* Check for rubbish */
			if (i == 0) {
				if (sDec[i] != '+' && sDec[i] != '-')
				*OK_Dec = FALSE;
			} else if (i == 3) {
				if (sDec[i] != ':' && sDec[i] != '*')
					*OK_Dec = FALSE;
			} else if (i == 6) {
				if (sDec[i] != ':')
					*OK_Dec = FALSE;
			} else {
				if (!isdigit (sDec[i]))
					*OK_Dec = FALSE;
			}
		}
		if (*OK_Dec) {
			sDec[3] = ':';  /* Make it ':' rather than '*' */
			*fDec = stof_Dec (sDec);
		} else {
			L_print ("{o}Warning - bad response from telescope controller. "
					 "Returning +00:00:00 for Dec\n");
			sprintf (sDec, "+00:00:00");
		}
		
		dt = get_time (TRUE);
		date1 = g_date_new_dmy (1, (GDateMonth) 1, 2000);
		date2 = g_date_new_dmy (dt->tm_mday, (GDateMonth) (1 + dt->tm_mon),
		                                                    1900 + dt->tm_year);
	
		now = (dt->tm_hour + ((dt->tm_min + dt->tm_sec / 60.0) / 60.0)) / 24.0;
		julian_diff = g_date_get_julian (date2) - g_date_get_julian (date1) + 
		                                                              now - 0.5;
		year_diff = julian_diff / 365.25;
		*epoch = 2000.0 + year_diff;
		
		g_date_free (date1);
		g_date_free (date2);
		
		if (precess) {  /* Approximate...! */
			
			/* The following corrections are in arcsec for both */
			
			dRA = m + n * sin (*fRA) * tan (*fDec);
			dDec = n * cos (*fRA);
			
			/* New values in radians for both */
	
			*fRA -= year_diff * dRA * M_PI / (180.0 * 3600.0);
			*fDec -= year_diff * dDec * M_PI / (180.0 * 3600.0);
	
			sRA = ftos_RA (sRA, *fRA);
			sDec = ftos_Dec (sDec, *fDec);
			
			*epoch = 2000.0;
		}
	}
	return TRUE;
}

gushort telescope_get_RA_worm_pos (void)
{
	/* Get the current RA worm position */
	
	gfloat val, ignore;
	
	if (GeminiCmds)
		gemini_get (PEC_RA_COUNTER, &val, &ignore);
	else
		val = 0;
	
	return (gushort) val;
}

void telescope_get_guide_corrs (guint *e, guint *w, guint *n, guint *s,
                                gfloat *ratio)
{
	/* Return the number of guiding corrections in each direction */
	
	*e = pos.e;
	*w = pos.w;
	*n = pos.n;
	*s = pos.s;
	
	*ratio = (gfloat) pos.NumGuide / (gfloat) pos.NumImg;
}

gboolean telescope_get_gemini_model (struct GemModel *gm)
{
	/* Return the Gemini system pointing parameters */
	
	gfloat val;
	gfloat ignore;

	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	/* Get the values */
	
	gemini_get (A, &val, &ignore);
	gm->A = (gint) val;
	gemini_get (E, &val, &ignore);
	gm->E = (gint) val;
	gemini_get (NP, &val, &ignore);
	gm->NP = (gint) val;
	gemini_get (NE, &val, &ignore);
	gm->NE = (gint) val;
	gemini_get (IH, &val, &ignore);
	gm->IH = (gint) val;
	gemini_get (ID, &val, &ignore);
	gm->ID = (gint) val;
	gemini_get (FR, &val, &ignore);
	gm->FR = (gint) val;
	gemini_get (FD, &val, &ignore);
	gm->FD = (gint) val;
	gemini_get (CF, &val, &ignore);
	gm->CF = (gint) val;
	gemini_get (TF, &val, &ignore);
	gm->TF = (gint) val;
	
	return TRUE;
}

gboolean telescope_set_gemini_model (struct GemModel *gm)
{
	/* Set the Gemini system pointing parameters */
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	/* Set the values */
	
	if (!gemini_set (A, (gfloat) gm->A, 0, 0))
		return FALSE;
	if (!gemini_set (E, (gfloat) gm->E, 0, 0))
		return FALSE;
	if (!gemini_set (NP, (gfloat) gm->NP, 0, 0))
		return FALSE;
	if (!gemini_set (NE, (gfloat) gm->NE, 0, 0))
		return FALSE;
	if (!gemini_set (IH, (gfloat) gm->IH, 0, 0))
		return FALSE;
	if (!gemini_set (ID, (gfloat) gm->ID, 0, 0))
		return FALSE;
	if (!gemini_set (FR, (gfloat) gm->FR, 0, 0))
		return FALSE;
	if (!gemini_set (FD, (gfloat) gm->FD, 0, 0))
		return FALSE;
	if (!gemini_set (CF, (gfloat) gm->CF, 0, 0))
		return FALSE;
	if (!gemini_set (TF, (gfloat) gm->TF, 0, 0))
		return FALSE;
	
	return TRUE;
}

gboolean telescope_get_gemini_PEC (gchar *filename)
{
	/* Get the PEC data from the Gemini unit and write it to a file */
	
	gushort off, NumRAsteps, offset, guide, repeat;
	gfloat GuideSpeed, val, val1, ignore;
	gchar buf[PEC_RECLEN];
	FILE *fe;
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	if (!(fe = fopen (filename, "w")))
		return show_error (__func__, "Error opening PEC file");
	
	L_print ("{b}Saving PEC data...\n");
			
	/* Get the number of RA steps per worm revolution */
			
	gemini_get (PEC_RA_STEPS, &val, &ignore);
	NumRAsteps = (gushort) val;
	L_print ("Number of RA steps per worm revolution: %d\n", NumRAsteps);
	sprintf (buf, "%-15d\n", NumRAsteps);
	fwrite (buf, PEC_RECLEN, 1, fe);
	
	/* Get the current guide speed (although this may not be the speed that
	 * was used to train the data).
	 */
	
	gemini_get (CMD_SPEED_GUIDE, &val, &ignore);
	GuideSpeed = val;
	L_print ("Current guide speed: %f\n", GuideSpeed);
	sprintf (buf, "%-15.1f\n", GuideSpeed);
	fwrite (buf, PEC_RECLEN, 1, fe);
	
	/* Get the PEC data */

	off = 0;
	while (off < NumRAsteps) {
		val = off;
		gemini_get (PEC_CURRENT_DATA, &val, &val1);
		offset = off;
		guide = (gushort) val;
		repeat = (gushort) val1;
		L_print ("PEC data: guide %d at offset %d for %d steps\n", guide, 
				                                                offset, repeat);
		sprintf (buf, "%1d %6d %6d\n", guide, offset, repeat);
		fwrite (buf, PEC_RECLEN, 1, fe);
		off += (gushort) val1;
	}
	
	fclose (fe);
	
	return TRUE;
}

gboolean telescope_set_gemini_PEC (gchar *filename)
{
	/* Read the PEC data from a file and set it in the Gemini unit */

	gushort NumRAsteps, offset, guide, repeat;
	gfloat GuideSpeed;
	gchar buf[PEC_RECLEN], *off, *rep;
	FILE *fe;
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	if (!(fe = fopen (filename, "r")))
		return show_error (__func__, "Error opening PEC file");
	
	L_print ("{b}Uploading PEC data...\n");
	
	/* Read number of RA steps */
	
	fread (buf, PEC_RECLEN, 1, fe);
	NumRAsteps = (gushort) strtod (buf, (gchar **) NULL);
	
	/* Read guide speed */
	
	fread (buf, PEC_RECLEN, 1, fe);
	GuideSpeed = strtod (buf, (gchar **) NULL);
	gemini_set (CMD_SPEED_GUIDE, GuideSpeed, 0, 0);
	save_PEC_guide_speed (GuideSpeed);  /* Save value to config. database */
	
	/* Read PEC data */
	
	gemini_set (PEC_CURRENT_DATA, 0, 0, NumRAsteps);  /* Clear out old */
	while (fread (buf, PEC_RECLEN, 1, fe)) {
		guide = (gushort) strtod (buf, &off);
		offset = (gushort) strtod (++off, &rep);
		repeat = (gushort) strtod (++rep, (gchar **) NULL);
		gemini_set (PEC_CURRENT_DATA, guide, offset, repeat);
	}

	fclose (fe);
	
	return TRUE;
}

gboolean telescope_set_gemini_defaults (gchar *filename)
{
	/* Load Gemini defaults from file */
	
	gushort cmd;
	gfloat val;
	gchar *s, *buf, *str;
	FILE *ff;
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	if (!(ff = fopen (filename, "r")))
		return show_error (__func__, "Error opening gemini defaults file");
	
	L_print ("{b}Uploading gemini defaults...\n");
	
	s = (gchar *) g_malloc0 (SER_BUFSIZ * sizeof (gchar));
	buf = (gchar *) g_malloc0 (DEF_RECLEN * sizeof (gchar));
	while (fread (buf, DEF_RECLEN, 1, ff)) {
		if (!strncmp (buf, "#", 1)) {        /* Begins with "#" - ignore     */
			;
		} else if (!strncmp (buf, "/", 1)) { /* Begins with "/" - echo line  */
			buf[0] = '>';
			buf[DEF_RECLEN - 1] = '\0';
			L_print ("%s\n", buf);
		} else if (!strncmp (buf, ":", 1)) { /* Begins with ";" -  LX200 cmd */
			buf = g_strchomp (buf);
			s_write (tel_comms->f, buf, strlen (buf));
			s_read (tel_comms->f, s, SER_BUFSIZ);
			L_print ("Written to gemini: %s, returned: %s\n", buf, s);
		} else {                             /* Gemini native command set    */
			cmd = (gushort) strtod (buf, &str);
			val = (gfloat) strtod (++str, (gchar **) NULL);
			gemini_set (cmd, val, 0, 0);
		}
		memset (buf, 0, sizeof (buf));
	}

	g_free (buf);
	g_free (s);
	fclose (ff);
	
	return TRUE;
}

gboolean telescope_PEC_on (gboolean On)
{
	/* Turn periodic error correction on/off in the Gemini box */
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return FALSE;
	}
	
	if (On)
		gemini_set (PEC_STATUS, PEC_AVAILABLE | PEC_ACTIVE, 0, 0);
	else
		gemini_set (PEC_STATUS, 0, 0, 0);
	
	return TRUE;
}

void telescope_set_time (void)
{
	/* Synchronise Gemini date and time to PC.  The Gemini real-time clock
	 * runs GMT so we tell it the offset from UTC in hours after (+) or
	 * before (-) and then the local date and time.
	 */
	
	struct tm *dt;
    time_t t;
	gint offset;
	gchar *s, *s1, *s2;
	
	if (!(tel_comms->user & PU_TEL)) {
		msg ("Warning - The telescope link isn't open!");
		return;
	}
	
	s = (gchar *) g_malloc0 (SER_BUFSIZ * sizeof (gchar));
	
	/* Get PC date/time info */
	
	time (&t);
	dt = localtime (&t);         /* Local time */
	offset = get_UTC_offset ();  /* Offset between UTC and local time */
	
	/* Get the current Gemini date */
	
	sprintf (s, "00/00/00");
	s_write (tel_comms->f, GET_DATE, 4);
	s_read (tel_comms->f, s, 9);
	s[8] = '\0'; /* Over-write ninth character which should be hash */
	s1 = g_strdup_printf ("Current gemini local time is: %s ", s);
	
	/* Get the current Gemini time */
	
	sprintf (s, "00:00:00");
	s_write (tel_comms->f, GET_LTM, 4);
	s_read (tel_comms->f, s, 9);
	s[8] = '\0'; /* Over-write ninth character which should be hash */
	s2 = g_strconcat (s1, s, NULL);
	
	L_print ("%s\n", s2);
	g_free (s2);
	g_free (s1);
	
	/* Get the current Gemini UTC offset */
	
	sprintf (s, "####");
	s_write (tel_comms->f, GET_UTCOFF, 4);
	s_read (tel_comms->f, s, 4);
	s[3] = '\0'; /* Over-write fourth character which should be hash */
	L_print ("Current gemini UTC offset is: %s\n", s);

	/* Set the timezone */
	
	sprintf (s, "%s%+.2d#", SET_UTCOFF, offset);
	s_write (tel_comms->f, s, 7);
	L_print ("Gemini set timezone offset to %+.2d...  ", offset);
	s_read (tel_comms->f, s, 1);
	L_print ("{.}%s\n", (strncmp (s, "0", 1) ? "OK" : "failed"));
	
	/* Set the date */
	
	sprintf (s, "%s%02i/%02i/%02i#", SET_DATE, dt->tm_mon + 1, dt->tm_mday, 
	                                                         dt->tm_year - 100);
	s_write (tel_comms->f, s, 12);
	s[11] = '\0';
	L_print ("Gemini set local date to: %s...  ", s + 3);
	s_read (tel_comms->f, s, SER_BUFSIZ);
	L_print ("{.}%s\n", (strncmp (s, "0", 1) ? "OK" : "failed"));
	
	/* Set the time */
	
	sprintf (s, "%s%02i:%02i:%02i#", SET_LTM, dt->tm_hour, dt->tm_min, 
	                                                                dt->tm_sec);
	s_write (tel_comms->f, s, 12);
	s[11] = '\0';
	L_print ("Gemini set local time to: %s...  ", s + 3);	
	s_read (tel_comms->f, s, 1);
	L_print ("{.}%s\n", (strncmp (s, "0", 1) ? "OK" : "failed"));
	
	/* Get the new Gemini date */
	
	sprintf (s, "00/00/00");
	s_write (tel_comms->f, GET_DATE, 4);
	s_read (tel_comms->f, s, 9);
	s[8] = '\0'; /* Over-write ninth character which should be hash */
	s1 = g_strdup_printf ("New gemini local time is: %s ", s);
	
	/* Get the new Gemini time */
	
	sprintf (s, "00:00:00");
	s_write (tel_comms->f, GET_LTM, 4);
	s_read (tel_comms->f, s, 9);
	s[8] = '\0'; /* Over-write ninth character which should be hash */
	s2 = g_strconcat (s1, s, NULL);
	
	L_print ("%s\n", s2);
	g_free (s2);
	g_free (s1);
	
	/* Get the new Gemini UTC offset */
	
	sprintf (s, "    ");
	s_write (tel_comms->f, GET_UTCOFF, 4);
	s_read (tel_comms->f, s, 4);
	s[3] = '\0'; /* Over-write fourth character which should be hash */
	L_print ("New gemini UTC offset is: %s\n", s);
	
	g_free (s);
}

guint telescope_query_status (gboolean list)
{
	/* Query Gemini status and report RA worm position */

	gushort WormPos;
	guint n, flags;	
	gfloat val, ignore;
	gchar buf[4];
	
	/* Send ACK to Gemini box */
	
	s_write (tel_comms->f, &ACK, 1);	
	s_read (tel_comms->f, buf, 2);
	
	if ((n = strncmp (buf, START_WAIT, 2)) == 0) {
		if (list) {
			L_print ("Gemini status:\n");
			L_print ("> Telescope is waiting to be started\n");
		}
		return TEL_START_WAIT;
	} else if ((n = strncmp (buf, STARTED, 2)) == 0) {
		gemini_get (INQUIRY, &val, &ignore);
		flags = (guint) val;
		if (list) {
			L_print ("Gemini status:\n");
			if (flags & TEL_ALIGNED)
				L_print ("> Telescope is aligned\n");
			if (flags & MOD_USE)
				L_print ("> Modelling is in use\n");
			if (flags & OBJ_SEL)
				L_print ("> Object is selected\n");
			if (flags & GOTO_ACTIVE)
				L_print ("> GoTo operation is in progress\n");
			if (flags & RA_LIM)
				L_print ("> RA limit has been reached\n");
			if (flags & PRECESS)
				L_print ("> Precessing coordinates from J2000.0 to equinox "
		                                                           "of date\n");
			WormPos = telescope_get_RA_worm_pos (); /* Get RA worm position */
			L_print ("> RA worm position is: %d\n", WormPos);
		}
		return flags;
	} else {
		if (list) {
			L_print ("Gemini status:\n");
            L_print ("> Status unknown\n");
		}			
		return TEL_UNKNOWN;
	}
}

void telescope_save_motion_rates (void)
{
	/* Save the original guide and center rates */
	
	gfloat val, ignore;
	
	gemini_get (CMD_SPEED_GUIDE, &orig_rate_guide, &ignore);
	gemini_get (CMD_SPEED_CENTER, &val, &ignore);
	orig_rate_center = (gint) val;
	L_print ("Saved telescope guide rate of %f and telescope centre rate "
	                              "of %d\n", orig_rate_guide, orig_rate_center);
}

void telescope_restore_motion_rates (void)
{
	/* Restore the original guide and center rates */
	
	gemini_set (CMD_SPEED_GUIDE, orig_rate_guide, 0, 0);
	gemini_set (CMD_SPEED_CENTER, (gfloat) orig_rate_center, 0, 0);
	L_print ("Restored telescope guide rate of %f and telescope centre rate "
	                              "of %d\n", orig_rate_guide, orig_rate_center);
}

static gboolean gemini_get (gushort id, gfloat *val, gfloat *val1)
{
	/* Get Gemini native value from Losmandy Gemini System */
	
	gint i;
	gchar c, chk;
	gchar *command, *string, *s;
	gchar input[9];
	
	/* Compose the string to be sent to the Gemini unit */
	
	if (id != PEC_CURRENT_DATA)
		command = g_strdup_printf ("<%d:", id);
    else
		command = g_strdup_printf ("<%d:-%d", id, (gushort) *val);
	string = g_strdup_printf ("%s%.1s#", command, chksum (command, &c));
	
	/* Write to Gemini */

	s_write (tel_comms->f, string, strlen (string));
	G_print ("gemini_get: written command string: %s\n", string);
		
	g_free (string);
	g_free (command);
		
	/* Read response - max. length is 9 characters e.g. '0:12345c#' for a PEC 
	 * query, where 'c' is the checksum.  (Although for a G11, the native PECmax
	 * value is only 4 digits, i.e. 6400).
	 */
	
	memset (input, 0, sizeof (input));
	s_read (tel_comms->f, input, 9);
	
	i = -1;
	while (!(input[++i] == '#')) {
		if (i == 8) { /* got to end of input[] without finding '#' */
			L_print ("{r}gemini_get: Warning - incomplete response returned "
			                               "from gemini system; '#' missing\n");
			*val = 0.0;
			*val1 = 0.0;
			return FALSE;
		}
	}
	if (i == 0) { /* gemini returns just '#' if unrecognised cmd sent to it */
		L_print ("{r}gemini_get: Warning - incorrect command id; gemini only "
		                                                      "returned '#'\n");
		*val = 0.0;
		*val1 = 0.0;
		return FALSE;
	}
	
	input[i] = '\0';    /* Overwrite terminating hash */
	chk = input[i-1];   /* Get the checksum */
	input[i-1] = '\0';  /* Overwrite checksum */
	if (strncmp (&chk, chksum (input, &c), 1) != 0) {
		L_print ("{r}gemini_get: Warning - incorrect checksum from "
		                                                     "gemini system\n");
		*val = 0.0;
		*val1 = 0.0;
		return FALSE;
	}
	
	G_print ("gemini_get: gemini returned: %s\n", input);
	
	/* Return the required value */
	
	if (id != PEC_CURRENT_DATA) {
		*val = (gfloat) atof (input);
	    *val1 = 0.0;
	} else {
		*val = strtod (input, &s);
		*val1 = strtod (++s, (gchar **) NULL);
	}
	
	return TRUE;	
}

static gboolean gemini_set (gushort id, gfloat val, gfloat val1, gfloat val2)
{
	/* Set Gemini native value in Losmandy Gemini System */

	gchar c, s[4];
	gchar *command, *string;
	
	/* Compose the string to be sent to the Gemini unit */
	
	switch (id) {
		case CMD_SPEED_GUIDE:  /* Guiding speed written as single float  */
			command = g_strdup_printf (">%d:%.1f", id, val);
			break;
		case PEC_CURRENT_DATA: /* PEC data written as three integer values */
			command = g_strdup_printf (">%d:%d;%d;%d", id, (gushort) val, 
			                                    (gushort) val1, (gushort) val2);
		    break;
		default:               /* Other values written as single integers  */
			command = g_strdup_printf (">%d:%d", id, (gint) val);
			break;
	}
	
	string = g_strdup_printf ("%s%.1s#", command, chksum (command, &c));
	
	/* Write to Gemini */
	
	s_write (tel_comms->f, string, strlen (string));
	s_read (tel_comms->f, s, 1);
	s[1] = '\0';
	G_print ("gemini_set: written command string: %s; returned: %s\n", 
			                                                         string, s);
	g_free (string);
	g_free (command);
	
	return TRUE;
}

static gchar *chksum (gchar *string, gchar *c)
{
	/* Return Gemini checksum of input string.  (This works provided that the
	 * string doesn't contain NULLs).
	 */
	
	gushort i;
	
	i = 0, *c = string[0];
	while (string[i++])
		*c ^= string[i];
	*c = (*c & 0x7F) + 0x40;
	
	return c;
}

gdouble stof_RA (gchar *RA)
{
	/* Convert string format to radians */
	
	gfloat h, m, s;
	gchar *endm, *ends;
	
	h = strtod (RA, &endm);
	m = strtod (++endm, &ends);
	s = strtod (++ends, (gchar **) NULL);
	
	return ((h + (m + s / 60.0) / 60.0) * M_PI / 12.0);
}

static gdouble stof_Dec (gchar *Dec)
{
	/* Convert string format to radians */
	
	gfloat d, m, s;
	gchar *endm, *ends;
	
	d = strtod (Dec, &endm);
	m = strtod (++endm, &ends);
	s = strtod (++ends, (gchar **) NULL);
	
	return (d > 0 ? (d + (m + s / 60.0) / 60.0) * M_PI / 180.0 :
	                (d - (m + s / 60.0) / 60.0) * M_PI / 180.0);
}

static gchar *ftos_RA (gchar *string, gdouble RA)
{
	/* Convert radians to string format */
	
	gdouble f, h, m, s;
	gint ih, im, is;
	
	f = RA  * 12.0 / M_PI;
	f = fabs (modf (f, &h));
	f = modf (f * 60.0, &m);
	f = modf (f * 60.0, &s);
	
	ih = (gint) h;
	im = (gint) m;
	is = (gint) s;
	
	sprintf (string, "%02d:%02d:%02d", ih, im, is);
	return string;
}

static gchar *ftos_Dec (gchar *string, gdouble Dec)
{
	/* Convert radians to string format */
	
	gdouble f, d, m, s;
	gint id, im, is;
	
	f = fabs (Dec * 180.0 / M_PI);
	f = fabs (modf (f, &d));
	f = modf (f * 60.0, &m);
	f = modf (f * 60.0, &s);
	
	id = (gint) d;
	im = (gint) m;
	is = (gint) s;
	
	sprintf (string, "%1s%02d:%02d:%02d", Dec >= 0 ? "+" : "-", id, im, is);
	return string;
}

static void mean_pos (gboolean *reset, gfloat pos[], guint size, 
				      enum MotionDirection dirn, gfloat val, gfloat *mean)
{
	/* Calculate the running mean of the array 'pos[]' of size 'size' */
	
	guint i, j;
	static guint h_i = 0, v_i = 0;

	i = dirn & H ? h_i : v_i;
	
	if (*reset) {
		for (i = 0; i < size; i++)
			pos[i] = 0;
		i = 0;
		*mean = 0;
		*reset = FALSE;
		G_print ("Resetting %s drift calculation\n", dirn & H ? "E/W" : "N/S");		
	}
	
	pos[i++] = val;
	if (i == size)
		i = 0;                  /* Wrap at end of array */
	
	for (j = 0, *mean = 0; j < size; j++)
		*mean += pos[j] / size;
	
	if (dirn & H)
		h_i = i;
	else
		v_i = i;
}
