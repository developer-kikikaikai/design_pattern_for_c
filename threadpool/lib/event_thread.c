/**
 * @file event_thread.c
 * This is API implement for EventThread class
**/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "event_thread.h"
#include "tpool_event_if.h"
#include "dp_util.h"
#include "config.h"

#define EVENT_THREAD_WAIT_TIMEOUT (2)/*sec*/

#define EVENT_THREAD_STACKSIZE (256 * 1024)/*suitable stack size*/

/*************
 * public define
*************/
/*! @name event_tpool_thread_t definition.*/
/*@{*/
/** interface definition*/
static struct {
	void * handle;
	EventInstance (*new)(void);
	EventHandler (*add)(EventInstance this, EventSubscriber subscriber, void *arg);
	EventHandler (*update)(EventInstance this, EventHandler handler, EventSubscriber subscriber, void *arg);
	void (*del)(EventInstance this, EventHandler handler);
	int (*getfd)(EventHandler handler);
	int (*loop)(EventInstance this);
	void (*loopbreak)(EventInstance this);
	void (*exit)(EventInstance this);
	void (*free)(EventInstance this);
} event_if_instance_g;

static void event_thread_set_func(void *handle, const char *func_name, void ** func);
static int event_tpool_thread_load_all_fun(void);
static const char * event_tpool_thread_get_defaullt_plugin(void);

/*! thread stack size, user can change it by using event_tpool_set_stack_size*/
static size_t event_thread_stack_size_g = EVENT_THREAD_STACKSIZE;

/*! message definition for manage subscriber*/
typedef struct event_thread_msg_body_add_t {
	event_subscriber_t subscriber;
	void * arg;
} event_thread_msg_body_add_t;

typedef struct event_thread_msg_body_del_t {
	int fd;
} event_thread_msg_body_del_t;

/*! message struct, to push queue */
struct event_thread_msg_t;
typedef struct event_thread_msg_t event_thread_msg_t, *EventThreadMsg;

/*! message struct definition */
struct event_thread_msg_t {
	enum {
		EVE_THREAD_MSG_TYPE_ADD,
		EVE_THREAD_MSG_TYPE_UPDATE,
		EVE_THREAD_MSG_TYPE_DEL,
		EVE_THREAD_MSG_TYPE_STOP,
	} type;
	union {
		event_thread_msg_body_add_t add;/*!< when msg is update, body is subscribe*/
		event_thread_msg_body_del_t del;/*!< when msg is del, body is fd */
		
	} data;
};

typedef struct event_thread_msg_info_t {
	event_thread_msg_t msg;
	size_t store_msg_cnt;
	event_thread_msg_t *store_msgs;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
} event_thread_msg_info_t;

//#define CHECK_STACKSIZE
#ifdef CHECK_STACKSIZE
#define MAGIC_NUMBER 'Z'
#endif

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

/*! thread information */
struct event_tpool_thread_t {
	EventSubscriberData head;/*!<list of subscriber*/
	EventSubscriberData tail;/*!<list of subscriber*/

	event_thread_msg_info_t msgdata;/*<! message queue*/
	int eventfd;/*!<eventfd to use add message*/
	EventInstance event_base;/*<! base event*/
	EventHandler msg_evinfo;/*<! msg event info*/
	pthread_t tid;/*<! thread id*/
	int is_stop;/*<! stop flag*/
#ifdef CHECK_STACKSIZE
	char * stack_adr;
#endif
};
/*@}*/
/*! @name API for message.*/
/*@{*/
#define EVMSG_LOCK(this) DPUTIL_LOCK(&this->msgdata.lock);
#define EVMSG_UNLOCK DPUTIL_UNLOCK

static int event_thread_msg_send(EventTPoolThread this, EventThreadMsg msg);
static int event_thread_msg_send_without_lock(EventTPoolThread this, EventThreadMsg msg);
static void event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type);
static void event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static void event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg);
static void event_thread_msg_send_del(EventTPoolThread this, int fd);
static int event_thread_msg_send_stop(EventTPoolThread this);
/*@}*/
/*! @name API for EventSubscriberData.*/
/*@{*/
/*! new instance */
static EventSubscriberData event_subscriber_data_new(EventTPoolThread this, EventSubscriber subscriber, void *arg);
/*! free instance */
static void event_subscriber_data_free(EventTPoolThread this, EventSubscriberData data);
/*! get fd */
static int event_subscriber_data_get_fd(EventSubscriberData this);
/*@}*/

