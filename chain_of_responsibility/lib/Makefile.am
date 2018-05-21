#load common setting
include $(top_srcdir)/chain_of_responsibility/CoR_am.conf

lib_LTLIBRARIES = libchainofresp.la

libchainofresp_la_SOURCES = chain_element.c chain_of_responsibility.c
#for statik library
libchainofresp_la_LDFLAGS=-shared $(top_srcdir)/design_pattern_util/lib/libdputil.la $(top_srcdir)/flyweight/lib/libflyweight.la -rpath $(libdir)
libchainofresp_la_LDFLAGS+=-Wl,--version-script,chain_of_responsibility.map
