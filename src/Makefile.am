bin_PROGRAMS    = gwavetool
gwavetool_SOURCES   = app.cc   dlgabout.cc  main.cc   options.cc    toolbox.cc waveview.cc base.cc  frame.cc     menus.cc  statusbar.cc  wave.cc

INCLUDES        = @GTK_CFLAGS@
LDADD           = @GTK_LIBS@ -lm -lsndfile -lpthread -lgthread-2.0
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P

menus.hc : menus.rc gwavetool.h
	../tools/rcrc.pl menus.rc menus.hc

menus.o : menus.hc
