SUBDIRS=src
PACKAGE=gwavetool
AC_INIT(src/main.c)
AC_PREREQ(2.57)
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE(gwavetool,0.0.9)
AC_REVISION($Revision: 1.6 $)
AC_PROG_CC
dnl <<y=yScale+cyWave*pWave->PeekSample(iChannel,li)/MAXSHORT;>> demands for this:
AC_PROG_GCC_TRADITIONAL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_C_CONST
AC_C_VOLATILE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SETPGRP
AC_FUNC_STAT
AC_HEADER_STDBOOL
AC_HEADER_SYS_WAIT
AC_TYPE_PID_T

AC_HEADER_STDC
AC_CHECK_LIB(m,floor,LIBS="-lm $LIBS")
AC_CHECK_HEADERS(fcntl.h values.h sys/ioctl.h,[],AC_MSG_ERROR(need some headers))
AC_CHECK_FUNCS(floor memset,[],AC_MSG_ERROR(need some API calls))

AC_CHECK_HEADERS(sndfile.h,[],AC_MSG_ERROR(need sndfile.h))
AC_CHECK_LIB(sndfile,sf_open,LIBS="-lsndfile $LIBS",AC_MSG_ERROR(need sndfile library))

AC_CHECK_HEADERS(pthread.h,[],AC_MSG_ERROR(need pthread.h))
AC_CHECK_LIB(pthread,pthread_detach,LIBS="-lpthread $LIBS",AC_MSG_ERROR(need pthread library))


dnl PKG_CHECK_MODULES(GLADE, libglade-2.0 libgnomeui-2.0 >= 1.110.0)
PKG_CHECK_MODULES(GLADE, libglade-2.0)
AC_SUBST(GLADE_CFLAGS)
AC_SUBST(GLADE_LIBS)

dnl AM_PATH_GLIB_2_0(2.0.0, gaveglib=yes,AC_MSG_ERROR(need gthread-2.0),gthread)
dnl AM_PATH_GTK_2_0(2.0.0, havegtk=yes, AC_MSG_ERROR(need gtk-2.0),gthread)

AC_CHECK_PROG(acme_have_podman,pod2man,yes)
AM_CONDITIONAL(HAVE_POD2MAN,test x$acme_have_podman = xyes)

AM_CONFIG_HEADER(src/config.h)
AC_OUTPUT(Makefile src/Makefile)

