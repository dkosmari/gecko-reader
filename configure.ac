#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Gecko Reader], [0.1])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

DEVKITPRO_WII_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_LANG([C++])
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXXFLAGS])


AC_ARG_WITH([libgxflux],
            [AS_HELP_STRING([--without-libgxflux], [build without libgxflux])])

AS_VAR_IF([with_libgxflux], [no],
          [AC_MSG_NOTICE([building without libgxflux: $with_libgxflux])],
          [DEVKITPRO_WII_CHECK_LIBGXFLUX])


AC_CONFIG_FILES([Makefile
                 hbc/meta.xml])
AC_OUTPUT
