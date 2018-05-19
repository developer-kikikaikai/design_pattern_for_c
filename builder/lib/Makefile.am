#load common setting
include $(top_srcdir)/builder/builder_am.conf

noinst_LTLIBRARIES = liblower_layer_director.la
#
#lib_LTLIBRARIES = libbuilder.la

liblower_layer_director_la_SOURCES = lower_layer_director.c builder_action.c director.c
#for statik library
liblower_layer_director_la_LDFLAGS=-rpath $(libdir)
