#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([designpettern], [1.0.0], [])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([flyweight/lib/flyweight.c])
AC_CONFIG_HEADERS([config.h])

AM_PROG_AR
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([dl], [main])
AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime memset select strerror strstr])

AC_CONFIG_FILES([Makefile
		 design_pattern_util/Makefile
		 design_pattern_util/lib/Makefile
		 design_pattern_util/include/Makefile
		 flyweight/Makefile
		 flyweight/lib/Makefile
		 flyweight/include/Makefile
		 flyweight/test/Makefile
		 builder/Makefile
		 builder/lib/Makefile
		 builder/include/Makefile
		 builder/sample_plugin/Makefile
		 builder/test/Makefile
		 publisher/Makefile
		 publisher/lib/Makefile
		 publisher/include/Makefile
		 publisher/test/Makefile
		 publisher/sample/Makefile
		 chain_of_responsibility/Makefile
		 chain_of_responsibility/lib/Makefile
		 chain_of_responsibility/include/Makefile
		 chain_of_responsibility/test/Makefile
		 mediator/Makefile
		 mediator/sample/Makefile
		 state/Makefile
		 state/lib/Makefile
		 state/include/Makefile
		 state/test/Makefile
		 ])

AC_OUTPUT
