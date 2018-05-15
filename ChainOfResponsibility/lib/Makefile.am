#load common setting
include $(top_srcdir)/ChainOfResponsibility/CoR_am.conf

noinst_LTLIBRARIES = libchainofresp.la
#

libchainofresp_la_SOURCES = chain_element.c chain_of_responsibility.c
#for statik library
libchainofresp_la_LDFLAGS=$(top_srcdir)/flyweight/lib/libflyweight.la -rpath $(libdir)
