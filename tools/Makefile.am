## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src -I$(includedir)
LDADD = $(top_builddir)/src/.libs/libgtv.a 

DEPS = $(top_builddir)/src/libgtv.la

bin_PROGRAMS = \
	gtv-delaunay \
	gtv-cube \
	gtv-check \
	gtv2msh \
	msh2gtv
