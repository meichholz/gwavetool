bin_PROGRAMS    = gwavetool
gwavetool_SOURCES   = app.c   dlgabout.c  main.c   waveview.c frame.c statusbar.c  wave.c

INCLUDES        = @GTK_CFLAGS@
LDADD           = @GTK_LIBS@ -lm -lsndfile -lpthread -lgthread-2.0
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P
