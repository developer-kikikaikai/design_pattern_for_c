noinst_LTLIBRARIES = libfopen_proxy.la
libfopen_proxy_la_SOURCES = fopen_wrapper.c
libfopen_proxy_la_LDFLAGS = -rpath $(libdir) -shared

noinst_PROGRAMS = sample2

sample2_CXXFLAGS = -g -O0
sample2_CFLAGS = -g -O0
sample2_SOURCES = main.c
