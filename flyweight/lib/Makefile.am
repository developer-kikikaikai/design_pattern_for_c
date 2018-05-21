#load common setting
include $(top_srcdir)/flyweight/flyweight_am.conf

lib_LTLIBRARIES = libflyweight.la

libflyweight_la_SOURCES = flyweight.c
#for statik library
libflyweight_la_LDFLAGS=-shared $(top_srcdir)/design_pattern_util/lib/libdputil.la
libflyweight_la_LDFLAGS+=-Wl,--version-script,flyweight.map
