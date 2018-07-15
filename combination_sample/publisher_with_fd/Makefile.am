#load common setting
include $(top_srcdir)/am.conf

noinst_PROGRAMS = publish_with_fdevent

publish_with_fdevent_CXXFLAGS = -g -O0
publish_with_fdevent_CFLAGS = -g -O0 -I$(PUBLISHER_DIR)/include -I$(THREADPOOL_DIR)/include
publish_with_fdevent_SOURCES = main.c 
publish_with_fdevent_LDADD= $(THREADPOOL_DIR)/lib/libevent_threadpool.la $(PUBLISHER_DIR)/lib/libpublisher.la
