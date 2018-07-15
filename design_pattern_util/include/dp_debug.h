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
/**
 * @brief This is API to store log with timestamp, to measure speed spec.
**/
struct dp_timelog_t;
typedef struct dp_timelog_t dp_timelog_t, *DPTimeLog;

/**
 * @brief Init store log
 *
 * @param[in] delimiter delimiter string if you want, default:" "
 * @param[in] maxloglen  Max length of logs
 * @param[in] maxstoresize  Storaged log size. If 0, always show log
 * @param[in] is_threadsafe threadsafe or not
 * @retval !NULL handle pointer to use other method
 * @retval NULL error
 */
DPTimeLog dp_timelog_init(const char *delimiter, size_t maxloglen, unsigned long maxstoresize, int is_threadsafe);

/**
 * @brief Store log
 *
 * @param[in] handle handle returned from timelog_init
 * @param[in] format log format as printf
 * @retval 0<=val success
 * @retval other failed (same as prinf)
 */
int dp_timelog_print(DPTimeLog handle, const char *format, ...); 

/**
 * @brief Exit stored log, and show stored log
 *
 * @param[in] handle handle returned from timelog_init
 * @return none
 */
void dp_timelog_exit(DPTimeLog handle);

/*************
 * define debug macro
*************/
#define DBGFLAG
#ifdef DBGFLAG
#include <errno.h>
#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#include <pthread.h>
extern DPTimeLog timelog_g;
#define DPDEBUG_INIT           timelog_g=dp_timelog_init(",", 2048, 8192, 0); dp_timelog_print(timelog_g,"init\n");
#define DPDEBUG_INIT_THREADSAFE timelog_g=dp_timelog_init(",", 2048, 8192, 1); dp_timelog_print(timelog_g,"init\n");
#define DEBUG_ERRPRINT_(fmt, ...)  \
        dp_timelog_print(timelog_g, "[%s(%s:%d)thread:%x]: "fmt"%s", __FUNCTION__,__FILE__,__LINE__,(unsigned int)pthread_self(), __VA_ARGS__)
#define DPDEBUG_EXIT           dp_timelog_print(timelog_g,"exit\n");dp_timelog_exit(timelog_g);timelog_g=NULL;
#else
#define DPDEBUG_INIT
#define DPDEBUG_INIT_THREDSAFE
#define DEBUG_ERRPRINT(...) 
#define DPDEBUG_EXIT
#endif
#endif
