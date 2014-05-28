/******************************************************************************/
/*               HEADER FILE FOR TELESCOPE INTERFACE ROUTINES                 */
/*                                                                            */
/* Header file for telescope interface routines.                              */
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

#ifndef GOQAT_TELESCOPE_H
#define GOQAT_TELESCOPE_H

enum TelMotion {                 /* Telescope motion directions               */
	TM_NONE = 0,
	TM_EAST = 1,
	TM_WEST = 2,
	TM_NORTH = 4,
	TM_SOUTH = 8,
	TM_ALL = 15,
};

enum MotionDirection {           /* Nominal motion directions of star on      */
	H = 1,                       /*  autoguider display (although may be      */ 
	V                            /*  rotated in practice).  Horizontal implies*/
};                               /*  east-west, Vertical implies north-south  */

#endif /* GOQAT_TELESCOPE_H */
