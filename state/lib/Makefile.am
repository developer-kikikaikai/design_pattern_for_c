#load common setting
include $(top_srcdir)/state/state_am.conf

noinst_LTLIBRARIES = libstate_manager.la libstate_machine.la
#
#lib_LTLIBRARIES = libbuilder.la

libstate_manager_la_SOURCES = state_manager.c
libstate_manager_la_LDFLAGS=-module -rpath $(libdir)

libstate_machine_la_SOURCES = state_machine.c
libstate_machine_la_LDFLAGS=-module -rpath $(libdir) libstate_managerla
