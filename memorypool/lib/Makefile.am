#load common setting
include $(top_srcdir)/memorypool/memorypool_am.conf

lib_LTLIBRARIES = libmemorypool.la

libmemorypool_la_SOURCES = memorypool.c
libmemorypool_la_LDFLAGS=-shared
if USE_GNU_VERSION_SCRIPT
libmemorypool_la_LDFLAGS+=-Wl,--version-script,memorypool.map
endif
