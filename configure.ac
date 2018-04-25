#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([designpettern], [1.0.0], [])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([flyweight/lib/flyweight.c])
AC_CONFIG_HEADERS([config.h])

AM_PROG_AR
LT_INIT

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

#AC_ARG_ENABLE( option, help message, script code for this option, default value)
AC_ARG_ENABLE(flyweight_forcesafe,
[  --enable-flyweight_forcesafe      enable to lock/unlock register data [[default=no]]],
[\
#
case "${enableval}" in
 yes) flyweight_forcesafe=yes ;;
 *)   AC_MSG_ERROR(bad value for --enable-) ;;
esac],
flyweight_forcesafe=no)

#check flag and set defile
if test x"${flyweight_forcesafe}" = x"yes"; then
  AC_DEFINE(STRICT_ENSUCE_THREADSAGE, 1, [Define to 1 if you want to ensure thread safe])
else
echo "libnotify is disabled"
fi

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime memset strerror])

AC_CONFIG_FILES([Makefile
		 flyweight/Makefile
		 flyweight/lib/Makefile
		 flyweight/include/Makefile
		 flyweight/test/Makefile
		 flyweight/sample/Makefile
		 flyweight/sample2/Makefile
		 flyweight/interface_sample/Makefile
		 ])

AC_OUTPUT
