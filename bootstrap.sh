#!/bin/sh

test Makefile && make distclean

rm aux/*

for PAT in aclocal.m4 "*~" stamp-h.in Makefile Makefile.in configure "t.*" "tmp.*" "*.bak" ; do
  find -name "$PAT" -exec rm "{}" \;
done

rm t config.* configure.scan src/config.h 2>/dev/null

test "$1" = "-c" && exit

GNOME_PATHS=""

for i in /opt/gnome2/share/aclocal
do
  test -d $i && GNOME_PATHS="-I $i $GNOME_PATHS"
done


autoscan
aclocal $GNOME_PATHS
autoconf
autoheader
automake --add-missing --copy --include-deps --foreign

test "$1" = "-b" || exit 0

./configure
make

