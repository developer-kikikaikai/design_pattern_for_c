/**
 * @file event_if.h
 * This is API of event wrapper
**/
#ifndef TPOOL_EVENT_IF_H_
#define TPOOL_EVENT_IF_H_

#include "event_threadpool_data.h"

/*! @name event instance definition*/
/*@{*/
/*! Almost event already has new API, so it's better to define by void *... */
typedef void * EventInstance;
typedef void * EventHandler;
/*@}*/

/*! @name API for event if */
/*@{*/
/** event new */
EventInstance event_if_new(void);
/** add new event */
EventHandler event_if_add(EventInstance this, EventSubscriber subscriber, void *arg);
/** update registered event */
void * event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg);
/** delete event */
void event_if_del(EventInstance this, EventHandler handler);
/** delete event */
int event_if_getfd(EventHandler handler);
/** main loop of this event */
int event_if_loop(EventInstance this);
/** break event */
void event_if_loopbreak(EventInstance this);
/** exit after main loop */
void event_if_exit(EventInstance this);
/** free event if instance */
void event_if_free(EventInstance this);
/*@}*/
#endif
