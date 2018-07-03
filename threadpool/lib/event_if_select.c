#include "event_if.h"
#include "dp_util.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

//loop to add event
#define EVENT_SELECT_TIMEOUT (50000)

struct event_select_handler_t;
typedef struct event_select_handler_t event_select_handler_t, *EventSelectHandler;

struct event_select_handler_t {
	EventSelectHandler next;
	EventSelectHandler prev;
	event_subscriber_t subscriber;
	void *arg;
};

typedef struct event_select_fds_t {
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
} event_select_fds_t;

static inline void event_select_reset_fds(event_select_fds_t * fds) {
	FD_ZERO(&fds->readfds);
	FD_ZERO(&fds->writefds);
	FD_ZERO(&fds->exceptfds);
}

struct event_select_t {
	/*this order is from lower fd number*/
	EventSelectHandler head;
	EventSelectHandler tail;
	event_select_fds_t storefds;
	event_select_fds_t waitfds;
	int maxfd;
	struct timeval timeout;
	struct timeval use_timeout;
	int is_stop;
};
typedef struct event_select_t event_select_t, *EventSelect;

#define event_select_handler_pull(this, data) dputil_list_pull((DPUtilList)(this), (DPUtilListData)(data))
#define event_select_handler_insert(this, prev, data)  dputil_list_insert((DPUtilList)(this),(DPUtilListData)(prev), (DPUtilListData)(data))

static inline void event_select_set_fds(EventSelect this , EventSubscriber subscriber) {
	if(subscriber->eventflag&EV_TPOOL_READ  ) FD_SET(subscriber->fd, &this->storefds.readfds);
	if(subscriber->eventflag&EV_TPOOL_WRITE ) FD_SET(subscriber->fd, &this->storefds.writefds);
	if(subscriber->eventflag&EV_TPOOL_HUNGUP) FD_SET(subscriber->fd, &this->storefds.exceptfds);
}

static inline void event_select_unset_fds(EventSelect this , EventSubscriber subscriber) {
	if(subscriber->eventflag&EV_TPOOL_READ  ) FD_CLR(subscriber->fd, &this->storefds.readfds);
	if(subscriber->eventflag&EV_TPOOL_WRITE ) FD_CLR(subscriber->fd, &this->storefds.writefds);
	if(subscriber->eventflag&EV_TPOOL_HUNGUP) FD_CLR(subscriber->fd, &this->storefds.exceptfds);
}

static inline int event_select_get_eventflag_from_fds(EventSelect this, int fd) {
	int eventflag=0;
	if(FD_ISSET(fd, &(this->waitfds.readfds))  ) eventflag |= EV_TPOOL_READ;
	if(FD_ISSET(fd, &(this->waitfds.writefds)) ) eventflag |= EV_TPOOL_WRITE;
	if(FD_ISSET(fd, &(this->waitfds.exceptfds))) eventflag |= EV_TPOOL_HUNGUP;
	return eventflag;
}

/*! @name API for event if */
/*@{*/
/** event new */
EventInstance event_if_new(void) {
	//create main instance
	EventSelect instance = calloc(1,sizeof(*instance));
	if(!instance) return NULL;

	//set member value
	instance->timeout.tv_usec=EVENT_SELECT_TIMEOUT;
	event_select_reset_fds(&instance->storefds);
	event_select_reset_fds(&instance->waitfds);

	return instance;
err:
	event_if_free(instance);
	return NULL;
}

/** add new event */
EventHandler event_if_add(EventInstance this, EventSubscriber subscriber, void *arg) {

	EventSelect base = (EventSelect)this;

	EventSelectHandler instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("#######calloc error!\n");
		return NULL;
	}

	//use subscriber for handler to get fd and delete event
	memcpy(&instance->subscriber, subscriber, sizeof(instance->subscriber));
	instance->arg = arg;

	DEBUG_ERRPRINT("event_add [%d]\n" , instance->subscriber.fd);
	EventSelectHandler prev=NULL;
	EventSelectHandler current = base->head;
	while(current) {
		/*list is sorted, so inset place == before over fd num place*/
		if(instance->subscriber.fd < current->subscriber.fd) {
			break;
		}
		prev = current;
		current = current->next;
	}
	event_select_handler_insert(base, prev, instance);

	/*change max fd*/
	if(base->tail == instance) {
		base->maxfd = instance->subscriber.fd;
	}

	DEBUG_ERRPRINT("maxfd [%d]\n" , base->maxfd);
	/*add set flag*/
	event_select_set_fds(base, &instance->subscriber);	
	return instance;
}

