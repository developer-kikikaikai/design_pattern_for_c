/**
 * @file event_thread.c
 * This is API implement for EventThread class
**/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include "event_thread.h"
#include "dp_util.h"

/*************
 * public define
*************/
/*! @name event_tpool_thread_t definition.*/
/*@{*/
/*! message definition for manage subscriber*/
typedef struct event_thread_msg_body_add_t {
	event_subscriber_t subscriber;
	void * arg;
} event_thread_msg_body_add_t;

typedef struct event_thread_msg_body_del_t {
	int fd;
} event_thread_msg_body_del_t;

typedef struct event_thread_msg_header_t {
	enum {
		EVE_THREAD_MSG_TYPE_ADD,
		EVE_THREAD_MSG_TYPE_UPDATE,
		EVE_THREAD_MSG_TYPE_DEL,
		EVE_THREAD_MSG_TYPE_STOP,
	} type;
	union {
		event_thread_msg_body_add_t add;/*!< when msg is add, body is subscribe*/
		event_thread_msg_body_add_t update;/*!< when msg is update, body is subscribe*/
		event_thread_msg_body_del_t del;/*!< when msg is del, body is fd */
		//null
	} body;
} event_thread_msg_t;

/*! subscriber information define */
typedef struct event_subscriber_data_t *EventSubscriberData;
/*! subscriber information define */
struct event_subscriber_data_t {
	EventSubscriberData next;
	EventSubscriberData prev;
	struct event *eventinfo;/*!< event related to subscriber */
};

#define event_thread_pop(this)  (EventSubscriberData)dputil_list_pop((DPUtilList)(this))
#define event_thread_pull(this, data) dputil_list_pull((DPUtilList)(this), (DPUtilListData)(data))
#define event_thread_push(this, data)  dputil_list_push((DPUtilList)(this), (DPUtilListData)(data))

/*! socket index from manager */
#define EVE_THREAD_SOCK_FROM_MNG (0)
/*! socket index for mine(to recv) */
#define EVE_THREAD_SOCK_FOR_MINE (1)
/*! thread information */
struct event_tpool_thread_t {
	EventSubscriberData head;/*!<list of subscriber*/
	EventSubscriberData tail;/*!<list of subscriber*/
	int sockpair[2];/*!<socketpair to use add message*/
	struct event_base* event_base;/*<! base event */
	struct event *msg_evfifo;/*<! msg event info*/
	pthread_t tid;/*<! thread id*/
	int is_stop;/*<! stop flag*/
};
/*@}*/

/*! @name API for message.*/
/*@{*/
#define wait_response(this) {int tmp;int ret = read(this->sockpair[EVE_THREAD_SOCK_FROM_MNG], &tmp, sizeof(tmp));}
#define send_response(this) {int tmp;int ret = write(this->sockpair[EVE_THREAD_SOCK_FOR_MINE], &tmp, sizeof(tmp));}
#define event_thread_msg_send(this, msg) {int ret = write(this->sockpair[EVE_THREAD_SOCK_FROM_MNG], msg, sizeof(event_thread_msg_t));}
static inline void event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type);
static inline void event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static inline void event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static inline void event_thread_msg_send_del(EventTPoolThread this, int fd);
static inline void event_thread_msg_send_stop(EventTPoolThread this);
/*@}*/
/*! @name API for EventSubscriberData.*/
/*@{*/
/*! new instance */
static EventSubscriberData event_subscriber_data_new(EventTPoolThread this, EventSubscriber subscriber, void *arg);
/*! free instance */
static void event_subscriber_data_free(EventSubscriberData this);
/*! get fd */
static inline int event_subscriber_data_get_fd(EventSubscriberData this);
/*@}*/