/*! @name private API for EventTPoolThread.*/
/*@{*/
/*! set event_base */
static int event_tpool_thread_set_event_base(EventTPoolThread this);
/*! remove event_base */
static void event_tpool_thread_remove_event_base(EventTPoolThread this);
/*! get subscriber */
static EventSubscriberData event_tpool_thread_get_subscriber(EventTPoolThread this, int fd);
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

static void event_tpool_thread_msg_cb_call(EventTPoolThread this, event_thread_msg_t *msg);

/*! main messages caller*/
static void event_tpool_thread_call_msgs(EventTPoolThread this, eventfd_t cnt);
/*! callback main*/
static void event_tpool_thread_cb(int, int, void *);
/*@}*/

/*************
 * for EventSubscriberData.
*************/
static int event_thread_msg_send(EventTPoolThread this, EventThreadMsg msg) {
	int ret = 0;
EVMSG_LOCK(this)
	ret = eventfd_write(this->eventfd, 1);
	if(ret < 0) {
		DEBUG_PRINT("################Failed to send event\n");
	} else {
		/*wait receive message notification from event thread main*/
		struct timespec timeout;
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_sec += EVENT_THREAD_WAIT_TIMEOUT;
		ret = pthread_cond_timedwait(&this->msgdata.cond, &this->msgdata.lock, &timeout);
		if(ret == ETIMEDOUT) {
			DEBUG_ERRPRINT("#####################timeout!!!!!!\n");
			ret = -1;
		}
	}
EVMSG_UNLOCK
	return ret;
}

static int event_thread_msg_send_without_lock(EventTPoolThread this, EventThreadMsg msg) {
	return eventfd_write(this->eventfd, 1);
}

static void event_thread_msg_send_subscribe(EventTPoolThread this, EventSubscriber subscriber, void *arg, int type) {
	int is_ownthread = ((this->tid==0) || (pthread_self() == this->tid));
	EventThreadMsg msg;
	if(!is_ownthread) {
		msg = &this->msgdata.store_msgs[this->msgdata.store_msg_cnt++];
	} else {
		msg = &this->msgdata.msg;
	}

	memset(msg, 0, sizeof(*msg));
	msg->type = type;
	memcpy(&msg->data.add.subscriber, subscriber, sizeof(*subscriber));
	msg->data.add.arg = arg;
	if(!is_ownthread) {
		int ret = event_thread_msg_send(this, msg);
		/*to care stop thread */
		if(ret < 0) this->msgdata.store_msg_cnt--;
	} else {
		event_tpool_thread_msg_cb_call(this, msg);
	}
}
static void event_thread_msg_send_add(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	DEBUG_PRINT("add, subscriber->%d!\n", subscriber->fd);

	event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_ADD);
}

static void event_thread_msg_send_update(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	DEBUG_PRINT("update, subscriber->%d!\n", subscriber->fd);
	event_thread_msg_send_subscribe(this, subscriber, arg, EVE_THREAD_MSG_TYPE_UPDATE);
}
static void event_thread_msg_send_del(EventTPoolThread this, int fd) {
	int is_ownthread = ((this->tid==0) || (pthread_self() == this->tid));
	EventThreadMsg msg;
	if(!is_ownthread) {
		msg = &this->msgdata.store_msgs[this->msgdata.store_msg_cnt++];
	} else {
		msg = &this->msgdata.msg;
	}

	memset(msg, 0, sizeof(*msg));
	msg->type=EVE_THREAD_MSG_TYPE_DEL;
	msg->data.del.fd = fd;
	DEBUG_PRINT("del, subscriber->%d!\n", fd);

	if(!is_ownthread) {
		int ret = event_thread_msg_send(this, msg);
		/*to care stop thread */
		if(ret < 0) this->msgdata.store_msg_cnt--;
	} else {
		event_tpool_thread_msg_cb_call(this, msg);
	}
}

