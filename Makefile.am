## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = COPYING.LESSER INSTALL README.md
SUBDIRS = include src tests doc
SUBLIBS = src/libautoptr_src.la

lib_LTLIBRARIES = libautoptr.la
libautoptr_la_SOURCES =

#
# The rules of thumb, when dealing with these values are:
#
#    Always increase the revision value.
#
#    Increase the current value whenever an interface has been added, removed or changed.
#
#    Increase the age value only if the changes made to the ABI are backward compatible.
#
# See: https://autotools.io/libtool/version.html
#
# version-info current:revision:age
#
libautoptr_la_LDFLAGS=-rpath '$(libdir)' -version-info 2:2:2
libautoptr_la_LIBADD = $(SUBLIBS)

pkgincludedir = ${includedir}
#pkginclude_HEADERS = include/autoptr.h

docs:
	(cd doc && make docs)

