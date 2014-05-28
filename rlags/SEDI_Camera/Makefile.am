## Process this file with automake to produce Makefile.in

## Sort out the package directories etc...

SUBDIRS = src 

GoQatdocdir = @docdir@
GoQatdoc_DATA = README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS \
	TODO \
	docs/GoQat.pdf

GoQatgladedir = @datarootdir@/GoQat/glade
GoQatglade_DATA = GoQat-gtkbuilder.glade \
	pixmaps/north.png \
	pixmaps/south.png \
	pixmaps/east.png \
	pixmaps/west.png

GoQatpixmapsdir = @datarootdir@/GoQat/pixmaps
GoQatpixmaps_DATA = pixmaps/GoQat.png

GoQatdatadir = @datarootdir@/GoQat/data
GoQatdata_DATA = data/Defaults.dat \
	data/trace.agr \
	data/temps.agr \
	data/hfd.agr \
	data/GoQat.conf \
	data/DemoTasks.txt \
	data/DemoScript.sh \
	data/DemoScript.pl \
	data/Analyse_HFD.pl \
	data/GoQat_SExtractor.pl \
	data/goqat_vid_photom_sextractor.conf \
	data/goqat_vid_photom_sextractor.param \
	data/goqat_vid_photom_sextractor_filter.conv

EXTRA_DIST = GoQat-gtkbuilder.glade \
	pixmaps/GoQat.png \
	pixmaps/north.png \
	pixmaps/south.png \
	pixmaps/east.png \
	pixmaps/west.png \
	data/Defaults.dat \
	data/trace.agr \
	data/temps.agr \
	data/hfd.agr \
	data/GoQat.conf \
	data/DemoTasks.txt \
	data/DemoScript.sh \
	data/DemoScript.pl \
	data/Analyse_HFD.pl \
	data/GoQat_SExtractor.pl \
	data/goqat_vid_photom_sextractor.conf \
	data/goqat_vid_photom_sextractor.param \
	data/goqat_vid_photom_sextractor_filter.conv \
	docs/GoQat.pdf
	
install-data-hook:
	chmod +x $(DESTDIR)@datarootdir@/GoQat/data/*.pl $(DESTDIR)@datarootdir@/GoQat/data/*.sh; \
	sed 's,@DATAROOTDIR\@,@datarootdir@,' < $(DESTDIR)@datarootdir@/GoQat/data/DemoTasks.txt > $(DESTDIR)@datarootdir@/GoQat/data/tmp.txt; \
	mv $(DESTDIR)@datarootdir@/GoQat/data/tmp.txt $(DESTDIR)@datarootdir@/GoQat/data/DemoTasks.txt


