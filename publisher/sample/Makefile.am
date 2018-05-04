#load common setting
include $(top_srcdir)/publisher/publisher_am.conf

bin_PROGRAMS = thread_sample

thread_sample_LDADD = $(LDPATH)/libpublisher.la

thread_sample_CFLAGS = -g -O0 
thread_sample_SOURCES = subscriber.c main.c
