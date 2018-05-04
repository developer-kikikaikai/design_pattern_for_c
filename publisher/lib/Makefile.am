#load common setting
include $(top_srcdir)/publisher/publisher_am.conf

noinst_LTLIBRARIES = libpublisher.la

libpublisher_la_SOURCES = publisher.c  publish_content.c
#for statik library
libpublisher_la_LDFLAGS=-module -shared -export-dynamic -static -rpath $(libdir)
libpublisher_la_LDFLAGS+=-Wl,--version-script,libpublisher.map
