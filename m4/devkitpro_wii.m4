# -*- mode: autoconf -*-
# devkitpro_wii.m4 - Macros to handle Wii setup.
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2025 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 4

# DEVKITPRO_WII_INIT
# ------------------
#
# This macro adjusts the environment for Wii homebrew.
#
# Output variables:
#   - `ELF2DOL'
#   - `GXTEXCONV'
#   - `DEVKITPRO_LIBOGC'
#   - `PATH': appends `DEVKITPRO/tools/bin' if necessary.

AC_DEFUN([DEVKITPRO_WII_INIT], [

    DEVKITPRO_PPC_INIT

    # Ensure $DEVKITPRO/tools/bin is in PATH
    DEVKITPRO_APPEND_TOOL_PATH([elf2dol], [$DEVKITPRO/tools/bin])

    AC_CHECK_PROGS([ELF2DOL], [elf2dol])
    AC_CHECK_PROGS([GXTEXCONV], [gxtexconv])

    # set DEVKITPRO_LIBOGC
    AC_ARG_VAR([DEVKITPRO_LIBOGC], [path to libogc])
    AS_VAR_SET([DEVKITPRO_LIBOGC], [$DEVKITPRO/libogc])
    AC_SUBST([DEVKITPRO_LIBOGC])

    # set DEVKITPRO_PORTLIBS_WII
    AC_ARG_VAR([DEVKITPRO_PORTLIBS_WII], [path to portlibs/wii])
    AS_VAR_SET([DEVKITPRO_PORTLIBS_WII], [$DEVKITPRO_PORTLIBS/wii])
    AC_SUBST([DEVKITPRO_PORTLIBS_WII])

    # Append portlibs/wii/bin and portlibs/ppc/bin to PATH
    # Note: we don't know if any portlibs package is installed or even needed.
    DEVKITPRO_APPEND_PATH([$DEVKITPRO_PORTLIBS_WII/bin])
    DEVKITPRO_APPEND_PATH([$DEVKITPRO_PORTLIBS_PPC/bin])

])dnl DEVKITPRO_WII_INIT


# DEVKITPRO_WII_OPT_INIT
# ----------------------
#
# Calls DEVKITPRO_WII_INIT only if `--enable-wii' argument is given.

AC_DEFUN([DEVKITPRO_WII_OPT_INIT],[

    AC_ARG_ENABLE([enable-wii],
                  [AS_HELP_STRING([--enable-wii], [build Wii homebrew])])

    AS_VAR_IF([enable_wii], [yes], [DEVKITPRO_WII_INIT])

])dnl DEVKITPRO_WII_OPT_INIT


# DEVKITPRO_WII_SETUP
# ------------------
#
# This macro adjusts compiler flags for Wii homebrew.
#
# Output variables:
#   - `CFLAGS'
#   - `CPPFLAGS'
#   - `CXXFLAGS'
#   - `LDFLAGS'
#   - `LIBS'

