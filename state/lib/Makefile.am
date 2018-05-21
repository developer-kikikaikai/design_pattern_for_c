#load common setting
include $(top_srcdir)/state/state_am.conf

lib_LTLIBRARIES = libstate_manager.la libstate_machine.la

libstate_manager_la_SOURCES = state_manager.c
libstate_manager_la_LDFLAGS=-shared -module -rpath $(libdir) $(top_srcdir)/design_pattern_util/lib/libdputil.la
libstate_manager_la_LDFLAGS+=-Wl,--version-script,state_manager.map

libstate_machine_la_SOURCES = state_machine.c
libstate_machine_la_LDFLAGS=-shared -module -rpath $(libdir) libstate_managerla $(top_srcdir)/design_pattern_util/lib/libdputil.la
libstate_machine_la_LDFLAGS+=-Wl,--version-script,state_machine.map
