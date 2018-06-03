#load common setting
include $(top_srcdir)/threadpool/threadpool_am.conf

lib_LTLIBRARIES = libevent_threadpool.la

libevent_threadpool_la_SOURCES = event_tpool_manager.c event_thread.c
libevent_threadpool_la_LDFLAGS=-shared -module $(top_srcdir)/design_pattern_util/lib/libdputil.la
libevent_threadpool_la_LDFLAGS+=-Wl,--version-script,threadpool.map