static int event_thread_msg_send_stop(EventTPoolThread this) {
	int is_ownthread = ((this->tid==0) || (pthread_self() == this->tid));
	EventThreadMsg msg;
	if(!is_ownthread) {
		msg = &this->msgdata.store_msgs[this->msgdata.store_msg_cnt++];
	} else {
		msg = &this->msgdata.msg;
	}

	memset(msg, 0, sizeof(*msg));
	msg->type=EVE_THREAD_MSG_TYPE_STOP;
	int ret = 0;
	if (!is_ownthread) {
		ret = event_thread_msg_send_without_lock(this, msg);
		/*to care stop thread */
		if(ret < 0) this->msgdata.store_msg_cnt--;
		ret=1;
	} else {
		event_tpool_thread_msg_cb_call(this, msg);
	}
	DEBUG_PRINT("send stop, return %d!\n", ret);
	return ret;
}

static void event_tpool_thread_msg_cb_call(EventTPoolThread this, event_thread_msg_t *msg) {
	event_tpool_thread_msg_cb_table[msg->type](this, msg);
}

/*@}*/
/*! new instance */
static EventSubscriberData event_subscriber_data_new(EventTPoolThread this, EventSubscriber subscriber, void *arg) {
	DEBUG_PRINT("add subscriber->%d!\n", subscriber->fd);
	EventSubscriberData instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->eventinfo = event_if_instance_g.add(this->event_base, subscriber, arg);
	if(!instance->eventinfo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	return instance;

err:
	event_subscriber_data_free(this, instance);
	return NULL;
}

/*! free instance */
static void event_subscriber_data_free(EventTPoolThread this, EventSubscriberData data) {
	if(data->eventinfo) {
		event_if_instance_g.del(this->event_base, data->eventinfo);
	}
	free(data);
}
/*! get fd */
static int event_subscriber_data_get_fd(EventSubscriberData this) {
	return event_if_instance_g.getfd(this->eventinfo);
}
/*************
 * for EventTPoolThread private API
*************/
/*! @name private API for EventTPoolThread.*/
/*@{*/
/*! set event_base */
static int event_tpool_thread_set_event_base(EventTPoolThread this) {
	this->event_base = event_if_instance_g.new();
	if(!this->event_base) {
		DEBUG_ERRPRINT("Failed to new event_base!\n" );
		goto err;
	}

	/*add event*/
	event_subscriber_t subscriber={this->eventfd, EV_TPOOL_READ, event_tpool_thread_cb};
	DEBUG_PRINT("base fd=%d\n", this->eventfd);
	this->msg_evinfo = event_if_instance_g.add(this->event_base, &subscriber, this);
	if(!this->msg_evinfo) {
		DEBUG_ERRPRINT("Failed to new event!\n" );
		goto err;
	}

	return 0;
err:
	event_tpool_thread_remove_event_base(this);
	return -1;
}

static void event_tpool_thread_remove_event_base(EventTPoolThread this) {
	if(this->msg_evinfo) {
		event_if_instance_g.del(this->event_base, this->msg_evinfo);
	}
	if(this->event_base) {
		event_if_instance_g.free(this->event_base);
	}
}

/** free thread instance, please call stop before calling it*/
static void event_tpool_thread_free(EventTPoolThread this) {
	EventSubscriberData data = event_thread_pop(this);
	while(data) {
		event_subscriber_data_free(this, data);	
		data = event_thread_pop(this);
	}
	event_tpool_thread_remove_event_base(this);
	close(this->eventfd);
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
	/*add atfork handler*/
	while(!this->is_stop) {
		if(event_if_instance_g.loop(this->event_base) < 0) {
			break;
		}
	}

	DEBUG_PRINT("exit main thread!\n" );
	event_if_instance_g.exit(this->event_base);

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
	event_thread_msg_body_add_t * body = (event_thread_msg_body_add_t *)(&msg->data);

	EventSubscriberData instance = event_subscriber_data_new(this, &body->subscriber, body->arg);
	if(!instance) {
		DEBUG_ERRPRINT("Failed to new subscriber!\n" );
		return;
	}

	event_thread_push(this, instance);
}

/*! for update*/
static void event_tpool_thread_msg_cb_update(EventTPoolThread this, event_thread_msg_t *msg) {
	/*add event*/
	event_thread_msg_body_add_t * body = (event_thread_msg_body_add_t *)(&msg->data);
	EventSubscriberData subscriber = event_tpool_thread_get_subscriber(this, body->subscriber.fd);
	if(!subscriber) {
		DEBUG_ERRPRINT("Failed to find subscriber!\n" );
		return;
	}

	/*update event*/
	subscriber->eventinfo = event_if_instance_g.update(this->event_base, subscriber->eventinfo, &body->subscriber, body->arg);
}

/*! for del*/
static void event_tpool_thread_msg_cb_del(EventTPoolThread this, event_thread_msg_t *msg) {
	event_thread_msg_body_del_t * body = (event_thread_msg_body_del_t *)(&msg->data);
	EventSubscriberData subscriber = event_tpool_thread_get_subscriber(this, body->fd);
	if(!subscriber) {
		return;
	}
	//pull data from list
	event_thread_pull(this, subscriber);
	//delete event
	event_subscriber_data_free(this, subscriber);
}

/*! for stop*/
static void event_tpool_thread_msg_cb_stop(EventTPoolThread this, event_thread_msg_t *msg) {
	(void)msg;
	this->is_stop=1;
	event_if_instance_g.loopbreak(this->event_base);
}

static void event_tpool_thread_call_msgs(EventTPoolThread this, eventfd_t cnt) {
EVMSG_LOCK(this)
	/* call */
	while(this->msgdata.store_msg_cnt && cnt) {
		cnt--;
		event_tpool_thread_msg_cb_call(this, &this->msgdata.store_msgs[--this->msgdata.store_msg_cnt]);
		/*notify event message to called API thread*/
		DEBUG_PRINT("cond signal from event thread to %p\n", &this->msgdata.cond );
		pthread_cond_signal(&this->msgdata.cond);
	}
EVMSG_UNLOCK
}

/*! callback main*/
static void event_tpool_thread_cb(int fd, int flag, void * arg) {
	EventTPoolThread this = (EventTPoolThread)arg;

	eventfd_t cnt=0;
	int ret = eventfd_read(this->eventfd, &cnt);
	if(ret < 0) {
		DEBUG_ERRPRINT("Failed to read event!\n" );
	}

	event_tpool_thread_call_msgs(this, cnt);
}

static const char * event_tpool_thread_get_defaullt_plugin(void) {
	static const char * pluginlist[] = {
		EVENT_IF_PLUGIN_PATH"/libevent_if_libevent.so",
		EVENT_IF_PLUGIN_PATH"/libevent_if_epoll.so",
		EVENT_IF_PLUGIN_PATH"/libevent_if_select.so",
		NULL,
	};

	size_t i = 0;
	mode_t fmode;
	struct stat stat_buf;
	for(i = 0; pluginlist[i]; i ++ ) {
			//check is it library file?
		if((stat(pluginlist[i], &stat_buf)==0) && ( S_ISREG((stat_buf.st_mode & S_IFMT)) || S_ISLNK(stat_buf.st_mode & S_IFMT))) {
			break;
		}
	}
	return pluginlist[i];
}

static int event_tpool_thread_load_all_fun(void) {
	event_if_instance_g.new = dlsym(event_if_instance_g.handle, "event_if_new");
	if(!event_if_instance_g.new) return -1;

	event_if_instance_g.add = dlsym(event_if_instance_g.handle, "event_if_add");
	if(!event_if_instance_g.add) return -1;

	event_if_instance_g.update = dlsym(event_if_instance_g.handle, "event_if_update");
	if(!event_if_instance_g.update) return -1;

	event_if_instance_g.del = dlsym(event_if_instance_g.handle, "event_if_del");
	if(!event_if_instance_g.del) return -1;

	event_if_instance_g.getfd = dlsym(event_if_instance_g.handle, "event_if_getfd");
	if(!event_if_instance_g.getfd) return -1;

	event_if_instance_g.loop = dlsym(event_if_instance_g.handle, "event_if_loop");
	if(!event_if_instance_g.loop) return -1;

	event_if_instance_g.loopbreak = dlsym(event_if_instance_g.handle, "event_if_loopbreak");
	if(!event_if_instance_g.loopbreak) return -1;

	event_if_instance_g.exit = dlsym(event_if_instance_g.handle, "event_if_exit");
	if(!event_if_instance_g.exit) return -1;

	event_if_instance_g.free = dlsym(event_if_instance_g.handle, "event_if_free");
	if(!event_if_instance_g.free) return -1;

	return 0;
}

/*@}*/
/*************
 * for public API
*************/
/** create and thread instance */
EventTPoolThread event_tpool_thread_new(size_t thread_size) {
	EventTPoolThread instance = calloc(1, sizeof(*instance) + sizeof(event_thread_msg_t)*thread_size);
	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance!\n" );
	}

	instance->msgdata.store_msgs = (EventThreadMsg)(instance + 1);

	instance->eventfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC );
	if(instance->eventfd == -1) {
		DEBUG_ERRPRINT("Failed to create revc socket pair!\n");
		goto err;
	}

	pthread_mutex_init(&instance->msgdata.lock, NULL);
	pthread_cond_init(&instance->msgdata.cond, NULL);

	if(event_tpool_thread_set_event_base(instance)) {
		DEBUG_ERRPRINT("Failed to set base event!\n" );
		goto err;
	}

	return instance;
