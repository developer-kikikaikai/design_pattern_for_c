/**
 * @file event_tpool_manager.c
 * This is API implement for EventTPoolManager class
**/
#define _GNU_SOURCE 
#include <stdio.h>
#include <sched.h>
#include "event_threadpool.h"
#include "event_thread.h"
#include "dp_util.h"

/*************
 * public define
*************/
/*! @name thread information list definition.*/
/*@{*/
/*! used fd data list typedef */
typedef struct event_tpool_fd_data_t *EventTPoolFDData;
/*! used fd data list */
struct event_tpool_fd_data_t {
	EventTPoolFDData next;
	EventTPoolFDData prev;
	int fd;
};

/*! thread instance and fd list */
typedef struct event_tpool_thread_info_t {
	EventTPoolFDData head;
	EventTPoolFDData tail;
	size_t fdcnt;
	EventTPoolThread tinstance;
} event_tpool_thread_info_t, *EventTPoolThreadInfo;

#define event_tpool_thread_pop(this)  (EventTPoolFDData)dputil_list_pop((DPUtilList)(this))
#define event_tpool_thread_pull(this, data)  dputil_list_pull((DPUtilList)this, (DPUtilListData)data);
#define event_tpool_thread_push(this, data) dputil_list_push((DPUtilList)this, (DPUtilListData)data);
#define event_tpool_thread_insert(this, fdplace_lt, data) dputil_list_insert((DPUtilList)this, (DPUtilListData)fdplace_lt, (DPUtilListData)data);

static inline void event_tpool_thread_insert_fddata(EventTPoolThreadInfo this, EventTPoolFDData fdplace_lt, EventTPoolFDData data);

/*! to insert request check */
typedef struct event_tpool_insert_info_t {
	EventTPoolFDData fdplace_lt;
	EventTPoolThreadInfo threadinfo;
} event_tpool_insert_info_t;
/*@}*/
/*! @name thread information list API definition.*/
/*@{*/
/*! start thread */
static void event_tpool_thread_info_start_thread(EventTPoolThreadInfo instance);
/*! stop thread */
static void event_tpool_thread_info_stop_thread(EventTPoolThreadInfo this);
/*! insert new thread */
static void event_tpool_thread_insert_thread(event_tpool_insert_info_t * info, EventSubscriber subscriber, void * arg);
/*! delete thread */
static void event_tpool_thread_delete_thread(event_tpool_insert_info_t * info, int fd);
/*! search current setting */
static EventTPoolFDData event_tpool_thread_info_search_insert_place(EventTPoolThreadInfo this, int fd);
/*@}*/
/*! @name EventTPoolThreadInfo list API and EventTPoolFDData list API definition.*/
/*@{*/
/*! new thread info */
static EventTPoolThreadInfo event_tpool_thread_info_new(size_t thread_size);
/*! free thread info */
static void event_tpool_thread_info_free(EventTPoolThreadInfo this, size_t thread_size);
/*! free fddata */
static void event_tpool_free_fddata_list(EventTPoolThreadInfo this);
/*@}*/

/*! @name for EventTPoolManager definition .*/
/*@{*/
/*! EventTPoolManager class instance definition. */
struct event_tpool_manager_t {
	size_t thread_size;
	EventTPoolThreadInfo threads;
	pthread_mutex_t *lock;
};
/*@}*/

/*! @name for EventTPoolManager private API definition .*/
/*@{*/
#define EVT_TPOOL_MNG_LOCK(this) DPUTIL_LOCK(this->lock);
#define EVT_TPOOL_MNG_UNLOCK DPUTIL_UNLOCK;
static void event_tpool_manager_free_without_lock(EventTPoolManager this);
static inline int event_tpool_manager_get_default_thrednum(void);
/*! search insert place, to use event_tpool_thread_insert_thread*/
static int event_tpool_manager_search_insert_thread(EventTPoolManager this, int fd, event_tpool_insert_info_t * info);
/*@}*/

/*************
 * for thread information list API
*************/
static inline void event_tpool_thread_insert_fddata(EventTPoolThreadInfo this, EventTPoolFDData fdplace_lt, EventTPoolFDData data) {
	if(!fdplace_lt) {
		event_tpool_thread_push(this, data);
	} else {
		event_tpool_thread_insert(this, fdplace_lt, data);
	}
	this->fdcnt++;
}

