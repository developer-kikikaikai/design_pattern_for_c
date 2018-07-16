#load common setting
include $(top_srcdir)/threadpool/threadpool_am.conf

lib_LTLIBRARIES = libevent_threadpool.la

libevent_threadpool_la_SOURCES = event_tpool_manager.c event_thread.c

if TPOOLEVENT_SELECT
libevent_threadpool_la_SOURCES += event_if_select.c
endif
if TPOOLEVENT_LIBEVENT
libevent_threadpool_la_SOURCES += event_if_libevent.c
endif
if TPOOLEVENT_LIBEV
libevent_threadpool_la_SOURCES += event_if_libev.c
endif
if TPOOLEVENT_EPOLL
libevent_threadpool_la_SOURCES += event_if_epoll.c
endif
libevent_threadpool_la_LDFLAGS=-shared $(DPUTIL_DIR)/lib/libdputil.la -Xlinker -rpath -Xlinker $(libdir)
libevent_threadpool_la_LDFLAGS+=-Wl,--version-script,threadpool.map
