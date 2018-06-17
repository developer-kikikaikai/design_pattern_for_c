#load common setting
include $(top_srcdir)/memorypool/memorypool_am.conf

noinst_PROGRAMS = test

test_CFLAGS = -g -O0 
test_SOURCES = main.c
test_LDFLAGS = $(AM_LDFLAGS) $(MEMPOOL_DIR)/lib/libmemorypool.la