/*! free fddata */
static void event_tpool_free_fddata_list(EventTPoolThreadInfo this) {
	EventTPoolFDData data = event_tpool_thread_pop(this);
	while(data) {
		/*delete event*/
		event_tpool_thread_del(this->tinstance, data->fd);
		free(data);
		data = event_tpool_thread_pop(this);
	}
}
static void event_tpool_thread_info_start_thread(EventTPoolThreadInfo instance) {
	event_tpool_thread_start(instance->tinstance);
}

/*! stop thread */
static void event_tpool_thread_info_stop_thread(EventTPoolThreadInfo this) {
	//delete all list
	event_tpool_free_fddata_list(this);
	//stop thread, instance is release in thread
	event_tpool_thread_stop(this->tinstance);
}
/*! insert new thread */
static void event_tpool_thread_insert_thread(event_tpool_insert_info_t * info, EventSubscriber subscriber, void * arg) {
	if(!(info->fdplace_lt) || info->fdplace_lt->fd != subscriber->fd ) {
		EventTPoolFDData fddata = calloc(1, sizeof(*fddata));
		if(!fddata) {
			return;
		}
		fddata->fd = subscriber->fd;
		event_tpool_thread_insert_fddata(info->threadinfo, info->fdplace_lt, fddata);
		event_tpool_thread_add(info->threadinfo->tinstance, subscriber, arg);
	} else {
		event_tpool_thread_update(info->threadinfo->tinstance, subscriber, arg);
	}
}

/*! delete thread */
static void event_tpool_thread_delete_thread(event_tpool_insert_info_t * info, int fd) {
	//prev==own
	event_tpool_thread_pull(info->threadinfo, info->fdplace_lt);
	free(info->fdplace_lt);
	info->threadinfo->fdcnt--;
	event_tpool_thread_del(info->threadinfo->tinstance, fd);
}

/*! search current setting */
static EventTPoolFDData event_tpool_thread_info_search_insert_place(EventTPoolThreadInfo this, int fd) {
	EventTPoolFDData data = this->head;
	EventTPoolFDData prev_less_than = NULL;
	while(data) {
		if(fd < data->fd) {
			break;
		}
		prev_less_than=data;
		data=data->next;
	}
	return prev_less_than;
}
/*************
 * for list API
*************/
static EventTPoolThreadInfo event_tpool_thread_info_new(size_t thread_size) {
	EventTPoolThreadInfo info = calloc(thread_size, sizeof(*info));
	if(!info) {
		DEBUG_ERRPRINT("Failed to get instance threads!\n" );
		return NULL;
	}

	size_t i=0;
	for( i = 0; i < thread_size; i ++ ) {
		info[i].tinstance = event_tpool_thread_new();
		if(!info[i].tinstance) {
			DEBUG_ERRPRINT("Failed to create thread new!\n" );
			return NULL;
		}

		event_tpool_thread_info_start_thread(&info[i]);
	}
	return info;
}

/*! free thread info */
static void event_tpool_thread_info_free(EventTPoolThreadInfo this, size_t thread_size) {
	size_t i=0;
	for( i = 0; i < thread_size ; i ++ ) {
		event_tpool_thread_info_stop_thread(&this[i]);
	}
	free(this);
}
/*************
 * for private  API
*************/
static void event_tpool_manager_free_without_lock(EventTPoolManager this) {
	event_tpool_thread_info_free(this->threads, this->thread_size);
	//lock is free after unlock
	free(this);
}

static inline int event_tpool_manager_get_default_thrednum(void) {
	cpu_set_t child_set;
	CPU_ZERO(&child_set);
	sched_getaffinity(0, sizeof(child_set), &child_set);
	return CPU_COUNT(&child_set)*2;
}

