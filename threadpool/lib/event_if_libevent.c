#include "event_if.h"
#include "dp_debug.h"
#include <event2/event.h>
#include <unistd.h>

static inline int convert_etpoll_eveid2own(int eventflag) {
	int ret_eveflag=EV_PERSIST;
	if(eventflag&EV_TPOOL_READ) ret_eveflag |= EV_READ;
	if(eventflag&EV_TPOOL_WRITE) ret_eveflag |= EV_WRITE;
	if(eventflag&EV_TPOOL_HUNGUP) ret_eveflag |= EV_SIGNAL;
	return ret_eveflag;
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
	struct event * instance = event_new(base, subscriber->fd, convert_etpoll_eveid2own(subscriber->eventflag), subscriber->event_callback, arg);
	if(!instance) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	if(event_add(instance, NULL) == -1) {
		DEBUG_ERRPRINT("Failed to add event!\n" );
		goto err;
	}

	return instance;
err:
	event_del(instance);
	return NULL;
}

/** update registered event */
void * event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg) {
	DEBUG_ERRPRINT("Update!\n" );
	struct event_base * event_base = (struct event_base *)this;
	struct event * eventinfo = (struct event *)handler;

	event_del(eventinfo);
	/*restart loop and reload settings*/
	eventinfo = event_if_add(event_base, subscriber, arg);
	event_base_loopcontinue(event_base);

	return eventinfo;
}

/** delete event */
void event_if_del(EventInstance this, EventHandler handler) {
	struct event * instance = (struct event *) handler;
	event_del(instance);
	event_free(instance);
}

int event_if_getfd(EventHandler handler) {
	return event_get_fd((struct event *)handler);
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
