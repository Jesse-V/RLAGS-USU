/******************************************************************************/
/*                           SERIAL PORT ROUTINES                             */
/*                                                                            */
/* All the routines for initialising, writing to and reading from serial      */
/* ports are contained in this module.                                        */
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define GOQAT_SERIAL
#include "interface.h"

#define BAUDRATE B9600                   /* Serial baud rate                  */

enum ReadSerial {                        /* Error states reading serial device*/
	RS_ERR_POLL = -1,
	RS_TIMEOUT = -2,
	RS_ERR_READ = -3
};

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void serial_define_comms_ports (void);
gboolean serial_set_comms_port (const gchar *name);
gboolean serial_open_port (struct port *p, enum PortUsers user);
void serial_close_port (struct port *p);
static void port_init (struct port *p, enum PortState state);
gint s_read (gint file, gchar string[], gint bytes);
gint s_write (gint file, gchar string[], gint bytes);
	

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void serial_define_comms_ports (void)
{
	/* Set up comms ports definitions */
	
	gushort i;
		
	for (i = 0; i < MAX_PORTS; i++) {
		ports[i].pnum = i;
		ports[i].user = 0;
		ports[i].ref_count = 0;
		ports[i].f = 0;
		ports[i].guide_start = telescope_d_start;
		ports[i].guide_stop = telescope_d_stop;
		ports[i].guide_pulse = telescope_d_pulse;
		ports[i].focus = NULL;
	}

	/* Assign names to ports.  USB ports are named when they are detected. */
	
	strcpy (ports[DUMMY].name, "unavailable port");
	strcpy (ports[TTY0].name, "/dev/ttyS0");
	strcpy (ports[TTY1].name, "/dev/ttyS1");
	strcpy (ports[TTY2].name, "/dev/ttyS2");
	strcpy (ports[TTY3].name, "/dev/ttyS3");
	
	/* Set initial values for ports pointers */
	
	autog_comms  = &ports[DUMMY];
	tel_comms    = &ports[DUMMY];
	filter_comms = &ports[DUMMY];
	focus_comms  = &ports[DUMMY];
}

gboolean serial_set_comms_port (const gchar *name)
{
	/* Set pointers to the requested comms port.  The passed values of 'name' 
	 * must match the names in the Glade gtkbuilder file, or as defined in code. 
	 * For the telescope port this is preceded by "t_", for the autoguider port 
	 * by "a_", for the filter wheel port by "w_" and for the focuser port 
	 * by "f_".
	 * NOTE: The first three ports aren't actually serial ports, they are the
	 * parallel port, the USB port used for direct (probably non-serial) 
	 * guide commands to the autoguider camera and the USB port used for direct 
	 * (probably non-serial) guide commands to the CCD camera.  It is convenient
	 * to set them here.
	 */
	 
	struct cam_img *aug = get_aug_image_struct ();
	struct cam_img *ccd = get_ccd_image_struct ();
	
	gushort pnum = 0;
	
	if (!(strcmp (name+2, "ParallelPort")))
		pnum = LPT;
	else if (!(strcmp (name+2, "GuideCam"))) {
		pnum = USBAUG;
		if (aug->device == SX && aug->Open)
			L_print ("{o}Please re-open the autoguider camera for this option "
					 "to take effect\n");
	} else if (!(strcmp (name+2, "CCDCam"))) {
		pnum = USBCCD;
		if (ccd->device == SX && ccd->Open)
			L_print ("{o}Please re-open the CCD camera for this option "
					 "to take effect\n");
	} else if (!(strcmp (name+2, "USBdirect"))) {
		pnum = USBDIR;
	} else if (!(strcmp (name+2, "ttyS0")))
		pnum = TTY0;
	else if (!(strcmp (name+2, "ttyS1")))
		pnum = TTY1;
	else if (!(strcmp (name+2, "ttyS2")))
		pnum = TTY2;
	else if (!(strcmp (name+2, "ttyS3")))
		pnum = TTY3;
	else if (!(strncmp (name+2, "ttyUSB", 6)))
		pnum = BASE_PORTS + (gushort) strtol (name+8, NULL, 10);
	else
	    return show_error (__func__, "Invalid port name!");
	    
	if (!(strncmp (name, "t_", 2))) {
		tel_comms = &ports[pnum];
	} else if (!(strncmp (name, "a_", 2))) {
		autog_comms = &ports[pnum];
	} else if (!(strncmp (name, "w_", 2))) {
		filter_comms = &ports[pnum];
	} else if (!(strncmp (name, "f_", 2))) {
		focus_comms = &ports[pnum];
	} else
	    return show_error (__func__, "Invalid port name!");
	
	return TRUE;
}

