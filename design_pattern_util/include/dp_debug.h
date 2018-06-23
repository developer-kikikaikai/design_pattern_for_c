/**
 * @file dp_debug.h
 * @brief For using debug log
**/
#ifndef DP_UTIL_DEBUG_H_
#define DP_UTIL_DEBUG_H_

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*************
 * define debug
*************/
//#define DBGFLAG
#ifdef DBGFLAG
#include <errno.h>
#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#define DEBUG_ERRPRINT_(fmt, ...)  \
        fprintf(stderr, "[%s(%s:%d)thread:%x]: "fmt"%s", __FUNCTION__,__FILE__,__LINE__,(unsigned int)pthread_self(), __VA_ARGS__)
#else
#define DEBUG_ERRPRINT(...) 
#endif

#endif
