#load common setting
include $(top_srcdir)/prototype/prototype_am.conf

lib_LTLIBRARIES = libprototype.la

libprototype_la_SOURCES = prototype_factory.c prototype_manager.c
#for static library
libprototype_la_LDFLAGS=-shared $(DPUTIL_DIR)/lib/libdputil.la
if USE_GNU_VERSION_SCRIPT
libprototype_la_LDFLAGS+=-Wl,--version-script,prototype.map
endif