AC_DEFUN([DEVKITPRO_WII_SETUP], [

    AS_VAR_SET_IF([DEVKITPRO_LIBOGC], [], [AC_MSG_ERROR([DEVKITPRO_LIBOGC not set.])])
    AS_VAR_SET_IF([DEVKITPRO_PORTLIBS_WII], [], [AC_MSG_ERROR([DEVKITPRO_PORTLIBS_WII not set.])])

    DEVKITPRO_PPC_SETUP

    AX_PREPEND_FLAG([-D__WII__],                         [CPPFLAGS])
    AX_PREPEND_FLAG([-DGEKKO],                           [CPPFLAGS])
    AX_PREPEND_FLAG([-I$DEVKITPRO_PORTLIBS_WII/include], [CPPFLAGS])
    AX_PREPEND_FLAG([-I$DEVKITPRO_LIBOGC/include],       [CPPFLAGS])

    AX_PREPEND_FLAG([-mcpu=750],    [CFLAGS])
    AX_PREPEND_FLAG([-meabi],       [CFLAGS])
    AX_PREPEND_FLAG([-mrvl],        [CFLAGS])
    AX_PREPEND_FLAG([-mhard-float], [CFLAGS])

    AX_PREPEND_FLAG([-mcpu=750],    [CXXFLAGS])
    AX_PREPEND_FLAG([-meabi],       [CXXFLAGS])
    AX_PREPEND_FLAG([-mrvl],        [CXXFLAGS])
    AX_PREPEND_FLAG([-mhard-float], [CXXFLAGS])

    # link against all libogc components
    AX_PREPEND_FLAG([-logc],         [LIBS])
    AX_PREPEND_FLAG([-laesnd],       [LIBS])
    AX_PREPEND_FLAG([-lasnd],        [LIBS])
    AX_PREPEND_FLAG([-lbte],         [LIBS])
    AX_PREPEND_FLAG([-ldb],          [LIBS])
    AX_PREPEND_FLAG([-ldi],          [LIBS])
    AX_PREPEND_FLAG([-liso9660],     [LIBS])
    AX_PREPEND_FLAG([-lmad],         [LIBS])
    AX_PREPEND_FLAG([-lmodplay],     [LIBS])
    AX_PREPEND_FLAG([-ltinysmb],     [LIBS])
    AX_PREPEND_FLAG([-lwiikeyboard], [LIBS])
    AX_PREPEND_FLAG([-lwiiuse],      [LIBS])

    AX_PREPEND_FLAG([-L$DEVKITPRO_PORTLIBS_WII/lib], [LIBS])
    AX_PREPEND_FLAG([-L$DEVKITPRO_LIBOGC/lib/wii],   [LIBS])


    # custom Makefile recipes
    AX_ADD_AM_MACRO([
clean: clean-dol
.PHONY: clean-dol
clean-dol:; \$(RM) *.dol
%.dol: %.strip.elf; \$(ELF2DOL) \$< \$[@]

clean: clean-tpl
.PHONY: clean-tpl
clean-tpl:; \$(RM) *.tpl
%.tpl: %.scf; \$(GXTEXCONV) -s \$< -o \$[@]
])

])dnl DEVKITPRO_WII_SETUP



# DEVKITPRO_WII_CHECK_LIBFAT([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------------------
#
# This macro checks for the presence of libfat-ogc.
#
# Output variables:
#   - `HAVE_DEVKITPRO_WII_LIBFAT'
#   - `DEVKITPRO_WII_LIBFAT_LIBS'

AC_DEFUN([DEVKITPRO_WII_CHECK_LIBFAT], [

    AS_VAR_SET_IF([DEVKITPRO_LIBOGC], [], [AC_MSG_ERROR([DEVKITPRO_LIBOGC not set.])])

    # Note: libfat-ogc is installed inside DEVKITPRO_LIBOGC already, so we don't need to update
    # the lib path.
    DEVKITPRO_CHECK_LIBRARY_FULL([DEVKITPRO_WII_LIBFAT],
                                 [fat.h],
                                 [fat],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            AC_MSG_ERROR([libfat-ogc not found; install the package with "dkp-pacman -S libfat-ogc"])))

])dnl DEVKITPRO_WII_CHECK_LIBFAT


# DEVKITPRO_WII_CHECK_LIBGXFLUX([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------------------------------
#
# This macro checks for the presence of libgxflux.
#
# Output variables:
#   - `HAVE_DEVKITPRO_WII_LIBGXFLUX'
#   - `DEVKITPRO_WII_LIBGXFLUX_LIBS'

AC_DEFUN([DEVKITPRO_WII_CHECK_LIBGXFLUX], [

    AS_VAR_SET_IF([DEVKITPRO_LIBOGC], [], [AC_MSG_ERROR([DEVKITPRO_LIBOGC not set.])])

    # Note: libgxflux is installed inside DEVKITPRO_LIBOGC already, so we don't need to
    # update the lib path.
    DEVKITPRO_CHECK_LIBRARY_FULL([DEVKITPRO_WII_LIBGXFLUX],
                                 [gxflux/gfx.h],
                                 [gxflux],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libgxflux not found; install the package with "dkp-pacman -S libgxflux"])]))

])dnl DEVKITPRO_WII_CHECK_LIBGXFLUX
