bin_PROGRAMS    = gwavetool
gwavetool_SOURCES   = app.c dlgnew.c dlgabout.c main.c waveview.c frame.c statusbar.c wave.c

INCLUDES        = @GLADE_CFLAGS@
LDADD           = @GLADE_LIBS@ -lm -lsndfile -lpthread -lgthread-2.0
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P*

test:		gwavetool
		gwavetool
