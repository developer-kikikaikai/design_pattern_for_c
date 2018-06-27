#load common setting
AM_CPPFLAGS=-I$(top_srcdir)/design_pattern_util/include/ -I$(top_srcdir)/design_pattern_util/include/
INCLUDES_PATH=$(top_srcdir)/design_pattern_util/include/ $(top_srcdir)/design_pattern_util/include/

lib_LTLIBRARIES = libdputil.la
#
libdputil_la_SOURCES = dp_util.c dp_timelog.c
libdputil_la_LDFLAGS=-shared