/*! @name private API for EventTPoolThread.*/
/*@{*/
/*! set event_base */
static int event_tpool_thread_set_event_base(EventTPoolThread this);
/*! remove event_base */
static void event_tpool_thread_remove_event_base(EventTPoolThread this);
/*! get subscriber */
static EventSubscriberData event_tpool_thread_get_subscriber(EventTPoolThread this, int fd);
/*! wait stop */
static inline void event_tpool_thread_wait_stop(EventTPoolThread this);
/*! main thread */
static void * event_tpool_thread_main(void *arg);
/*@}*/
/*! @name API for EventTPoolThread msg callback*/
/*@{*/
typedef void (*event_tpool_thread_msg_cb)(EventTPoolThread this, event_thread_msg_t *msg);
/*! for add*/
static void event_tpool_thread_msg_cb_add(EventTPoolThread this, event_thread_msg_t *msg);
/*! for update*/
static void event_tpool_thread_msg_cb_update(EventTPoolThread this, event_thread_msg_t *msg);
/*! for del*/
static void event_tpool_thread_msg_cb_del(EventTPoolThread this, event_thread_msg_t *msg);
/*! for stop*/
static void event_tpool_thread_msg_cb_stop(EventTPoolThread this, event_thread_msg_t *msg);
/*! callback table */
static event_tpool_thread_msg_cb event_tpool_thread_msg_cb_tabe[]={
	event_tpool_thread_msg_cb_add,/*!< for EVE_THREAD_MSG_TYPE_ADD*/
	event_tpool_thread_msg_cb_update,/*!< for EVE_THREAD_MSG_TYPE_UPDATE*/
	event_tpool_thread_msg_cb_del,/*!< for EVE_THREAD_MSG_TYPE_DEL*/
	event_tpool_thread_msg_cb_stop,/*!< for EVE_THREAD_MSG_TYPE_STOP*/
};
/*! callback main*/
static void event_tpool_thread_cb(evutil_socket_t, short, void *);
/*@}*/

/*************
 * for EventSubscriberData.
*************/
static inline void event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type) {
	event_thread_msg_t msg;
	msg.type = type;
	memcpy(&msg.body.add.subscriber, subscriber, sizeof(*subscriber));
	msg.body.add.arg = arg;
	event_thread_msg_send(this, &msg);
}

static inline void event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_ADD);
}

static inline void event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_UPDATE);
}
static inline void event_thread_msg_send_del(EventTPoolThread this, int fd) {
	event_thread_msg_t msg={.type=EVE_THREAD_MSG_TYPE_DEL, .body.del.fd = fd};
	event_thread_msg_send(this, &msg);
}
static inline void event_thread_msg_send_stop(EventTPoolThread this) {
	event_thread_msg_t msg={.type=EVE_THREAD_MSG_TYPE_STOP};
	event_thread_msg_send(this, &msg);
}

