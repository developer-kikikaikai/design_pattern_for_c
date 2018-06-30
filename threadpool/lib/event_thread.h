/**
 * @file event_thread.h
 * This is API of event thread
**/
#ifndef EVENT_THREAD_H_
#define EVENT_THREAD_H_
#include "event_threadpool_data.h"
#include "pthread.h"

/*! @name msg definition */
/*@{*/
struct event_tpool_thread_t;
typedef struct event_tpool_thread_t event_tpool_thread_t,  * EventTPoolThread;
/*@}*/

/*! @name API for thread */
/*@{*/
/** create and thread instance */
EventTPoolThread event_tpool_thread_new(size_t thread_size);
/** start thread */
void event_tpool_thread_start(EventTPoolThread this);
/** stop thread, and remove resource*/
void event_tpool_thread_stop(EventTPoolThread this);
/** add new subscriber */
void event_tpool_thread_add(EventTPoolThread this, EventSubscriber subscriber, void * arg);
/** update subscriber */
void event_tpool_thread_update(EventTPoolThread this, EventSubscriber subscriber, void * arg);
/** delete subscriber */
void event_tpool_thread_del(EventTPoolThread this, int fd);
/*@}*/
#endif
