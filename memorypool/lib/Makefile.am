#load common setting
include $(top_srcdir)/memorypool/memorypool_am.conf

lib_LTLIBRARIES = libmemorypool.la

libmemorypool_la_SOURCES = memorypool.c
libmemorypool_la_LDFLAGS=-shared
libmemorypool_la_LDFLAGS+=-Wl,--version-script,memorypool.map