err:
	if(instance->eventfd) {
		close(instance->eventfd);
	}
	free(instance);
	return NULL;
}

/** start thread */
void event_tpool_thread_start(EventTPoolThread this) {
	this->tid=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, event_thread_stack_size_g);

#ifdef CHECK_STACKSIZE
	this->stack_adr = (char *) malloc(event_thread_stack_size_g);
	memset(this->stack_adr, MAGIC_NUMBER, event_thread_stack_size_g);
	pthread_attr_setstack(&attr, (void *) this->stack_adr, event_thread_stack_size_g);
#endif
	pthread_create(&this->tid, &attr, event_tpool_thread_main, this);
	pthread_attr_destroy(&attr);
}

/** stop thread */
void event_tpool_thread_stop(EventTPoolThread this) {
	pthread_t tid = this->tid;
	int ret = event_thread_msg_send_stop(this);
	if(0<ret) {
		pthread_join(tid, NULL);
	}
#ifdef CHECK_STACKSIZE
	int i=0;
	for(i=0;i<event_thread_stack_size_g;i++) {
		if (this->stack_adr[i] != MAGIC_NUMBER) break;
	}
	fprintf(stderr, "Used %d byte\n", event_thread_stack_size_g - i);
#endif
}

/** add new subscriber */
void event_tpool_thread_add(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	event_thread_msg_send_add(this, subscriber, arg);
}
void event_tpool_thread_update(EventTPoolThread this, EventSubscriber subscriber, void * arg) {
	event_thread_msg_send_update(this, subscriber, arg);
}

