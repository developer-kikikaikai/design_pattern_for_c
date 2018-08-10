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
AM_CONDITIONAL([USE_BUILDER], [test "$with_builder" = yes])

#without threadpool
AC_ARG_WITH([threadpool],
  [AC_HELP_STRING([--without-threadpool],
    [exclude threadpool pattern library.]
  )],
  [with_threadpool=$withval],
  [with_threadpool=yes]
)
#check flag
AM_CONDITIONAL([USE_THREADPOOL], [test "$with_threadpool" = yes])

#without threadpool event_if
# without libev
AC_ARG_ENABLE(threadpool-libev,
  [AC_HELP_STRING([--disable-threadpool-libev],
    [disable event_if_libev pluigni plugin_if_libev.so, if you want enable, please install libev4. Please care this license is "GPL" license [[default=yes]]]
  )],
  [with_plugin_libev="$withval"],
  [with_plugin_libev="yes"]
)

tpool_event_libev=no
if [ test "${with_plugin_libev}" = "yes" ]; then
  PKG_CHECK_MODULES( [LIBEV], [libev], [tpool_event_libev="yes"], [
     AC_CHECK_HEADERS(
       [ev.h], [
         AC_CHECK_LIB(
           [ev],
           [ev_io_start],
           [tpool_event_libev="yes"],
           [AC_MSG_ERROR([libev not found, please install libev4 or add option --disable-threadpool-libev])]
         )], [
           AC_MSG_ERROR([libev header found, please install libev4 or add option --disable-threadpool-libev])
         ])
    ])
fi

#without threadpool event_if
# without libevent
AC_ARG_ENABLE(threadpool-libevent,
  [AC_HELP_STRING([--disable-threadpool-libevent],
    [disable event_if_libevent pluigni plugin_if_libevent.so, if you want enable, please install libevent-dev (event2) [[default=yes]]]
  )],
  [with_plugin_libevent=${enableval}],
  [with_plugin_libevent="yes"]
)
tpool_event_libevevent=no
if [ test "${with_plugin_libevent}" = "yes" ]; then
  PKG_CHECK_MODULES( [LIBEVENT], [libevent], [
     AC_CHECK_LIB([event],[event_new])
     tpool_event_libevent="yes"
   ], [
     AC_CHECK_HEADERS(
       [event/evenv2.h], [
         AC_CHECK_LIB(
           [event],
           [event_new],
           [tpool_event_libevent="yes"],
           [AC_MSG_ERROR([libevnet not found, please install libevent-dev or add option --disable-threadpool-libevent.])]
         )], [
           AC_MSG_ERROR([libevent header found, please install libevent-dev or add option --disable-threadpool-libevent.])
         ])
    ])
fi

#without threadpool event_if
# without libselect
AC_ARG_ENABLE(threadpool-select,
  [AC_HELP_STRING([--disable-threadpool-select],
    [disable event_if_select pluigni plugin_if_select.so,  [[default=yes]]]
  )],
  [AC_MSG_NOTICE(["$withval"]);tpool_event_select="$withval"],
  [tpool_event_select="yes"]
)

#without threadpool event_if
# without libepoll
AC_ARG_ENABLE(threadpool-epoll,
  [AC_HELP_STRING([--disable-threadpool-epoll],
    [disable event_if_epoll pluigni plugin_if_epoll.so,  [[default=yes]]]
  )],
  [tpool_event_epoll="$withval"],
  [tpool_event_epoll="yes"]
)

#change threadpool fd max
AC_ARG_WITH([threadpool-maxfd],
  [AC_HELP_STRING([--with-threadpool-maxfd=value],
    [set max fd size (default: 64). fd size of select is always fixed at 15]
  )],
  [threadpool_maxfd=$withval],
  [threadpool_maxfd=64]
)

#check flag
event_other_select="no"
if [ test "$tpool_event_libev" = "yes" ]; then
   AC_MSG_NOTICE([======threadpool event:libev (default: unuse)===============])
   event_other_select="yes"
fi
if [ test "$tpool_event_libevent" = "yes" ]; then
   AC_MSG_NOTICE([======threadpool event:libevent(default priority 1)============])
   event_other_select="yes"
fi
if [ test "$tpool_event_epoll" = "yes" ]; then
   AC_MSG_NOTICE([======threadpool event:epoll(default priority 2)================])
   event_other_select="yes"
fi
if [ test "$tpool_event_select" = "yes" ]; then
  AC_MSG_NOTICE([======threadpool event:select(default priority 3)==============])
fi

