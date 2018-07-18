/**
 * @file event_if.h
 * This is API definition for event wrapper plugin. Threadpool loads this plugin.
**/
#ifndef TPOOL_EVENT_IF_H_
#define TPOOL_EVENT_IF_H_

#include "event_threadpool_data.h"

/*! @name event instance definition*/
/*@{*/
/*! Event management instance which get from event_if_new. Definition is in plugin */
typedef void * EventInstance;
/*! Event handler related to fd. Definition is in plugin */
typedef void * EventHandler;
/*@}*/

/*! @name API for event if plugin interface*/
/*@{*/

/**
 * new event instance
 * @param[in] none
 * @retval !=NULL  this class handle
 * @retval NULL error
 * @note when call it, stop all threads.
 */
EventInstance event_if_new(void);
/**
 * add event handler related to fd
 * @param[in] this EventInstance instance returned at event_if_new.
 * @param[in] subscriber  EventSubscriber
 * @param[in] arg argument for event_callback
 * @retval !=NULL event handler instance
 * @retval NULL error
 */
EventHandler event_if_add(EventInstance this, EventSubscriber subscriber, void *arg);
/**
 * update event handler related to fd
 * @param[in] this EventInstance instance returned at event_if_new.
 * @param[in] handler EventHandler instance returned at event_if_add.
 * @param[in] subscriber  EventSubscriber
 * @param[in] arg argument for event_callback
 * @retval !=NULL new event handler instance
 * @retval NULL error
 */
EventHandler event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg);
/**
 * delete event handler related to fd
 * @param[in] this EventInstance instance returned at event_if_new.
 * @param[in] handler EventHandler instance returned at event_if_add.
 * @return none
 */
void event_if_del(EventInstance this, EventHandler handler);
/**
 * get fd related to handler
 * @param[in] handler EventHandler instance returned at event_if_add.
 * @return fd
 */
int event_if_getfd(EventHandler handler);
/**
 * main loop, start to watch event
 * @param[in] this EventInstance instance returned at event_if_new.
 * @retval 0 Normally stop
 * @retval !=0 Error stop
 * @note this API brock thread as select, etc
 */
int event_if_loop(EventInstance this);
/**
 * break main loop
 * @param[in] this EventInstance instance returned at event_if_new.
 * @return none
 */
void event_if_loopbreak(EventInstance this);
/**
 * exit instances for loop, if plugin want.
 * @param[in] this EventInstance instance returned at event_if_new.
 * @return none
 */
void event_if_exit(EventInstance this);
/**
 * free instances
 * @param[in] this EventInstance instance returned at event_if_new.
 * @return none
 */
void event_if_free(EventInstance this);
/*@}*/
#endif
