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
#include <fcntl.h>
#include "event_if.h"
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
	void * eventinfo;/*!< event related to subscriber */
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
	EventInstance event_base;/*<! base event*/
	EventHandler msg_evfifo;/*<! msg event info*/
	pthread_t tid;/*<! thread id*/
	int is_stop;/*<! stop flag*/
};
/*@}*/

/*! @name API for message.*/
/*@{*/
#define wait_response(this) {int tmp=0;int ret = read(this->sockpair[EVE_THREAD_SOCK_FROM_MNG], &tmp, sizeof(tmp));}
#define send_response(this) {int tmp=0;int ret = write(this->sockpair[EVE_THREAD_SOCK_FOR_MINE], &tmp, sizeof(tmp));}
#define event_thread_msg_send(this, msg) write(this->sockpair[EVE_THREAD_SOCK_FROM_MNG], msg, sizeof(event_thread_msg_t));
static inline int event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type);
static inline int event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static inline int event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static inline int event_thread_msg_send_del(EventTPoolThread this, int fd);
static inline int event_thread_msg_send_stop(EventTPoolThread this);
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
static event_tpool_thread_msg_cb event_tpool_thread_msg_cb_table[]={
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
static inline int event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type) {
	event_thread_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = type;
	memcpy(&msg.body.add.subscriber, subscriber, sizeof(*subscriber));
	msg.body.add.arg = arg;
	int ret = 0;
	if(pthread_self() != this->tid) {
		DEBUG_ERRPRINT("other thread!\n" );
		ret = event_thread_msg_send(this, &msg);
	} else {
		DEBUG_ERRPRINT("own thrad!\n" );
		event_tpool_thread_msg_cb_table[msg.type](this, &msg);
	}
	return ret;
}

static inline int event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	return event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_ADD);
}

static inline int event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	return event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_UPDATE);
}
static inline int event_thread_msg_send_del(EventTPoolThread this, int fd) {
	event_thread_msg_t msg={.type=EVE_THREAD_MSG_TYPE_DEL, .body.del.fd = fd};
	int ret = 0;
	if(pthread_self() != this->tid) {
		ret = event_thread_msg_send(this, &msg);
	} else {
		event_tpool_thread_msg_cb_table[msg.type](this, &msg);
	}
	return ret;
}
static inline int event_thread_msg_send_stop(EventTPoolThread this) {
	event_thread_msg_t msg={.type=EVE_THREAD_MSG_TYPE_STOP};
	int ret = 0;
	if (pthread_self() != this->tid) {
		ret = event_thread_msg_send(this, &msg);
	} else {
		event_tpool_thread_msg_cb_table[msg.type](this, &msg);
	}
	return ret;
}

/*@}*/
/*! new instance */
static EventSubscriberData event_subscriber_data_new(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	EventSubscriberData instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->eventinfo = event_if_add(this->event_base, subscriber, arg);
	if(!instance->eventinfo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
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
	return (int)event_if_getfd(this->eventinfo);
}
static inline void event_tpool_thread_wait_stop(EventTPoolThread this) {
	pthread_t tid = 0;
	tid = this->tid;
	//to free this, keep tid
	pthread_join(tid, NULL);
}
/*************
 * for EventTPoolThread private API
*************/
/*! @name private API for EventTPoolThread.*/
/*@{*/
/*! set event_base */
static int event_tpool_thread_set_event_base(EventTPoolThread this) {
	this->event_base = event_if_new();
	if(!this->event_base) {
		DEBUG_ERRPRINT("Failed to new event_base!\n" );
		goto err;
	}

	/*add event*/
	event_subscriber_t subscriber={this->sockpair[EVE_THREAD_SOCK_FOR_MINE], EV_TPOOL_READ, event_tpool_thread_cb};
	this->msg_evfifo = event_if_add(this->event_base, &subscriber, this);
	if(!this->msg_evfifo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	return 0;
err:
	event_tpool_thread_remove_event_base(this);
	return -1;
}

static void event_tpool_thread_remove_event_base(EventTPoolThread this) {
	if(this->msg_evfifo) {
		event_if_del(this->event_base, this->msg_evfifo);
	}
	if(this->event_base) {
		event_if_free(this->event_base);
	}
}

/** free thread instance, please call stop before calling it*/
static void event_tpool_thread_free(EventTPoolThread this) {
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
		event_if_loop(this->event_base);
	}

	event_if_exit(this->event_base);

	event_tpool_thread_free(this);
	if(pthread_detach(pthread_self())) {
		//already wait join, call exit
		pthread_exit(NULL);
	}
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
	subscriber->eventinfo = event_if_update(this->event_base, subscriber->eventinfo, &msg->body.update.subscriber, msg->body.update.arg);
	send_response(this);
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
	event_if_loopbreak(this->event_base);
}
/*! callback main*/
static void event_tpool_thread_cb(evutil_socket_t fd, short flag, void * arg) {
	EventTPoolThread this = (EventTPoolThread)arg;
	event_thread_msg_t msg;
	int ret = read(fd, &msg, sizeof(msg));
	DEBUG_ERRPRINT("own thrad!\n" );
	event_tpool_thread_msg_cb_table[msg.type](this, &msg);
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

/** start thread */
void event_tpool_thread_start(EventTPoolThread this) {
	this->tid=0;
	pthread_create(&this->tid, NULL, event_tpool_thread_main, this);
}

/** stop thread */
void event_tpool_thread_stop(EventTPoolThread this) {
	int ret = event_thread_msg_send_stop(this);
	if(0<ret) {
		event_tpool_thread_wait_stop(this);
	}
}

/** add new subscriber */
void event_tpool_thread_add(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	int ret = event_thread_msg_send_add(this, subscriber, arg);
	if(0<ret) {
		wait_response(this);
	}
}
void event_tpool_thread_update(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	int ret = event_thread_msg_send_update(this, subscriber, arg);
	if(0<ret) {
		DEBUG_ERRPRINT("wait!\n" );
		wait_response(this);
		DEBUG_ERRPRINT("wait end!\n" );
	}
}

/** delete subscriber */
void event_tpool_thread_del(EventTPoolThread this, int fd) {
	int ret = event_thread_msg_send_del(this, fd);
	if(0<ret) {
		wait_response(this);
	}
}
