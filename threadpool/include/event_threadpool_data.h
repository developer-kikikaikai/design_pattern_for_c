/**
 * @file event_threadpool_data.h
 * This is API as ThreadPool data definition for ThreadPooldesign petten by using libevent
**/
#ifndef EVENT_THREADPOOL_DATA_H_
#define EVENT_THREADPOOL_DATA_H_

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

/*! @struct event_subscriber_t
 * EventSubscriber class instance definition, this is storaged in any threads.
*/
struct event_subscriber_t {
	int fd;/*!< file descripter of this subscriber */
	int eventflag;/**< event flag related to event.h 
                       *(or of EV_READ, EV_WRITE and EV_PERSIST.
                       *If you want to keep notification many times, please set EV_PERSIST.)
                       */
	void (*event_callback)(int socketfd, short eventflag, void * event_arg);
};

/** EventSubscriber class definition  */
typedef struct event_subscriber_t event_subscriber_t,  * EventSubscriber;

/*! @struct event_tpool_fd_data_t
 * added event handler class instance definition
 */
struct event_tpool_fd_data_t;
typedef struct event_tpool_fd_data_t *EventTPoolFDData;

/** add result definition */
struct event_tpool_add_result_t {
	int result;
	EventTPoolFDData event_handle;
};

typedef struct event_tpool_add_result_t event_tpool_add_result_t;

/*@}*/
#endif
