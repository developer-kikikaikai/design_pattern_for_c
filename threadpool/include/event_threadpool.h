/**
 * @file event_threadpool.h
 * This is API as ThreadPool design petten by using libevent
**/
#ifndef EVENT_THREADPOOL_H_
#define EVENT_THREADPOOL_H_
#include "event_threadpool_data.h"
/*! @name API for EventTPoolManager instance */
/*@{*/
/**
 * constructor of EventTPoolManager
 * @param[in] thread_num size of thread. If this is negative value, this library set thread CPU number * 2
 * @param[in] is_threadsafe if you use this instance in multi thread, please set 1.
 * @note when call it, start thread which has event_base struct. Those threads pool event fd, and call event function.
 */
EventTPoolManager event_tpool_manager_new(int thread_num, int is_threadsafe);
/**
 * destructor of EventTPoolManager
 * @param[in] this EventTPoolManager instance returned at event_tpool_new.
 * @note when call it, stop all threads.
 */
void event_tpool_manager_free(EventTPoolManager this);
/**
 * get size of thread
 * @param[in] this EventTPoolManager instance returned at event_tpool_new.
 * @retval size of thread
 * @retval -1 error
 */
size_t event_tpool_manager_get_threadnum(EventTPoolManager this);
/*@}*/

/*! @name API for EventThreadPool */
/*@{*/
/**
 * add EventSubscriber to threadpool
 * @param[in] this EventTPoolManager instance returned at event_tpool_new.
 * @param[in] subscriber EventSubscriber
 * @param[in] arg argument for event_callback
 * @retval thread id (0-thread_num-1) which subscriber added
 * @retval <0 error (no resource, or if a same fd's subscriber was already registred)
 * @note if there already exist same fd's subscriber, it will override
 */
int event_tpool_add(EventTPoolManager this, EventSubscriber subscriber, void * arg);
/**
 *  add EventSubscriber to threadpool, if you want to choose thead, please use it.
 * @param[in] this EventTPoolManager instance returned at event_tpool_new.
 * @param[in] threadid thread id (0-thread_num-1)
 * @param[in] subscriber EventSubscriber
 * @param[in] arg argument for event_callback
 * @retval thread number (0-thread_num-1) which subscriber added
 * @retval <0 error (no resource, or if a same fd's subscriber was already registred)
 * @note if there already exist same fd's subscriber, it will override
 * @note this API doesn't remove other thread's same fd setting.
 */
int event_tpool_add_thread(EventTPoolManager this, int threadid, EventSubscriber subscriber, void * arg);
/**
 * delete EventSubscriber to threadapool.
 * @param[in] this EventTPoolManager instance returned at event_tpool_new.
 * @param[in] fd removed fd (related to subscriber)
 * @return none
 * @note this API doesn't close fd, please close ownself.
 */
void event_tpool_del(EventTPoolManager this, int fd);
/*@}*/
#endif
