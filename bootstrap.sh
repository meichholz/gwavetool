#!/bin/sh

test Makefile && make distclean

test -d aux && rm -rf aux
mkdir aux

for PAT in aclocal.m4 "*~" stamp-h.in Makefile Makefile.in configure "t.*" "tmp.*" "*.bak" ; do
  find -name "$PAT" -exec rm "{}" \;
done

rm src/gwavetool.glade t config.* configure.scan src/config.h */.deps 2>/dev/null

test "$1" = "-c" && exit

ln -s $PWD/ui/gwavetool.glade src/

autoscan
aclocal -I acextra
autoconf
autoheader
automake --add-missing --copy --include-deps --foreign

test "$1" = "-b" || exit 0

./configure
make