/** delete subscriber */
void event_tpool_thread_del(EventTPoolThread this, int fd) {
	event_thread_msg_send_del(this, fd);
}
void event_thread_atfork_child(EventTPoolThread this) {
	/*because there is no other thread, so always work like on pooled thread */
	this->tid = 0;
	pthread_mutex_init(&this->msgdata.lock, NULL);
	pthread_cond_init(&this->msgdata.cond, NULL);
}

void event_thread_set_stack_size(size_t stack_size) {
	event_thread_stack_size_g = stack_size;
}

int event_tpool_thread_load_plugin(const char *plugin_path) {
	if(event_if_instance_g.handle) {
		return 0;
	}

	const char * path;
	if(!plugin_path) {
		path = event_tpool_thread_get_defaullt_plugin();
	} else {
		path = plugin_path;
	}

	if(!path) {
		DEBUG_ERRPRINT("There is no install plugin, please check it!\n" );
		return -1;
	}

	event_if_instance_g.handle = dlopen(path, RTLD_NOW);
	if(!event_if_instance_g.handle) {
		DEBUG_ERRPRINT("Failed to open %s, err=%s!\n", plugin_path , dlerror() );
		return -1;
	}

	return event_tpool_thread_load_all_fun();
}

int event_tpool_thread_unload_plugin(void) {
	if(event_if_instance_g.handle) {
		dlclose(event_if_instance_g.handle);
	}
	memset(&event_if_instance_g, 0, sizeof(event_if_instance_g));
}
