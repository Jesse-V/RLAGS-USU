/******************************************************************************/
/*                    HEADER FILE FOR I/O PORT ROUTINES                       */
/*                                                                            */
/* Header file for I/O port routines.                                         */
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

#ifndef GOQAT_PORTS_H
#define GOQAT_PORTS_H

#include <termios.h>
#include "telescope.h"

#define MAX_PORTS 264            /* Maximum number of ports                   */
#define BASE_PORTS 8             /* See enum BasePorts below                  */
#define SER_BUFSIZ 50            /* Maximum size of serial port data buffer   */

/* This enum is for ports selected via the Communications menu, but excluding */
/* the /dev/ttyUSB 'serial' ports that can come and go or get renumbered.     */ 
/* The /dev/ttyUSB ports are enumerated dynamically.                          */
 
enum BasePorts {
	DUMMY = 0,                   /* Dummy port if no others available         */
	LPT,                         /* Parallel port managed by Parapin library. */
	USBAUG,                      /* USB ports for sending guide commands via  */
	USBCCD,                      /*  autoguider or CCD cameras.               */
	TTY0,                        /* Traditional serial ports /dev/ttyS0       */
	TTY1,                        /*                          /dev/ttyS1       */
	TTY2,                        /*                          /dev/ttyS2       */
	TTY3,                        /*                          /dev/ttyS3       */ 
};

enum PortState {                 /* Possible serial comms port states         */
	PS_TEST, 
	PS_READY
};

enum PortUsers {                 /* Serial port users                         */
	PU_TEL = 1, 
	PU_AUTOG = 2,
	PU_FOCUS = 4
};

struct port {                    /* Port data                                 */
	void (*guide_start) (enum TelMotion direction); /* Funcs. to be used with */
	void (*guide_stop) (enum TelMotion direction);  /*           this port for*/
	void (*guide_pulse) (enum TelMotion direction, gint duration);/*autoguider*/
	void (*focus) (struct focus *f); /* Focuser functions used with this port */
	struct termios old_tio;      /* Original port settings    (serial port)   */ 
	struct termios new_tio;      /* New port settings         (serial port)   */
	gushort pnum;                /* Port number (0 ... number of ports)       */
	gushort user;                /* User IDs of this port                     */
	gushort ref_count;           /* Number of users of this port              */
	gint f;                      /* Device file for this port (serial port)   */
	gint RAp;                    /* RA+ physical pin number   (parallel port) */
	gint RAm;                    /* RA- physical pin number   (parallel port) */
	gint Decp;                   /* Dec+ physical pin number  (parallel port) */
	gint Decm;                   /* Dec- physical pin number  (parallel port) */
	//gint SC1LongExp;           /* SC1 long exposure pin number         (pp) */
	//gint SC1Pause;             /* SC1 pause interval between exposures (pp) */
	gchar name[20];              /* Name of port                              */
	gchar address[6];            /* Base address              (parallel port) */
};	
  
struct port ports[MAX_PORTS];
struct port *tel_comms, *autog_comms;  /* Telescope and autog. ports          */
struct port *focus_comms;              /* Focuser port                        */
	
#endif /* GOQAT_PORTS_H */
