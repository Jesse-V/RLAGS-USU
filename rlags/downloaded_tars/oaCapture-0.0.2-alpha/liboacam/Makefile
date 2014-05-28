#
# Makefile -- makefile for camera API library
#
# Copyright 2013 James Fidell (james@openastroproject.org)
#
# License:
#
# This file is part of the Open Astro Project.
#
# The Open Astro Project is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Open Astro Project is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Open Astro Project.  If not, see
# <http://www.gnu.org/licenses/>.
#
SLIB	= liboacam.a

SRCS	= oacam.c getState.c getFeatures.c control.c capture.c \
	PGRgetFeatures.c PGRgetState.c PGRinit.c PGRoacam.c \
	PWCgetFeatures.c PWCgetState.c PWCinit.c PWCoacam.c \
	QHYgetFeatures.c QHYgetState.c QHYinit.c QHYoacam.c \
	V4L2capture.c V4L2control.c V4L2getFeatures.c V4L2getState.c \
	V4L2init.c v4l2ioctl.c V4L2oacam.c \
	ZWASIcapture.c ZWASIcontrol.c ZWASIgetFeatures.c ZWASIgetState.c \
	ZWASIinit.c ZWASIoacam.c

OBJS	= $(SRCS:.c=.o)
PREREQ	= $(SRCS:.c=.P)

INCLUDES = -I../include

CFLAGS	= -g $(INCLUDES)

all:	$(SLIB)

clean:
	rm -f $(OBJS) $(SLIB) $(PREREQ)

%.o : %.c
	$(COMPILE.c) -MD -o $@ $<
	@cp $*.d $*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	  -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

$(SLIB): $(OBJS)
	$(AR) rv $@ $(OBJS)

-include $(SRCS:.c=.P)
