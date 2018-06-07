#load common setting
include $(top_srcdir)/state/state_am.conf

lib_LTLIBRARIES = libstate_manager.la libstate_machine.la

libstate_manager_la_SOURCES = state_manager.c
libstate_manager_la_LDFLAGS=-shared $(top_srcdir)/design_pattern_util/lib/libdputil.la
libstate_manager_la_LDFLAGS+=-Wl,--version-script,state_manager.map

libstate_machine_la_SOURCES = state_machine.c
libstate_machine_la_LDFLAGS=-shared libstate_manager.la $(top_srcdir)/design_pattern_util/lib/libdputil.la 
libstate_machine_la_LDFLAGS+= $(THREADPOOL_DIR)/lib/libevent_threadpool.la -Xlinker -rpath -Xlinker $(libdir) 
libstate_machine_la_LDFLAGS+= -Wl,--version-script,state_machine.map