static int event_tpool_manager_search_insert_thread(EventTPoolManager this, int fd, event_tpool_insert_info_t * info) {
	int threadid=0;
	size_t i=0;
	EventTPoolFDData tmpfddata=NULL;
	for( i = 0; i < this->thread_size; i ++ ) {
		//already add fd?
		tmpfddata = event_tpool_thread_info_search_insert_place(&this->threads[i], fd);
		if( tmpfddata && tmpfddata->fd == fd) {
			info->threadinfo = &this->threads[i];
			info->fdplace_lt = tmpfddata;
			threadid=i;
			break;
		}

		//add to other place
		if( (info->threadinfo) && (info->threadinfo->fdcnt < this->threads[i].fdcnt) ) {
			continue;
		}

		info->threadinfo = &this->threads[i];
		info->fdplace_lt = tmpfddata;
		threadid=i;
	}
	return threadid;
}

/*************
 * for public API
*************/
/*! @name API for EventTPoolManager instance */
/*@{*/
EventTPoolManager event_tpool_manager_new(int thread_num, int is_threadsafe) {
	pthread_mutex_t *lock=NULL;
	EventTPoolManager instance = (EventTPoolManager)calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance!\n" );
		return NULL;
	}

	//get lock instance
	if(is_threadsafe) {
		instance->lock = (pthread_mutex_t *)calloc(1, sizeof(*instance->lock));
		if(!instance->lock) {
			DEBUG_ERRPRINT("Failed to get instance lock!\n" );
			goto err;
		}
	}

	//get thread instance
	if(thread_num<=0) {
		instance->thread_size = event_tpool_manager_get_default_thrednum();
	} else {
		instance->thread_size = (size_t)thread_num;
	}

	instance->threads = event_tpool_thread_info_new(instance->thread_size);
	if(!instance->threads) {
		DEBUG_ERRPRINT("Failed to get instance threads!\n" );
		goto err;
	}
	return instance;

err:
	lock=instance->lock;
	event_tpool_manager_free_without_lock(instance);
	free(lock);
	return NULL;
}

void event_tpool_manager_free(EventTPoolManager this) {
	pthread_mutex_t *lock=NULL;
	if(!this) {
		return;
	}
EVT_TPOOL_MNG_LOCK(this);
	lock=this->lock;
	event_tpool_manager_free_without_lock(this);
EVT_TPOOL_MNG_UNLOCK
	free(lock);
}

size_t event_tpool_manager_get_threadnum(EventTPoolManager this) {
	size_t size=0;
	if(!this) {
		return -1;
	}

EVT_TPOOL_MNG_LOCK(this);
	size = this->thread_size;
EVT_TPOOL_MNG_UNLOCK
	return size;
}

int event_tpool_add(EventTPoolManager this, EventSubscriber subscriber, void * arg) {
	int id=-1;
	if(!this || !subscriber) {
		return id;
	}
EVT_TPOOL_MNG_LOCK(this);

	event_tpool_insert_info_t info={0};

	id = event_tpool_manager_search_insert_thread(this, subscriber->fd, &info);

	event_tpool_thread_insert_thread(&info, subscriber, arg);
EVT_TPOOL_MNG_UNLOCK;
	return id;
}

int event_tpool_add_thread(EventTPoolManager this, int threadid, EventSubscriber subscriber, void * arg) {
	int id=-1;
	if(!this || !subscriber) {
		return id;
	}
EVT_TPOOL_MNG_LOCK(this);
	if(0 <= threadid && threadid < this->thread_size) {
		event_tpool_insert_info_t info;
		info.threadinfo=&this->threads[threadid];
		info.fdplace_lt = event_tpool_thread_info_search_insert_place(&this->threads[threadid], subscriber->fd);
		event_tpool_thread_insert_thread(&info, subscriber, arg);
		id = threadid;
	}
EVT_TPOOL_MNG_UNLOCK;
	return id;
}

void event_tpool_del(EventTPoolManager this, int fd) {
	int id=0;
	if(!this) {
		return;
	}
EVT_TPOOL_MNG_LOCK(this);

	event_tpool_insert_info_t info={0};
	id = event_tpool_manager_search_insert_thread(this, fd, &info);
	if(0 <= id && info.fdplace_lt) {
		event_tpool_thread_delete_thread(&info, fd);
	}
EVT_TPOOL_MNG_UNLOCK;
}
/*@}*/
