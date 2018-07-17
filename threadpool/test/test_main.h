#ifndef TEST_MAIN_H
#define TEST_MAIN_H
#include <stdio.h>
#include <pthread.h>
#if 0
#include "dp_debug.h"
#else
#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#define DEBUG_ERRPRINT_(fmt, ...)  \
        fprintf(stderr, "[%s(%s:%u)thread:%x]: "fmt"%s", __func__,__FILE__,__LINE__,(unsigned int)pthread_self(), __VA_ARGS__)
#endif
int test_main(const char * plugin);
#endif
