#include "tpool_event_if.h"
#include "dp_debug.h"
#include <event2/event.h>
#include <unistd.h>

struct event_libevent_handler_t {
	event_subscriber_t subscriber;
	void *arg;
	struct event * event;
};

typedef struct event_libevent_handler_t event_libevent_handler_t, *EventLibeventHandler;

static inline short convert_etpoll_eveid2own(int eventflag) {
	short ret_eveflag=EV_PERSIST;
	if(eventflag&EV_TPOOL_READ) ret_eveflag |= EV_READ;
	if(eventflag&EV_TPOOL_WRITE) ret_eveflag |= EV_WRITE;
#ifdef EV_CLOSED
	if(eventflag&EV_TPOOL_HUNGUP) ret_eveflag |= EV_CLOSED;
#endif
	return ret_eveflag;
}

static inline int convert_etpoll_own2eveid(short eventflag) {
	int ret_eveflag=0;
	if(eventflag&EV_READ) ret_eveflag |= EV_TPOOL_READ;
	if(eventflag&EV_WRITE) ret_eveflag |= EV_TPOOL_WRITE;
#ifdef EV_CLOSED
	if(eventflag&EV_CLOSED) ret_eveflag |= EV_TPOOL_HUNGUP;
#endif
	return ret_eveflag;
}

static void event_if_libevent_callback(evutil_socket_t fd, short eventflag, void * arg) {
	EventLibeventHandler instance = (EventLibeventHandler)arg;
	int eflag = convert_etpoll_own2eveid(eventflag);
	instance->subscriber.event_callback(fd, eflag, instance->arg);
}

/*! @name API for event if */
/*@{*/
/** event new */
EventInstance event_if_new(void) {
	return event_base_new();
}

/** add new event */
EventHandler event_if_add(EventInstance this, EventSubscriber subscriber, void *arg) {

	struct event_base * base = (struct event_base *)this;

	/*add event*/
	EventLibeventHandler instance = (EventLibeventHandler)calloc(1, sizeof(*instance));
	if(!instance) return NULL;

	memcpy(&instance->subscriber, subscriber, sizeof(instance->subscriber));
	instance->arg = arg;
	instance->event = event_new(base, subscriber->fd, convert_etpoll_eveid2own(subscriber->eventflag), event_if_libevent_callback, instance);
	if(!instance->event) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	if(event_add(instance->event, NULL) == -1) {
		DEBUG_ERRPRINT("Failed to add event!\n" );
		goto err;
	}

	return instance;
err:
	if(instance->event) event_free(instance->event);
	free(instance);
	return NULL;
}

/** update registered event */
void * event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg) {
	DEBUG_ERRPRINT("Update!\n" );
	struct event_base * event_base = (struct event_base *)this;
	EventLibeventHandler eventinfo = (EventLibeventHandler)handler;

	/*delete and add to update*/
	event_if_del(event_base, eventinfo);
	eventinfo = event_if_add(this, subscriber, arg);

#if 0	
	/*This function support after 2.1.2-alpha*/
	/*restart loop and reload settings*/
	event_base_loopcontinue(event_base);
#endif
	return eventinfo;
}

/** delete event */
void event_if_del(EventInstance this, EventHandler handler) {
	EventLibeventHandler instance = (EventLibeventHandler)handler;
	event_del(instance->event);
	event_free(instance->event);
	free(instance);
}

int event_if_getfd(EventHandler handler) {
	EventLibeventHandler instance = (EventLibeventHandler)handler;
	return event_get_fd(instance->event);
}

/** main loop of this event */
int event_if_loop(EventInstance this) {
	event_base_dispatch((struct event_base *)this);
	return 0;
}

/** break event */
void event_if_loopbreak(EventInstance this) {
	event_base_loopbreak((struct event_base *)this);
}

/** exit after main loop */
void event_if_exit(EventInstance this) {
	struct event_base * base = (struct event_base *)this;
	if(event_base_got_break(base)) {
		event_base_loopbreak(base);
	}
}

/** free event if instance */
void event_if_free(EventInstance this) {
	event_base_free((struct event_base *)this);
}
/*@}*/
