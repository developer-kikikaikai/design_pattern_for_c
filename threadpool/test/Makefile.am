#load common setting
include $(top_srcdir)/threadpool/threadpool_am.conf

noinst_PROGRAMS = test

test_CFLAGS = -g -O0 
test_SOURCES = test_main.c main.c
test_LDFLAGS=${AM_LDFLAGS}  $(DPUTIL_DIR)/lib/libdputil.la -Xlinker -rpath -Xlinker $(libdir)
