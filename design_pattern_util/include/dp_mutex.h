#ifndef DPUTIL_MUTEX_
#define DPUTIL_MUTEX_
/**
 *    @brief  Utility mutex API for design pattern
**/
/*! @name Lock/Unlock API */
#include <pthread.h>
/* @{ */
/*! lock */
void dputil_lock(void *handle);
/*! unlock */
void dputil_unlock(void *handle);
/*! lock macro*/
#define DPUTIL_LOCK(lock) \
        dputil_lock(lock);\
        pthread_cleanup_push(dputil_unlock, lock);
#define DPUTIL_UNLOCK pthread_cleanup_pop(1);
/* @} */
#endif