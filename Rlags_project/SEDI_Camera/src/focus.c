/******************************************************************************/
/*                            FOCUSER ROUTINES                                */
/*                                                                            */
/* All the routines for interacting with the focuser are contained in here.   */
/*                                                                            */
/* Copyright (C) 2011 - 2013  Edward Simonson                                 */
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

#define GOQAT_FOCUS
#include "interface.h"

#define RF_CMD_LENGTH 9   /* Robofocus command length */

static gint ref_pos = 0;
static gfloat current_temp = 0.0, ref_temp = 0.0;

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

gboolean focus_open_focus_port (void);
void focus_close_focus_port (void);
gboolean focus_is_moving (void);
void focus_store_temp_and_pos (void);
void focus_get_temp_diff_pos (gdouble *temp_diff, gint *pos);
static void focus_robofocus (struct focus *f);
static void robo_write (gchar *string);
static gboolean robo_read (gchar *string);
static gchar *rf_chksum (gchar *string, gchar *c);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

gboolean focus_open_focus_port (void)
{
	/* Open the requested serial port for the focuser */
	
	struct focus f;
	
	L_print ("{b}****---->>>> Opening focuser comms link on "
	                                              "%s...\n", focus_comms->name);
	
	/* Return if already open */
	
	if (focus_comms->ref_count && focus_comms->user & PU_FOCUS) {
		L_print ("{b}Port already open for focuser communications!\n");
		L_print ("{b}****---->>>> Opened focuser link\n");
		return TRUE;
	}
	
	/* Open if not already open */
	
	if (!focus_comms->ref_count) {
		if (!serial_open_port (focus_comms, PU_FOCUS))
			return show_error (__func__, "Unable to open focuser "
							             "communications link");
	} else {
		focus_comms->ref_count++;
		focus_comms->user |= PU_FOCUS;
		L_print ("{b}Selected port %s is already open - sending focuser "
			                      "commands on same port\n", focus_comms->name);
	}
	
	/* Assign focuser functions for this port */
	
	if (!strcmp (menu.focuser, "focuser_robofocus")) {  /* Robofocus */
	    ports[focus_comms->pnum].focus = focus_robofocus;
		f.cmd = FC_VERSION;
		focus_comms->focus (&f);
		if (f.Error) {
			focus_close_focus_port ();
			return show_error (__func__, "Can't find Robofocus!");
		} else
			L_print ("Found Robofocus version %.3f\n", f.version);
	} else {  /* Some other focuser... */
		msg ("Warning - Invalid option for focuser!");
	}
	
	L_print ("{b}****---->>>> Opened focuser link\n");

	return TRUE;
}

void focus_close_focus_port (void)
{
	/* Close down the communications link with the focuser if it's open and
	 * nothing else (e.g. telescope commands) is using this link.
	 */
	 
	if (!focus_comms->ref_count)  /* Return if not open */
		return;
	
	if (focus_comms->user & PU_FOCUS) {
		focus_comms->ref_count--;   /* Decrease ref. count */
		focus_comms->user &= ~PU_FOCUS;
		L_print ("{b}****---->>>> Communications with focuser closed\n");
	}
	if (focus_comms->ref_count)    /* If still ref'd by someone else, return */
		return;
	
	serial_close_port (focus_comms);
	
	/* Reset focuser function pointers */
	
	ports[focus_comms->pnum].focus = NULL;
}	

gboolean focus_is_moving (void)
{
	/* Check to see if focuser is moving.  Return TRUE if it is, FALSE if it
	 * has stopped.
	 */
	
	gchar buf[256];
	
	memset (buf, 0, 256);
	s_read (focus_comms->f, buf, 255);
	if (!strcmp (buf, "#"))
		return FALSE;
	
	return TRUE;
}

void focus_store_temp_and_pos (void)
{
	/* Store the current temperature and position as the reference point for 
	 * temperature compensated focusing.
	 */
	
	struct focus f;
	
	ref_temp = current_temp;
	
	f.cmd = FC_CUR_POS_GET;
	focus_comms->focus (&f);
	ref_pos = f.cur_pos;	
}

void focus_get_temp_diff_pos (gdouble *temp_diff, gint *pos)
{
	/* Return the difference between the stored and the current temperature,
	 * and the focuser position corresponding to the stored temperature.
	 */
	
	*temp_diff = current_temp - ref_temp;
	*pos = ref_pos;
}

