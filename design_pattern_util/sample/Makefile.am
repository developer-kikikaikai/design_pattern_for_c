#load common setting
include $(top_srcdir)/am.conf

noinst_PROGRAMS = test

test_CFLAGS = -g -O0 
test_SOURCES = main.c
test_LDFLAGS = $(AM_LDFLAGS) $(top_srcdir)/design_pattern_util/lib/libdputil.la
