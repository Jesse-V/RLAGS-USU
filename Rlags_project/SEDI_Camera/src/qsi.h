/******************************************************************************/
/*     HEADER FILE FOR HIGH LEVEL INTERFACE FUNCTION PROTOTYPES TO QSI API    */
/*                                                                            */
/* This file contains the function prototypes for the interface functions to  */
/* the QSI API.                                                               */
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

#ifdef HAVE_LIBQSIAPI
#define HAVE_QSI 1
#endif

#ifdef HAVE_QSI
#include "ccd.h"

extern int qsi_get_cameras (const char *serial[], const char *desc[], int *num);
extern int qsi_connect (int connect, const char *serial);
extern int qsi_get_cap (struct ccd_capability *cam_cap);
extern int qsi_set_state (enum CamState state, int ival, double dval, ...);
extern int qsi_get_state (struct ccd_state *state, gboolean AllSettings, ...);
extern int qsi_set_imagearraysize (long x, long y, long x_wid, long y_wid, 
							       long x_bin, long y_bin);
extern int qsi_start_exposure (char *dateobs, double length, int light);
extern int qsi_cancel_exposure (void);
extern int qsi_interrupt_exposure (void);
extern int qsi_get_imageready (int *ready);
extern int qsi_get_exposuretime (char *dateobs, double *length);
extern int qsi_get_imagearraysize (int *x_wid, int *y_wid, int *bytes);
extern int qsi_get_imagearray (unsigned short *array);
extern void qsi_pulseguide (enum TelMotion direction, int duration); 
extern void qsi_error_func (void (*err_func) (int *err, const char *func, 
                                              char *msg));
#endif /* HAVE_QSI */
