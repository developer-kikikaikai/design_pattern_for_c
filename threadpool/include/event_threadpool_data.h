/**
 * @file event_threadpool_data.h
 * This is API as ThreadPool data definition for ThreadPooldesign petten by using libevent
**/
#ifndef EVENT_THREADPOOL_DATA_H_
#define EVENT_THREADPOOL_DATA_H_
#include "dp_define.h"
DP_H_BEGIN

/*! @name Used class name definition.*/
/*@{*/
/*! @struct event_tpool_manager_t
 * EventTPoolManager class instance definition, member is defined in event_tpool_manager.c.
*/
struct event_tpool_manager_t;
/** EventTPoolManager class definition  */
typedef struct event_tpool_manager_t * EventTPoolManager;


#define EV_TPOOL_READ (0x01<<0)
#define EV_TPOOL_WRITE (0x01<<1)
#define EV_TPOOL_HUNGUP (0x01<<2)

/*! @struct event_subscriber_t
 * EventSubscriber class instance definition, this is storaged in any threads.
*/
struct event_subscriber_t {
	int fd;/*!< file descripter of this subscriber */
	int eventflag;/**< OR value of  EV_TPOOL_XXX definition*/
	void (*event_callback)(int socketfd, int eventflag, void * event_arg);
};

/** EventSubscriber class definition  */
typedef struct event_subscriber_t event_subscriber_t,  * EventSubscriber;

/*! @struct event_tpool_fd_data_t
 * added event handler class instance definition
 */
struct event_tpool_thread_info_t;
typedef struct event_tpool_thread_info_t * EventTPoolThreadInfo;

/** add result definition */
struct event_tpool_add_result_t {
	int result;
	EventTPoolThreadInfo event_handle;
};

typedef struct event_tpool_add_result_t event_tpool_add_result_t;

DP_H_END
/*@}*/
#endif
