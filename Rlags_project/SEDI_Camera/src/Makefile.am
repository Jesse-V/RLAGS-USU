## Process this file with automake to produce Makefile.in

bin_PROGRAMS = GoQat

GoQat_SOURCES = \
	interface.c \
	ccdcam.c \
	augcam.c \
	image.c \
	loop.c \
	telescope.c \
	serial.c \
	gqusb.c \
	filter.c \
	focus.c \
	tasks.c \
	video.c \
	debayer.c \
	sx.c \
	qsiapi_c.cpp \
	qsi.c \
	qsi.h \
	sx.h \
	ccd.h \
	focus.h \
	ports.h \
	telescope.h \
	gqusb.h \
	interface.h

AM_CPPFLAGS = \
	-D_GNU_SOURCE \
	-DENABLE_NLS \
	-DGOQAT_DATADIR=\"@datarootdir@/GoQat/data/\" \
	-DGLADE_INTERFACE=\"@datarootdir@/GoQat/glade/GoQat-gtkbuilder.glade\" \
	-DGRACE_TEMPS=\"@datarootdir@/GoQat/data/temps.agr\" \
	-DGRACE_TRACE=\"@datarootdir@/GoQat/data/trace.agr\" \
	-DGRACE_HFD=\"@datarootdir@/GoQat/data/hfd.agr\" \
	-DGOQAT_HFD_PL=\"@datarootdir@/GoQat/data/Analyse_HFD.pl\" \
	-DGOQAT_SEXTRACTOR_PL=\"@datarootdir@/GoQat/data/GoQat_SExtractor.pl\" \
	-DGOQAT_SEXTRACTOR_CONF=\"@datarootdir@/GoQat/data/goqat_vid_photom_sextractor.conf\" \
	-DGOQAT_SEXTRACTOR_PARAM=\"@datarootdir@/GoQat/data/goqat_vid_photom_sextractor.param\" \
	-DGOQAT_SEXTRACTOR_FILTER=\"@datarootdir@/GoQat/data/goqat_vid_photom_sextractor_filter.conv\" \
	-D_GNU_SOURCE \
	@GTK_CFLAGS@ \
	@LIBGOOCANVAS_CFLAGS@ \
	@LIBV4L2_CFLAGS@ \
	@LIBUSB_CFLAGS@ \
	@LIBUDEV_CFLAGS@ \
	@LIBRAW1394_CFLAGS@ \
	@LIBUNICAPGTK_CFLAGS@ \
	@LIBGTHREAD_CFLAGS@

AM_CFLAGS = \
	-Wall

LDADD = \
	@GTK_LIBS@ \
	@LIBGOOCANVAS_LIBS@ \
	@LIBV4L2_LIBS@ \
	@LIBUSB_LIBS@ \
	@LIBUDEV_LIBS@ \
	@LIBRAW1394_LIBS@ \
	@LIBUNICAPGTK_LIBS@ \
	@LIBGTHREAD_LIBS@
	
AM_LDFLAGS = \
	-export-dynamic