AM_CONDITIONAL([TPOOLEVENT_LIBEVENT], [test "$tpool_event_libevent" = "yes"])
AM_CONDITIONAL([TPOOLEVENT_LIBEV], [test "$tpool_event_libev" = "yes"])
if [ test "$tpool_event_libev" = "yes"] ; then
	AC_DEFINE([USE_LIBEV], [1], [when use libev])
fi

AM_CONDITIONAL([TPOOLEVENT_EPOLL], [test "$tpool_event_epoll" = "yes"])
AM_CONDITIONAL([TPOOLEVENT_SELECT], [test "$tpool_event_select" = "yes"])
if [ test "$tpool_event_select" = "yes" ]; then
	AC_DEFINE([EV_TPOLL_MAXFDS_SELECT], [15], ["Define Threadpool MAX FD SIZE (this value * 64)"])
fi

if [ test "$event_other_select" = "yes" ] ; then
	AC_DEFINE([EV_TPOLL_MAXFDS], [1], ["Define Threadpool MAX FD SIZE (this value * 64)"])
	$as_echo "#define EV_TPOLL_MAXFDS ${threadpool_maxfd}" >>confdefs.h
fi

#add prefix path
AC_DEFINE([EVENT_IF_PLUGIN_PATH], ["dummy"], ["Define path of default plugin"])
case ${prefix} in
  NONE) $as_echo "#define EVENT_IF_PLUGIN_PATH \"${ac_default_prefix}/lib\"" >>confdefs.h;;
  *) $as_echo "#define EVENT_IF_PLUGIN_PATH \"${prefix}/lib\"" >>confdefs.h;;
esac

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
    [exclude state pattern/state machine library, if threadpool is excluded, exclude it.]
  )],
  [with_state=$withval],
  [with_state=yes]
)
#check flag
AM_CONDITIONAL([USE_STATE], [test "$with_state" = yes -a "$with_threadpool" = yes])

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

#without memorypool
AC_ARG_WITH([memorypool],
  [AC_HELP_STRING([--without-memorypool],
    [exclude memorypool library]
  )],
  [with_memorypool=$withval],
  [with_memorypool=yes]
)
#check flag
AM_CONDITIONAL([USE_MEMPOOL], [test "$with_memorypool" = yes])

#without prototype
AC_ARG_WITH([prototype],
  [AC_HELP_STRING([--without-prototype],
    [exclude prototype library]
  )],
  [with_prototype=$withval],
  [with_prototype=yes]
)
#check flag
AM_CONDITIONAL([USE_PROTOTYPE], [test "$with_prototype" = yes])

#without mement
AC_ARG_WITH([mement],
  [AC_HELP_STRING([--without-mement],
    [exclude mement library]
  )],
  [with_mement=$withval],
  [with_mement=yes]
)
#check flag
AM_CONDITIONAL([USE_MEMENT], [test "$with_mement" = yes])

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

####
# Unit test option
#AC_ARG_ENABLE(coverage,
#  [AC_HELP_STRING([--enable-coverage],
#    [enable coverage test, only for developer of this package. Please "Don't" use this package if you install it, gcov is GPL license [[default=no]]]
#  )],
#[\
#case "${enableval}" in
# yes) enable_coverage=yes ;;
# *)   AC_MSG_ERROR(bad value for --enable-) ;;
#esac],
#enable_coverage=no)
##check flag, and add gcov option
#if test x"${enable_coverage}" = x"yes"; then
#  AC_CHECK_LIB([gcov], [main])
#  GCOV_OPT="-fprofile-arcs -ftest-coverage"
#  CFLAGS="-g -O0"
#  AC_SUBST([GCOV_OPT])
#fi

AC_CONFIG_FILES([Makefile
		 design_pattern_util/Makefile
		 design_pattern_util/lib/Makefile
		 design_pattern_util/include/Makefile
		 design_pattern_util/sample/Makefile
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
		 threadpool/Makefile
		 threadpool/lib/Makefile
		 threadpool/event_if/Makefile
		 threadpool/include/Makefile
		 threadpool/test/Makefile
		 state/Makefile
		 state/lib/Makefile
		 state/include/Makefile
		 state/test/Makefile
		 memorypool/Makefile
		 memorypool/lib/Makefile
		 memorypool/include/Makefile
		 memorypool/test/Makefile
		 prototype/Makefile
		 prototype/lib/Makefile
		 prototype/include/Makefile
		 prototype/test/Makefile
		 mement/Makefile
		 mement/lib/Makefile
		 mement/include/Makefile
		 mement/test/Makefile
		 combination_sample/Makefile
		 combination_sample/publisher_with_fd/Makefile
		 ])

AC_OUTPUT
