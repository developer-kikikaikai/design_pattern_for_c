#load common setting
include $(top_srcdir)/chain_of_responsibility/CoR_am.conf

lib_LTLIBRARIES = libchainofresp.la

libchainofresp_la_SOURCES = chain_element.c chain_of_responsibility.c
#for statik library
libchainofresp_la_LDFLAGS=-shared $(DPUTIL_DIR)/lib/libdputil.la $(FLYWEIGHT_DIR)/lib/libflyweight.la -Xlinker -rpath -Xlinker $(libdir)
libchainofresp_la_LDFLAGS+=-Wl,--version-script,chain_of_responsibility.map
