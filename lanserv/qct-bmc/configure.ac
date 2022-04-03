#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([qst_mode], [0.0.1], [double.chiang@gmail.com])
AM_INIT_AUTOMAKE([])
AC_CONFIG_SRCDIR([qct_mod.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strdup strerror])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
LT_INIT([shared disable-static])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
