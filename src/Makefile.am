bin_PROGRAMS    = gwavetool
gwavetool_SOURCES   = app.cc   dlgabout.cc  main.cc   options.cc    toolbox.cc waveview.cc base.cc  frame.cc statusbar.cc  wave.cc

INCLUDES        = @GTK_CFLAGS@
LDADD           = @GTK_LIBS@ -lm -lsndfile -lpthread -lgthread-2.0
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P
