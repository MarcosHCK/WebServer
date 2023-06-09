# Copyright 2023 MarcosHCK
# This file is part of WebServer.
#
# WebServer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# WebServer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with WebServer. If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.69])

#
# Take version information
# Extract mayor, minor and micro version
# numbers from version string
# Also define release type
#

m4_define([v_VERSION], m4_esyscmd_s([git describe]))
m4_define([v_MAJOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\1/']))
m4_define([v_MINOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\2/']))
m4_define([v_MICRO], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\3/']))
m4_define([v_STAGE], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\4/']))

#
# Initialize autoconf
#

AC_INIT([webserver], [v_VERSION], [nonuser@nonhost.none], [webserver], [https://github.com/MarcosHCK/webserver/])
AC_CONFIG_SRCDIR([src/webserver.c])
AC_LANG([C])

#
# Configure aclocal path
#

AC_CONFIG_MACRO_DIRS([m4])

#
# Setup default cross-compiling
# environment mechanics
#

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#
# Initialize automake
#

AM_INIT_AUTOMAKE([subdir-objects])

#
# Custom build options
#

AC_ARG_ENABLE(
  [debug],
  [AS_HELP_STRING(
    [--enable-debug],
    [Enable debug object output @<:@default=no@:>@])],
  [ AM_CONDITIONAL([DEBUG], [test "x$enableval" != "xno"])
    AS_IF(
      [test "x$enableval" != "xno"],
      [ AC_DEFINE([DEBUG], [1], [Debugging enabled])
        AC_SUBST([DEBUG], [1]) ],
      [ AC_DEFINE([DEBUG], [0], [Debugging disabled])
        AC_SUBST([DEBUG], [0]) ]) ],
  [ AM_CONDITIONAL([DEBUG], [test 0 = 1])
    AC_DEFINE([DEBUG], [0], [Debugging disabled])
    AC_SUBST([DEBUG], [0])])

AC_ARG_ENABLE(
  [developer],
  [AS_HELP_STRING(
    [--enable-developer],
    [Enable developer features @<:@default=no@:>@])],
  [AM_CONDITIONAL([DEVELOPER], [test "x$enableval" != "xno"])
   AS_IF(
    [test "x$enableval" != "xno"],
    [ AC_DEFINE([DEVELOPER], [1], [Developer features enabled])
      AC_SUBST([DEVELOPER], [1]) ],
    [ AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
      AC_SUBST([DEVELOPER], [0]) ]) ],
  [ AM_CONDITIONAL([DEVELOPER], [test 0 = 1])
    AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
    AC_SUBST([DEVELOPER], [0])])

AC_SUBST([PACKAGE_VERSION_MAJOR], [v_MAJOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MAJOR], [v_MAJOR], [Version mayor number])
AC_SUBST([PACKAGE_VERSION_MINOR], [v_MINOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MINOR], [v_MINOR], [Version minor number])
AC_SUBST([PACKAGE_VERSION_MICRO], [v_MICRO])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MICRO], [v_MICRO], [Version micro number])
AC_SUBST([PACKAGE_RELEASE], ["v_STAGE"])
AC_DEFINE([PACKAGE_RELEASE], ["v_STAGE"], [Release stage])

#
# Checks for programs we'll
# need to build
#

LT_PREREQ([2.4.6])
LT_INIT

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_RANLIB

PKG_PROG_PKG_CONFIG

AC_CHECK_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [glib-compile-resources], [no])
AS_IF([test "x$GLIB_COMPILE_RESOURCES" = "xno"], [AC_MSG_FAILURE([glib-compile-resources not found our your system])])
AC_CHECK_PROG([GLIB_GENMARSHAL], [glib-genmarshal], [glib-genmarshal], [no])
AS_IF([test "x$GLIB_GENMARSHAL" = "xno"], [AC_MSG_FAILURE([glib-genmarshal not found our your system])])
AC_CHECK_PROG([GPERF], [gperf], [gperf], [no])
AS_IF([test "x$GPERF" = "xno"], [AC_MSG_FAILURE([gperf not found our your system])])

#
# Check for libraries using pkg-config
# Note: keep sorted alphabetically 
#

PKG_CHECK_MODULES([GIO], [gio-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0])
PKG_CHECK_MODULES([GTK], [gtk+-3.0])

#
# Check for libraries
#

#
# Checks for header files.
#

#
# Checks for typedefs, structures, and compiler characteristics.
#

#
# Checks for library functions.
#

AC_FUNC_REALLOC
AC_CHECK_FUNCS([memcpy])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])

#
# Prepare output
#

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
    src/Makefile
    Makefile
  ])

AC_OUTPUT
