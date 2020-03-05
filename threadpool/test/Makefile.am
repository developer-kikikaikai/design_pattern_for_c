#load common setting
include $(top_srcdir)/threadpool/threadpool_am.conf

noinst_PROGRAMS = test

test_CFLAGS = -g -O0 

#Set define option related to event select
if TPOOLEVENT_SELECT
test_CFLAGS += -DTPOOLEVENT_SELECT
endif
if TPOOLEVENT_LIBEVENT
test_CFLAGS += -DTPOOLEVENT_LIBEVENT
endif
if TPOOLEVENT_LIBEV
test_CFLAGS += -DTPOOLEVENT_LIBEV
endif
if TPOOLEVENT_EPOLL
test_CFLAGS += -DTPOOLEVENT_EPOLL
endif


test_SOURCES = test_main.c main.c
test_LDFLAGS=${AM_LDFLAGS}  $(DPUTIL_DIR)/lib/libdputil.la -Xlinker -rpath -Xlinker $(libdir)
