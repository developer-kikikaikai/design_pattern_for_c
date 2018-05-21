#load common setting
include $(top_srcdir)/builder/builder_am.conf

lib_LTLIBRARIES = liblower_layer_director.la

liblower_layer_director_la_SOURCES = lower_layer_director.c builder_action.c director.c
#for statik library
liblower_layer_director_la_LDFLAGS=-shared -rpath $(libdir) 
liblower_layer_director_la_LDFLAGS+=-Wl,--version-script,builder.map
