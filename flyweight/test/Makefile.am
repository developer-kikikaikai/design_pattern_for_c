#load common setting
include $(top_srcdir)/flyweight/flyweight_am.conf

bin_PROGRAMS = test

test_CXXFLAGS = -g -O0
test_CFLAGS = -g -O0
test_SOURCES = main.c
test_LDFLAGS = $(top_srcdir)/flyweight/lib/.libs/libflyweight.a -lpthread