static void focus_robofocus (struct focus *f)
{
	/* All the Robofocus functions are handled here */
	
	gchar *w, r[RF_CMD_LENGTH];
	
	f->Error = FALSE;
	if (f->cmd & FC_VERSION && !f->Error) {  /* Request firmware version */
		w = "FV000000";
		robo_write (w);
		if (robo_read (r) && !strncmp (r, "FV", 2))
			f->version = strtof (r+2, (gchar **) NULL);
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_MAX_TRAVEL_GET && !f->Error) {  /* Get max. travel */
		w = "FL000000";
		robo_write (w);
		if (robo_read (r))
			f->max_travel = (gint) strtol (r+2, (gchar **) NULL, 10);
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_MAX_TRAVEL_SET && !f->Error) {  /* Set max. travel */
		w = g_strdup_printf ("FL%06d", f->max_travel);
		robo_write (w);
		g_free (w);
		if (robo_read (r))
			f->max_travel = (guint) strtol (r+2, (gchar **) NULL, 10);
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_MOVE_BY && !f->Error) {      /* Move in/out */
		if (f->move_by <= 0)
			w = g_strdup_printf ("FI%06d", -f->move_by);
		else
			w = g_strdup_printf ("FO%06d", f->move_by);
		robo_write (w);
		g_free (w);
	}
	if (f->cmd & FC_MOVE_TO && !f->Error) {      /* Move to given position */
		w = g_strdup_printf ("FG%06d", f->move_to);
		robo_write (w);
		g_free (w);
	}
	if (f->cmd & FC_STOP && !f->Error) {      /* Stop focuser motion */
		s_write (focus_comms->f, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		                         "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		                         "XXXXXXXXXXXXXXXXXXXX", 100);
	}
	if (f->cmd & FC_CUR_POS_GET && !f->Error) {  /* Get current position */
		w = "FS000000";
		robo_write (w);
		if (robo_read (r))
			f->cur_pos = (gint) strtol (r+2, (gchar **) NULL, 10);
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_CUR_POS_SET && !f->Error) {  /* Set current position */
		w = g_strdup_printf ("FS%06d", f->cur_pos);
		robo_write (w);
		g_free (w);
		if (robo_read (r))
			f->cur_pos = (guint) strtol (r+2, (gchar **) NULL, 10);
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_BACKLASH_GET && !f->Error) {  /* Get backlash settings */
		w = "FB000000";
		robo_write (w);
		if (robo_read (r)) {
			f->backlash_steps = (gint) strtol (r+3, (gchar **) NULL, 10);
			f->BacklashIn = strncmp (r+2, "2", 1) ? FALSE : TRUE;
		}
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_BACKLASH_SET && !f->Error) {  /* Set backlash settings */
		w = g_strdup_printf ("FB%1d%05d", f->BacklashIn ? 2 :3, 
														 f->backlash_steps);
		robo_write (w);
		g_free (w);
		if (robo_read (r)) {
			f->backlash_steps = (gint) strtol (r+3, (gchar **) NULL, 10);
			f->BacklashIn = strncmp (r+2, "2", 1) ? FALSE : TRUE;
		}
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_MOTOR_GET && !f->Error) {  /* Get motor config. data */
		w = "FC000000";
		robo_write (w);
		if (robo_read (r)) {
			f->duty_cycle = (guchar) r[2];
			f->step_pause = (guchar) r[3];
			f->step_size = (guchar) r[4];
		}
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_MOTOR_SET && !f->Error) {  /* Set motor config. data */
		w = g_malloc0 (RF_CMD_LENGTH);
		strcpy (w, "FC000000");
		w[2] = f->duty_cycle;
		w[3] = f->step_pause;
		w[4] = f->step_size;
		robo_write (w);
		g_free (w);
		if (robo_read (r)) {
			f->duty_cycle = (guchar) r[2];
			f->step_pause = (guchar) r[3];
			f->step_size = (guchar) r[4];
		}
		else
			f->Error = TRUE;
	}
	if (f->cmd & FC_TEMP_GET && !f->Error) {  /* Get current temperature */
		w = "FT000000"; 
		robo_write (w);
		if (robo_read (r)) {
			f->temp = ((gfloat) strtol (r+2, (gchar **) NULL, 10)) / 2 - 273;
		    current_temp = f->temp;           /* Store current temperature */
		} else
			f->Error = TRUE;
	}
}

static void robo_write (gchar *string)
{
	/* Write to Robofocus */
	
	gchar c;
	
	s_write (focus_comms->f, string, RF_CMD_LENGTH - 1);
	s_write (focus_comms->f, rf_chksum (string, &c), 1);
}

static gboolean robo_read (gchar *string)
{
	/* Read from Robofocus */
	
	gchar chk, c;
	
	memset (string, 0, RF_CMD_LENGTH);
	s_read (focus_comms->f, string, RF_CMD_LENGTH);
	chk = string[RF_CMD_LENGTH - 1];   /* Get the checksum   */
	string[RF_CMD_LENGTH - 1] = '\0';  /* Overwrite checksum */
	if (strncmp (&chk, rf_chksum (string, &c), 1) != 0)
		return show_error (__func__, "Incorrect checksum from Robofocus");
	
	return TRUE;
}

static gchar *rf_chksum (gchar *string, gchar *c)
{
	/* Return Robofocus checksum of input string */
	
	gushort i;
	
	for (i = 0, *c = 0; i < RF_CMD_LENGTH - 1; i++)
		*c += string[i];
	
	return c;
}
