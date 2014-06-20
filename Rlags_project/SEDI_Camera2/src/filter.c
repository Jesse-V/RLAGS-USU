/******************************************************************************/
/*                        FILTER WHEEL ROUTINES                               */
/*                                                                            */
/* All the routines for interacting with external filter wheels (i.e. ones    */
/* not integral to the camera) are in here.                                   */
/*                                                                            */
/* Copyright (C) 2011 - 2014  Edward Simonson                                 */
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

#define GOQAT_FILTER
#include "interface.h"
#ifdef HAVE_SX_FILTERWHEEL
#include "sx.h"
#endif

struct filterwheel {
	enum HWDevice device;            /* Type of filter wheel e.g. QSI, SX etc */
	gshort devnum;    /* Number of selected device in device_selection struct */
	gushort init_pos;                /* Initial position (1 or 0)             */
	gushort cur_pos;                 /* Current position                      */
	gboolean Open;                   /* TRUE if filter wheel is open          */
};

static struct filterwheel fw;

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void filter_init (void);
void filter_set_device (enum HWDevice dev);
gboolean filter_get_filterwheels (void);
gshort *get_filter_devnumptr (void);
gboolean filter_open_comms_port (void);
gboolean filter_close_comms_port (void);
gboolean filter_is_open (void);
gboolean filter_set_filter_pos (gushort pos);
gushort filter_get_filter_pos (void);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void filter_init (void)
{
	/* Initialise filter data structure to sensible values */
	
	fw.devnum = -1;
	fw.init_pos = 0;
	fw.cur_pos = 0;
	fw.Open = FALSE;
}

void filter_set_device (enum HWDevice dev)
{
	/* Set the filter wheel device type */
	
	fw.device = dev;
}

gboolean filter_get_filterwheels (void)
{
	/* Fill the device selection structure with a list of detected filter
	 * wheels.
	 */
	 
	switch (fw.device) {
		case INTERNAL:
			break;
		#ifdef HAVE_SX_FILTERWHEEL
		case SX: /* Assume direct USB connection */
			/* Use same error handler as CCD code */
			sx_error_func (ccdcam_sx_error_func);
			ds.num = MAX_DEVICES;
			if (!sxf_get_filterwheels (ds.id, ds.desc, &ds.num))
				return show_error (__func__, 
								   "Error searching for filter wheels!");
			if (!ds.num) {
				L_print ("{o}Didn't find any filter wheels!\n");
				return FALSE;
			}
			break;
		#endif
		case OTHER: /* INDI etc */
			break;
		default:
			break;
	}
	    
	return TRUE;	
}

gshort *get_filter_devnumptr (void)
{
	/* Return a pointer to the filter wheel device number */
	
	return &fw.devnum;
}

gboolean filter_open_comms_port (void)
{
	/* Open the link to the requested filter wheel */
	
	gint num;
	
	if (fw.Open)  /* return if already open */
		return TRUE; 
	
	L_print ("{b}****---->>>> Opening filter wheel\n");
	
	switch (fw.device) {
		case INTERNAL:
			break;
		#ifdef HAVE_SX_FILTERWHEEL
		case SX:
			if (filter_comms->pnum == USBDIR) { /* Direct USB connection */
				/* Use same error handler as CCD code */
				sx_error_func (ccdcam_sx_error_func);
				num = MAX_DEVICES;
				if (!sxf_get_filterwheels (ds.id, ds.desc, &num))
					return show_error (__func__, 
									   "Error searching for filter wheels!");
				if (num == 0) {
					L_print ("{o}Didn't find any filter wheels!\n");
					return FALSE;
				} else if (num == 1) {
					if (!sxf_connect (TRUE, ds.id[0]))
						return show_error (__func__, 
										  "Unable to connect to filter wheel!");
					fw.Open = TRUE;
				} else if (num > 1 && fw.devnum == -1) {
					L_print ("{o}Found more than one filter wheel!  Please "
							 "select the one you want via the "
							 "'Filters' menu.\n");
					return FALSE;
				} else if (num > 1 && fw.devnum < num) {
					if (!sxf_connect (TRUE, ds.id[fw.devnum]))
						return show_error (__func__, 
						                  "Unable to connect to filter wheel!");
					fw.Open = TRUE;
				} else
					return show_error (__func__, "Error opening filter wheel!");
			} else { /* e.g. serial connection */
				return show_error (__func__, "Filter wheel connection option "
				                   "not supported");
			}
			break;
		#endif
		case OTHER: /* INDI etc */
			break;
		default:
			return show_error (__func__, "Unknown filter wheel");
	}

	return TRUE;
}

gboolean filter_close_comms_port (void)
{
	/* Close the requested filter wheel type */
	
	if (!fw.Open)  /* return if already closed */
		return TRUE;
	 
	switch (fw.device) {
		case INTERNAL:
			break;
		#ifdef HAVE_SX_FILTERWHEEL
		case SX: /* Assume direct USB connection */
			if (!sxf_connect (FALSE, NULL))
				return show_error (__func__, "Unable to close filter wheel!");
			fw.Open = FALSE;
			break;
		#endif
		case OTHER: /* INDI etc */
			break;
		default:
			break;
	}
	
	filter_init ();
	L_print ("{b}****---->>>> Filter wheel closed\n");
	return TRUE;
}

gboolean filter_is_open (void)
{
	return fw.Open;
}

gboolean filter_set_filter_pos (gushort pos)
{
	/* Set the current filter position */
	
	switch (fw.device) {
		case INTERNAL:
			break;
		#ifdef HAVE_SX_FILTERWHEEL
		case SX: /* Assume direct USB connection */
			return sxf_set_filter_pos (pos);
			break;
		#endif
		case OTHER: /* INDI etc */
			break;
		default:
			break;
	}
	
	return TRUE;
}

gushort filter_get_filter_pos (void)
{
	/* Return the current filter position */
	
	switch (fw.device) {
		case INTERNAL:
			break;
		#ifdef HAVE_SX_FILTERWHEEL
		case SX: /* Assume direct USB connection */
			return sxf_get_filter_pos ();
			break;
		#endif
		case OTHER: /* INDI etc */
			break;
		default:
			break;
	}
	
	return TRUE;
}