/*@}*/
/*! new instance */
static EventSubscriberData event_subscriber_data_new(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	EventSubscriberData instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->eventinfo = event_new(this->event_base, subscriber->fd, subscriber->eventflag, subscriber->event_callback, arg);
	if(!instance->eventinfo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	if(event_add(instance->eventinfo, NULL) == -1) {
		DEBUG_ERRPRINT("Failed to add event!\n" );
		goto err;
	}

	return instance;

err:
	event_subscriber_data_free(instance);
	return NULL;
}

/*! free instance */
static void event_subscriber_data_free(EventSubscriberData this) {
	if(this->eventinfo) {
		event_del(this->eventinfo);
		event_free(this->eventinfo);
	}
	free(this);
}
/*! get fd */
static inline int event_subscriber_data_get_fd(EventSubscriberData this) {
	return (int)event_get_fd(this->eventinfo);
}
static inline void event_tpool_thread_wait_stop(EventTPoolThread this) {
	pthread_t tid = this->tid;
	//free all instance into thread side
	pthread_join(tid, NULL);
}
/*************
 * for EventTPoolThread private API
*************/
/*! @name private API for EventTPoolThread.*/
/*@{*/
/*! set event_base */
static int event_tpool_thread_set_event_base(EventTPoolThread this) {
	this->event_base = event_base_new();
	if(!this->event_base) {
		DEBUG_ERRPRINT("Failed to new event_base!\n" );
		goto err;
	}

	/*add event*/
	this->msg_evfifo = event_new(this->event_base, this->sockpair[EVE_THREAD_SOCK_FOR_MINE], EV_READ|EV_PERSIST, event_tpool_thread_cb, this);
	if(!this->msg_evfifo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	if(event_add(this->msg_evfifo, NULL) == -1) {
		DEBUG_ERRPRINT("Failed to add event!\n" );
		goto err;
	}

	return 0;
err:
	event_tpool_thread_remove_event_base(this);
	return -1;
}

static void event_tpool_thread_remove_event_base(EventTPoolThread this) {
	if(this->msg_evfifo) {
		event_del(this->msg_evfifo);
		event_free(this->msg_evfifo);
	}
	if(this->event_base) {
		event_base_free(this->event_base);
	}
}

static EventSubscriberData event_tpool_thread_get_subscriber(EventTPoolThread this, int fd) {
	EventSubscriberData subscriber = this->head;
	while(subscriber) {
		if(event_subscriber_data_get_fd(subscriber) == fd) {
			break;
		}
		subscriber=subscriber->next;
	}
	return subscriber;
}
/*! main thread */
static void * event_tpool_thread_main(void *arg) {
	EventTPoolThread this = (EventTPoolThread)arg;
	while(!this->is_stop) {
		event_base_dispatch(this->event_base);
	}
	event_base_got_exit(this->event_base);
	event_tpool_thread_free(this);
	pthread_exit(NULL);
	return NULL;
}
/*@}*/
/*! @name API for EventTPoolThread msg callback*/
/*@{*/
/*! for add*/
static void event_tpool_thread_msg_cb_add(EventTPoolThread this, event_thread_msg_t *msg) {
	/*add event*/
	EventSubscriberData instance = event_subscriber_data_new(this, &msg->body.add.subscriber, msg->body.add.arg);
	if(!instance) {
		DEBUG_ERRPRINT("Failed to new subscriber!\n" );
		return;
	}

	event_thread_push(this, instance);
	send_response(this);
}

/*! for update*/
static void event_tpool_thread_msg_cb_update(EventTPoolThread this, event_thread_msg_t *msg) {
	/*add event*/
	EventSubscriberData subscriber = event_tpool_thread_get_subscriber(this, msg->body.update.subscriber.fd);
	if(!subscriber) {
		DEBUG_ERRPRINT("Failed to find subscriber!\n" );
		return;
	}

	/*update event*/
	event_del(subscriber->eventinfo);
	subscriber->eventinfo = event_new(this->event_base, msg->body.update.subscriber.fd, msg->body.update.subscriber.eventflag, msg->body.update.subscriber.event_callback, msg->body.update.arg);
	event_add(subscriber->eventinfo, NULL);
	send_response(this);
	
	/*restart loop and reload settings*/
	event_base_loopcontinue(this->event_base);
}

/*! for del*/
static void event_tpool_thread_msg_cb_del(EventTPoolThread this, event_thread_msg_t *msg) {
	EventSubscriberData subscriber = event_tpool_thread_get_subscriber(this, msg->body.del.fd);
	if(!subscriber) {
		return;
	}
	//pull data from list
	event_thread_pull(this, subscriber);
	//delete event
	event_subscriber_data_free(subscriber);
	send_response(this);
}

/*! for stop*/
static void event_tpool_thread_msg_cb_stop(EventTPoolThread this, event_thread_msg_t *msg) {
	(void)msg;
	this->is_stop=1;
	event_base_loopbreak(this->event_base);
}
/*! callback main*/
static void event_tpool_thread_cb(evutil_socket_t fd, short flag, void * arg) {
	EventTPoolThread this = (EventTPoolThread)arg;
	event_thread_msg_t msg;
	int ret = read(fd, &msg, sizeof(msg));
	event_tpool_thread_msg_cb_tabe[msg.type](this, &msg);
}
/*@}*/
/*************
 * for public API
*************/
/** create and thread instance */
EventTPoolThread event_tpool_thread_new(void) {
	EventTPoolThread instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance!\n" );
	}

	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, instance->sockpair) == -1) {
		DEBUG_ERRPRINT("Failed to create revc socket pair!\n");
		goto err;
	}

	static int debugflag=0;
	if(!debugflag) {
		event_enable_debug_mode();
		debugflag++;
	}
	if(event_tpool_thread_set_event_base(instance)) {
		DEBUG_ERRPRINT("Failed to set base event!\n" );
		goto err;
	}

	return instance;
err:
	if(instance->sockpair[0] || instance->sockpair[1]) {
		close(instance->sockpair[0]);
		close(instance->sockpair[1]);
	}
	free(instance);
	return NULL;
}
/** free thread instance, please call stop before calling it*/
void event_tpool_thread_free(EventTPoolThread this) {
	EventSubscriberData data = event_thread_pop(this);
	while(data) {
		event_subscriber_data_free(data);	
		data = event_thread_pop(this);
	}
	event_tpool_thread_remove_event_base(this);
	close(this->sockpair[0]);
	close(this->sockpair[1]);
	free(this);
}

/** start thread */
void event_tpool_thread_start(EventTPoolThread this) {
	if(!this) {
		return;
	}

	pthread_create(&this->tid, NULL, event_tpool_thread_main, this);
}

/** stop thread */
void event_tpool_thread_stop(EventTPoolThread this) {
	event_thread_msg_send_stop(this);
	event_tpool_thread_wait_stop(this);
}

/** add new subscriber */
void event_tpool_thread_add(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	event_thread_msg_send_add(this, subscriber, arg);
	wait_response(this);
}
void event_tpool_thread_update(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	event_thread_msg_send_update(this, subscriber, arg);
	wait_response(this);
}

/** delete subscriber */
void event_tpool_thread_del(EventTPoolThread this, int fd) {
	event_thread_msg_send_del(this, fd);
	wait_response(this);
}
