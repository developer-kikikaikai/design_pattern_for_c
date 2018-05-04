#ifndef DEBUG_
#define DEBUG_

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
        fprintf(stderr, "%s(%d): "fmt"%s", __FUNCTION__,__LINE__, __VA_ARGS__)
#else
#define DEBUG_ERRPRINT(...) 
#endif

//#define DBGENER
#ifdef DBGENER
#define ENTERLOG printf("<%s>enter\n", __FUNCTION__);
#define EXITLOG  printf("<%s>exit\n", __FUNCTION__);
#else
#define ENTERLOG
#define EXITLOG
#endif

#endif
