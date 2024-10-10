#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Gecko Reader], [0.2+])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

DEVKITPRO_WII_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_LANG([C++])
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])

DEVKITPRO_WII_CHECK_LIBGXFLUX

AC_CONFIG_FILES([Makefile
                 hbc/meta.xml])
AC_OUTPUT
