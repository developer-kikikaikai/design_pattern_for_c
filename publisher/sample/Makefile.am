#load common setting
#include $(top_srcdir)/publisher/publisher_am.conf

AM_CPPFLAGS=-I$(top_srcdir)/publisher/include
INCLUDES_PATH=$(top_srcdir)/publisher/include

LDPATH=$(top_srcdir)/publisher/lib/

bin_PROGRAMS = thread_sample

thread_sample_LDADD = $(LDPATH)/libpublisher.la ../../design_pattern_util/lib/.libs/libdputil.a

thread_sample_CFLAGS = -g -O0 
thread_sample_SOURCES = subscriber.c main.c
