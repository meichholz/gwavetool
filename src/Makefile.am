bin_PROGRAMS    = gwavetool
gwavetool_SOURCES   = app.c   dlgabout.c  main.c   waveview.c frame.c statusbar.c  wave.c

INCLUDES        = @GTK_CFLAGS@ @GLADE_CFLAGS@
LDADD           = @GTK_LIBS@ @GLADE_LIBS@ -lm -lsndfile -lpthread -lgthread-2.0
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P*

test:		gwavetool
		gwavetool
