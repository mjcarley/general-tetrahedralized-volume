AC_INIT
AC_CONFIG_SRCDIR([src/gtv.h])

# Making releases:
#   GTV_MICRO_VERSION += 1;
#   GTV_INTERFACE_AGE += 1;
#   GTV_BINARY_AGE += 1;
# if any functions have been added, set GTV_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set GTV_BINARY_AGE and GTV_INTERFACE_AGE to 0.
#
GTV_MAJOR_VERSION=0
GTV_MINOR_VERSION=1
GTV_MICRO_VERSION=0
GTV_INTERFACE_AGE=0
GTV_BINARY_AGE=0
GTV_VERSION=$GTV_MAJOR_VERSION.$GTV_MINOR_VERSION.$GTV_MICRO_VERSION

AC_SUBST(GTV_MAJOR_VERSION)
AC_SUBST(GTV_MINOR_VERSION)
AC_SUBST(GTV_MICRO_VERSION)
AC_SUBST(GTV_VERSION)

# libtool versioning
LT_RELEASE=$GTV_MAJOR_VERSION.$GTV_MINOR_VERSION
LT_CURRENT=`expr $GTV_MICRO_VERSION - $GTV_INTERFACE_AGE`
LT_REVISION=$GTV_INTERFACE_AGE
LT_AGE=`expr $GTV_BINARY_AGE - $GTV_INTERFACE_AGE`
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

# For automake.
VERSION=$GTV_VERSION
PACKAGE=gtv

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

# Specify a configuration file
AM_CONFIG_HEADER(config.h)

AC_DEFINE_UNQUOTED(GTV_MAJOR_VERSION, $GTV_MAJOR_VERSION, [Major version])
AC_DEFINE_UNQUOTED(GTV_MINOR_VERSION, $GTV_MINOR_VERSION, [Minor version])
AC_DEFINE_UNQUOTED(GTV_MICRO_VERSION, $GTV_MICRO_VERSION, [Micro version])
AC_DEFINE_UNQUOTED(GTV_INTERFACE_AGE, $GTV_INTERFACE_AGE, [Interface age])
AC_DEFINE_UNQUOTED(GTV_BINARY_AGE, $GTV_BINARY_AGE, [Binary age])

dnl Initialize libtool
AM_PROG_LIBTOOL

dnl Initialize maintainer mode
AM_MAINTAINER_MODE

AC_PROG_CC


if test x$with_developer_debug = xyes ; then
  echo \#################################################################
  echo \#
  echo \# Developer debug mode: are you the developer? If not, how did
  echo \# you know about this and do you know what you are doing?
  echo \#
  echo \#################################################################
  AC_DEFINE_UNQUOTED(DEVELOPER_DEBUG, $DEVELOPER_DEBUG, [Debug messages for developer])
fi


if test x$GCC = xyes ; then
  CFLAGS="$CFLAGS -Wall -Werror-implicit-function-declaration -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations"
fi

AC_PROG_AWK
AC_ISC_POSIX
AC_HEADER_STDC

AC_CHECK_PROG(GTS, gts-config, yes, no)
if test x$GTS = xyes ; then
   gts_cflags=`gts-config --cflags`
   gts_libs=`gts-config --libs`
else
  echo \#################################################################
  echo \#
  echo \# GNU Triangulated Surface Library not available or not
  echo \# properly installed. You can get the most recent version from 
  echo \#      http://gts.sourceforge.net/
  echo \#
  echo \#################################################################
  exit
fi

AC_SUBST(gts_cflags)
AC_SUBST(gts_libs)

CFLAGS="$CFLAGS $gts_cflags"
LDFLAGS="$LDFLAGS $gts_libs"

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_CHECK_LIB(m, sqrt)
#AC_CHECK_LIB(verdict, v_quad_maximum_angle,,[AC_MSG_NOTICE(
#             [** libverdict not found, some features will not be available])])

LIBS="$gts_libs $LIBS"

dnl header file checks
AC_CHECK_HEADERS(fpu_control.h, AC_DEFINE(HAVE_FPU_CONTROL_H))
AC_CHECK_HEADERS(floatingpoint.h, AC_DEFINE(HAVE_FLOATINGPOINT_H))
AC_CHECK_HEADERS(unistd.h, AC_DEFINE(HAVE_UNISTD_H))
AC_CHECK_HEADERS(getopt.h, AC_DEFINE(HAVE_GETOPT_H))
#AC_CHECK_HEADERS(verdict.h)

dnl functions checks
AC_CHECK_FUNCS(getopt_long)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/gtv-config
tools/Makefile
doc/Makefile
doc/html/Makefile
])
AC_OUTPUT
