#load common setting
include $(top_srcdir)/state/state_am.conf

lib_LTLIBRARIES = 

COMMON_LDFLAGS=-shared $(top_srcdir)/design_pattern_util/lib/libdputil.la -Xlinker -rpath -Xlinker $(libdir) -I.

if TPOOLEVENT_SELECT
lib_LTLIBRARIES += libevent_if_select.la
libevent_if_select_la_SOURCES = event_if_select.c
libevent_if_select_la_LDFLAGS=$(COMMON_LDFLAGS)
endif

if TPOOLEVENT_LIBEVENT
lib_LTLIBRARIES += libevent_if_libevent.la
libevent_if_libevent_la_SOURCES = event_if_libevent.c
libevent_if_libevent_la_LDFLAGS=$(COMMON_LDFLAGS) -levent
endif

if TPOOLEVENT_LIBEV
lib_LTLIBRARIES += libevent_if_libev.la
libevent_if_libev_la_SOURCES = event_if_libev.c
libevent_if_libev_la_LDFLAGS=$(COMMON_LDFLAGS) -lev
endif

if TPOOLEVENT_EPOLL
lib_LTLIBRARIES += libevent_if_epoll.la
libevent_if_epoll_la_SOURCES = event_if_epoll.c
libevent_if_epoll_la_LDFLAGS=$(COMMON_LDFLAGS)
endif
