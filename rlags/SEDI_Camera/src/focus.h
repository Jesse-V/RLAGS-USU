/******************************************************************************/
/*                      HEADER FILE FOR FOCUSER ROUTINES                      */
/*                                                                            */
/* Header file for focuser routines.                                          */
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

#ifndef GOQAT_FOCUS_H
#define GOQAT_FOCUS_H

enum FocusCmd {
	FC_VERSION =        0x0001,
	FC_MOVE_TO =        0x0002,
	FC_MOVE_BY =        0x0004,
	FC_STOP =           0x0008,
	FC_MAX_TRAVEL_GET = 0x0010,
	FC_MAX_TRAVEL_SET = 0x0020,
	FC_CUR_POS_GET =    0x0040,
	FC_CUR_POS_SET =    0x0080,
	FC_BACKLASH_GET =   0x0100,
	FC_BACKLASH_SET =   0x0200,
	FC_MOTOR_GET =      0x0400,
	FC_MOTOR_SET =      0x0800,
	FC_TEMP_GET =       0x1000
};

struct focus {
	enum FocusCmd cmd;
	gint move_by;
	gint move_to;
	gint max_travel;
	gint cur_pos;
	gint backlash_steps;
	gint step_size;
	gint step_pause;
	gint duty_cycle;
	gfloat temp;
	gfloat version;
	gboolean BacklashIn;
	gboolean Error;
};

#endif /* GOQAT_FOCUS_H */