gboolean serial_open_port (struct port *p, enum PortUsers user)
{
	/* Open the given serial port */
	
	gchar c[2];
	
	/* 'open' can block in newer linux kernels if the serial device does not
	 * indicate it is ready (e.g. with data carrier detect) and some of the 
	 * devices that we connect to may not do that, so we have to open 
	 * non-blocking here.  We revert to blocking behaviour when the port is 
	 * initialised - see port_init.
	 */
	  
	//if ((p->f = open (p->name, O_RDWR | O_NOCTTY)) == -1)
	if ((p->f = open (p->name, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)
		return FALSE;
	p->ref_count++;
	p->user |= user;
	
	/* Save current port settings */
	
	tcgetattr (p->f, &p->old_tio);
	
	/* Initialise the serial port for testing */
	
	port_init (p, PS_TEST);
	
	/* Check to see if port is already in use */
	
	if (read (p->f, c, 1)) {
		c[1] = '\0';
		L_print ("{o}Comms port %s appears to be in use - read '%s' "
													 "from port\n", p->name, c);
	}
	
	/* Initialise the serial port for communications */
	
	port_init (p, PS_READY);
	
	return TRUE;	
}

void serial_close_port (struct port *p)
{
	/* Close the given serial port */
	
	/* Restore previous port settings */
	
	tcsetattr (ports[p->pnum].f, TCSANOW, &ports[p->pnum].old_tio);
	
	/* Close the device file */
	
	close (p->f);
	p->f = 0;
}

static void port_init (struct port *p, enum PortState state)
{
	/* Initialise the serial comms port */
	
	gint flags;
	
	/* Revert to blocking behaviour */
	
	flags = fcntl (p->f, F_GETFL);
    fcntl (p->f, F_SETFL, flags & ~O_NONBLOCK);
	
	/* Define new port settings */
	
	memset (&p->new_tio, 0, sizeof (p->new_tio));
	
	p->new_tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	p->new_tio.c_iflag = IGNPAR;
	p->new_tio.c_oflag = 0;
	p->new_tio.c_lflag = 0; /* Local mode: non-canonical, no echo */
 
	switch (state) {
		case PS_TEST:
			p->new_tio.c_cc[VTIME] = 10;        /* Return when any character  */
			p->new_tio.c_cc[VMIN] = 0;          /*  received or more than 1s  */
			break;                              /*  has elapsed.              */
		case PS_READY:
			p->new_tio.c_cc[VTIME] = 1;         /* Return when SER_BUFSIZ     */
			p->new_tio.c_cc[VMIN] = SER_BUFSIZ; /*  characters have been      */
			break;                              /*  received, or more than    */
	}                                           /*  0.1s between characters.  */
	
	/* Flush any pending IO */

	tcflush (p->f, TCIOFLUSH);
	
	/* Set the port settings with immediate effect */
	
	tcsetattr (p->f, TCSANOW, &p->new_tio);
}

gint s_read (gint file, gchar string[], gint bytes)
{
	/* Read from serial port */
	
	struct timeval fdtv;       /* Time struc. for monitoring period */ 
	fd_set rfds;               /* File descriptors for monitoring   */
	gint read_bytes, retval;
	gchar s[SER_BUFSIZ];
	
	memset (string, 0, bytes);

	if ((file == autog_comms->f && (autog_comms->user & PU_AUTOG)) ||
	    (file == tel_comms->f && (tel_comms->user & PU_TEL)) ||
	    (file == filter_comms->f && (filter_comms->user & PU_FILTER)) ||
	    (file == focus_comms->f && (focus_comms->user & PU_FOCUS))) {
			
		/* Poll the serial device to see if there's something to read */
			
		FD_ZERO (&rfds);    /* Will serial device file read without blocking? */
		FD_SET (file, &rfds);
		fdtv.tv_sec = 1;    /* Max. blocking delay - 1 second */
		fdtv.tv_usec = 0;
		retval = select (file + 1, &rfds, NULL, NULL, &fdtv);
		if (retval == -1) {
			L_print ("{r}s_read: Error polling serial device file\n");
			strcpy (string, "#");
			return RS_ERR_POLL;
		} else if (retval == 0) {
			strcpy (string, "#");
			G_print ("s_read: Read timeout on serial device file... no data\n");
			return RS_TIMEOUT;
		} else {
			
			/* Read from the serial device */
			
			if ((read_bytes = read (file, string, bytes)) >= 0) {
				G_print ("s_read: read %d bytes: %s\n", read_bytes, string);
				if (read_bytes < bytes) {  /* Didn't get all we expected */
					G_print ("{r}s_read: Requested %d bytes from serial device "
							 "but only got %d bytes\n", bytes, read_bytes);
				}
				
				/* Check to see if the calling routine asked for fewer bytes
				 * than were available and issue a warning if more data found.
				 */
				
				fdtv.tv_sec = 0;    /* Poll without blocking */
				fdtv.tv_usec = 0;
				retval = select (file + 1, &rfds, NULL, NULL, &fdtv);
				if (retval > 0) {
					memset (s, 0, SER_BUFSIZ);
					if (read (file, s, SER_BUFSIZ) > 0) {
						L_print ("{r}s_read: Incomplete read from serial "
								 "device - found %s still in buffer\n", s);
					}
				}
				
				return read_bytes;  /* Return number of bytes read by initial */
			} else {                /*  request, ignoring any surplus.        */
				L_print ("{r}s_read: Error reading from serial device\n");
				return RS_ERR_READ;
			}
		}
	} else {
		
		/* Requested port is not open */
		
		G_print ("s_read: Requested read port is not open!\n");
	    if (file == tel_comms->f && (tel_comms->user & PU_TEL))
		    strcpy (string, "0p#");
		else
		    strcpy (string, "#");
		return 0;
	}
}

gint s_write (gint file, gchar string[], gint bytes)
{
	/* Write to serial port */
	
	if (file == autog_comms->f && (autog_comms->user & PU_AUTOG)) {
		G_print ("s_write: %s written to %s\n", string, autog_comms->name);
		return write (file, string, bytes);
	} else if (file == tel_comms->f && (tel_comms->user & PU_TEL)) {
		G_print ("s_write: %s written to %s\n", string, tel_comms->name);
		return write (file, string, bytes);
	} else if (file == filter_comms->f && (filter_comms->user & PU_FILTER)) {
		G_print ("s_write: %s written to %s\n", string, filter_comms->name);
		return write (file, string, bytes);
	} else if (file == focus_comms->f && (focus_comms->user & PU_FOCUS)) {
		G_print ("s_write: %s written to %s\n", string, focus_comms->name);
		return write (file, string, bytes);
	} else
		G_print ("s_write: Requested write port is not open!\n");

	return 0;		
}
