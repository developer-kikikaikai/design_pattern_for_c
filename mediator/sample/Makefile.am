#load common setting
include $(top_srcdir)/am.conf

bin_PROGRAMS = test

test_CXXFLAGS = -g -O0
test_CFLAGS = -g -O0
test_SOURCES = main.c colleague_warrior.c  colleague_priest.c colleague_witch.c mediator.c 
test_LDADD= $(top_srcdir)/design_pattern_util/lib/libdputil.la
