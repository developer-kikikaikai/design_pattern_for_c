/**
 * @file event_threadpool_data.h
 * This is API as ThreadPool data definition for ThreadPooldesign petten by using libevent
**/
#ifndef EVENT_THREADPOOL_DATA_H_
#define EVENT_THREADPOOL_DATA_H_
#include <event2/event.h>

/*! @name Used class name definition.*/
/*@{*/
/*! @struct event_tpool_manager_t
 * EventTPoolManager class instance definition, member is defined in event_tpool_manager.c.
*/
struct event_tpool_manager_t;
/** EventTPoolManager class definition  */
typedef struct event_tpool_manager_t * EventTPoolManager;

/*! @struct event_subscriber_t
 * EventSubscriber class instance definition, this is storaged in any threads.
*/
struct event_subscriber_t {
	int fd;/*!< file descripter of this subscriber */
	int eventflag;/**< event flag related to event.h 
                       *(or of EV_READ, EV_WRITE and EV_PERSIST.
                       *If you want to keep notification many times, please set EV_PERSIST.)
                       */
	event_callback_fn event_callback;/*!< defined in event.h, void (*event_callback_fn)(evutil_socket_t socketfd, short eventflag, void * event_arg);*/
};
/** EventSubscriber class definition  */
typedef struct event_subscriber_t event_subscriber_t,  * EventSubscriber;
/*@}*/
#endif