/** update registered event */
void * event_if_update(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg) {
	EventSelect base = (EventSelect)this;
	EventSelectHandler instance = (EventSelectHandler) handler;

	DEBUG_ERRPRINT("event_update [%d]\n" , instance->subscriber.fd);
	/*is different event?*/
	if(instance->subscriber.eventflag != subscriber->eventflag) {
		/*unset old, and set new*/
		event_select_unset_fds(base, &instance->subscriber);
		event_select_set_fds(base, subscriber);
	}

	/*copy subscribe*/
	memcpy(&instance->subscriber, subscriber, sizeof(instance->subscriber));
	instance->arg = arg;

	return instance;
}

/** delete event */
void event_if_del(EventInstance this, EventHandler handler) {
	EventSelect base = (EventSelect)this;
	EventSelectHandler instance = (EventSelectHandler) handler;

	DEBUG_ERRPRINT("event_del [%d]\n" , instance->subscriber.fd);
	/*change maxfd*/
	if(base->tail == instance) {
		if(base->tail->prev) {
			base->maxfd = base->tail->prev->subscriber.fd;
		} else {
			base->maxfd = 0;
		}
	}

	/*clear fd event*/
	event_select_unset_fds(base, &instance->subscriber);

	/*pull and free data*/
	event_select_handler_pull(base, instance);
	free(instance);
}

int event_if_getfd(EventHandler handler) {
	return ((EventSelectHandler)handler)->subscriber.fd;
}

/** main loop of this event */
int event_if_loop(EventInstance this) {
	EventSelect base = (EventSelect)this;
	int i=0, ret=0;
	short eventflag;
	EventSelectHandler handler;
	EventSubscriber subscriber;
	fd_set readfds, writefds, exceptfds;
	base->is_stop=0;
	while(!base->is_stop) {
		/*reset fds*/
		memcpy(&base->waitfds, &base->storefds, sizeof(base->storefds));
		memcpy(&base->use_timeout, &base->timeout, sizeof(base->use_timeout));

//		DEBUG_ERRPRINT("Wait!\n" );
		ret = select(base->maxfd + 1, &base->waitfds.readfds, &base->waitfds.writefds, &base->waitfds.exceptfds, &base->use_timeout);
		if(ret<0) {
			DEBUG_ERRPRINT("Exit loop! errno=%d\n", errno );
			break;
		}
		/*timeout*/
		if(ret==0) {
//			DEBUG_ERRPRINT("Timeout!\n" );
			continue;
		}

		handler = base->tail;
		while(handler) {
			subscriber = &handler->subscriber;
			//DEBUG_ERRPRINT("check event [%d]\n" , subscriber->fd);
			eventflag = event_select_get_eventflag_from_fds(base, subscriber->fd);
			if(eventflag & subscriber->eventflag) {
			//	DEBUG_ERRPRINT("call handler of [%d]\n", subscriber->fd);
				subscriber->event_callback(subscriber->fd, eventflag, handler->arg);
			//	DEBUG_ERRPRINT("call handler of [%d] end\n", subscriber->fd);
			}
			handler=handler->prev;
		}
	}
	DEBUG_ERRPRINT("###exit main loop\n");
	return ret;
}

/** break event */
void event_if_loopbreak(EventInstance this) {
	EventSelect base = (EventSelect)this;
	base->is_stop=1;
}

/** exit after main loop */
void event_if_exit(EventInstance this) {
	return;
}

/** free event if instance */
void event_if_free(EventInstance this) {
	EventSelect instance = (EventSelect)this;
	if(!instance) return;
	free(instance);
}
/*@}*/
