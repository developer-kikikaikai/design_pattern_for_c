#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([designpettern], [0.01], [])
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
AC_CHECK_LIB([event], [main])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime memset select strerror strstr])


###Disable option
#without builder
AC_ARG_WITH([builder],
  [AC_HELP_STRING([--without-builder],
    [exclude builder pattern library]
  )],
  [with_builder=$withval],
  [with_builder=yes]
)
#check flag
AM_CONDITIONAL([USE_BUILDER], [test "$with_builder" = yes])

#without publisher
AC_ARG_WITH([publisher],
  [AC_HELP_STRING([--without-publisher],
    [exclude publisher pattern library]
  )],
  [with_publisher=$withval],
  [with_publisher=yes]
)
#check flag
AM_CONDITIONAL([USE_PUBLISHER], [test "$with_publisher" = yes])

#without state
AC_ARG_WITH([state],
  [AC_HELP_STRING([--without-state],
    [exclude state pattern/state machine library]
  )],
  [with_state=$withval],
  [with_state=yes]
)
#check flag
AM_CONDITIONAL([USE_STATE], [test "$with_state" = yes])

#without flyweight
AC_ARG_WITH([flyweight],
  [AC_HELP_STRING([--without-flyweight],
    [exclude flyweight pattern library]
  )],
  [with_flyweight=$withval],
  [with_flyweight=yes]
)
#check flag
AM_CONDITIONAL([USE_FLYWEIGHT], [test "$with_flyweight" = yes])

#without cor
AC_ARG_WITH([cor],
  [AC_HELP_STRING([--without-cor],
    [exclude chain of responsibility pattern library, if flyweight is excluded, exclude it.]
  )],
  [with_cor=$withval],
  [with_cor=yes]
)
#check flag
AM_CONDITIONAL([USE_COR], [test "$with_cor" = yes -a "$with_flyweight" = yes])

#without cor
AC_ARG_WITH([threadpool],
  [AC_HELP_STRING([--without-threadpool],
    [exclude threadpool pattern library.]
  )],
  [with_threadpool=$withval],
  [with_threadpool=yes]
)
#check flag
AM_CONDITIONAL([USE_THREADPOOL], [test "$with_threadpool" = yes])

#without sample
AC_ARG_WITH([sample],
  [AC_HELP_STRING([--without-sample],
    [exclude only sample design pattern compile]
  )],
  [with_sample=$withval],
  [with_sample=yes]
)
#check flag
AM_CONDITIONAL([USE_SAMPLE], [test "$with_sample" = yes])

###
# Unit test option
AC_ARG_ENABLE(coverage,
  [AC_HELP_STRING([--enable-coverage],
    [enable coverage test, only for developer of this package. Please "Don't" use this package if you install it, gcov is GPL license [[default=no]]]
  )],
[\
case "${enableval}" in
 yes) enable_coverage=yes ;;
 *)   AC_MSG_ERROR(bad value for --enable-) ;;
esac],
enable_coverage=no)
#check flag, and add gcov option
if test x"${enable_coverage}" = x"yes"; then
  AC_CHECK_LIB([gcov], [main])
  GCOV_OPT="-fprofile-arcs -ftest-coverage"
  CFLAGS="-g -O0"
  AC_SUBST([GCOV_OPT])
fi

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
		 state/Makefile
		 state/lib/Makefile
		 state/include/Makefile
		 state/test/Makefile
		 mediator/Makefile
		 mediator/sample/Makefile
		 proxy/Makefile
		 proxy/sample1/Makefile
		 proxy/sample2/Makefile
		 threadpool/Makefile
		 threadpool/lib/Makefile
		 threadpool/include/Makefile
		 threadpool/test/Makefile
		 ])

AC_OUTPUT
