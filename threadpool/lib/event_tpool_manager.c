/**
 * @file event_tpool_manager.c
 * This is API implement for EventTPoolManager class
**/
#define _GNU_SOURCE 
#include <elf.h>
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
/*! thread instance and fd list */
/*Fix size of max fds 2048 (to care sign */
#define EV_TPOLL_MAXFDS (64)
#define EV_TPOLL_UINT64_BITSIZE (64)
#define EV_TPOLL_USABLE_BITSIZE (32)
/*TODO: use full bit place, care over 2048 fd*/
typedef struct event_tpool_thread_info_t {
	EventTPoolThread tinstance;
	size_t fdcnt;
	uint64_t *fds;
} event_tpool_thread_info_t;

static void event_tpool_thread_set_fds(uint64_t *fds, int fd);
static void event_tpool_thread_unset_fds(uint64_t *fds, int fd);
static int event_tpool_thread_is_set_fds(uint64_t *fds, int fd);

static void event_tpool_thread_insert_fddata(EventTPoolThreadInfo this, int fd);

/*@}*/
/*! @name thread information list API definition.*/
/*@{*/
/*! start thread */
static void event_tpool_thread_info_start_thread(EventTPoolThreadInfo instance);
/*! stop thread */
static void event_tpool_thread_info_stop_thread(EventTPoolThreadInfo this);
/*! search current setting */
static int event_tpool_thread_has_fd(EventTPoolThreadInfo this, int fd);
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
static int event_tpool_manager_get_default_thrednum(void);
/*! search insert place, to use event_tpool_thread_insert_thread*/
static int event_tpool_manager_search_insert_thread(EventTPoolManager this, int fd, int *has_fd);
/*@}*/

/*************
 * for thread information list API
*************/;
static int event_tpoll_get_far_right_bit_index(uint64_t data) {
	int index=0;
	for(index=0;index< EV_TPOLL_UINT64_BITSIZE;index++) {
		if((data)&(0x1<<index))break;
	}
	return index;
}

#define EV_TPOLL_FDSPLACE(fd) ((((uint64_t)fd)-1)/EV_TPOLL_USABLE_BITSIZE)
#define EV_TPOLL_FDINDEX(fd, place) (((uint64_t)fd) - (place*EV_TPOLL_USABLE_BITSIZE) - 1)

static void event_tpool_thread_set_fds(uint64_t *fds, int fd) {
	uint64_t place = EV_TPOLL_FDSPLACE(fd);
	fds[place] |= (0x1) << EV_TPOLL_FDINDEX(fd,place);
}
static void event_tpool_thread_unset_fds(uint64_t *fds, int fd) {
	uint64_t place = EV_TPOLL_FDSPLACE(fd);
	fds[place] &= ~((0x1) << EV_TPOLL_FDINDEX(fd,place));
}
static int event_tpool_thread_is_set_fds(uint64_t *fds, int fd) {
	DEBUG_ERRPRINT("%s:fd=%d\n", __func__, fd);
	uint64_t place = EV_TPOLL_FDSPLACE(fd);
	DEBUG_ERRPRINT( "%s:place=%d\n", __func__, place);
	DEBUG_ERRPRINT( "%s:fds=%x\n", __func__, fds[place]);
	return (fds[place] & ((0x1) << EV_TPOLL_FDINDEX(fd,place)) );
}

static void event_tpool_thread_insert_fddata(EventTPoolThreadInfo this, int fd) {
	event_tpool_thread_set_fds(this->fds, fd);
	this->fdcnt++;
}

static int event_tpool_thread_has_fd(EventTPoolThreadInfo this, int fd) {
	return event_tpool_thread_is_set_fds(this->fds, fd);
}

/*! free fddata */
static void event_tpool_free_fddata_list(EventTPoolThreadInfo this) {
	size_t i = 0;
	int fd=0;
	int fd_base=0;
	for(i = 0; this->fdcnt && i < EV_TPOLL_MAXFDS; i ++, fd_base += EV_TPOLL_USABLE_BITSIZE) {
		while(this->fdcnt && this->fds[i] != 0) {
			/*get fd place*/
			fd = event_tpoll_get_far_right_bit_index(this->fds[i]) + fd_base;

			/*delete event*/
			event_tpool_thread_del(this->tinstance, fd-1);

			/*unset*/
			this->fds[i] &= ~((0x1) << (fd-1 - fd_base));
			this->fdcnt--;
		}
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

/*! delete thread */
static void event_tpool_thread_delete_thread(EventTPoolThreadInfo this, int fd) {
	event_tpool_thread_unset_fds(this->fds, fd);
	this->fdcnt--;
}

/*************
 * for list API
*************/
static EventTPoolThreadInfo event_tpool_thread_info_new(size_t thread_size) {
	EventTPoolThreadInfo info = NULL;
	size_t size = sizeof(*info) * thread_size;/*sizeof EventTPoolThreadInfo*/
	size += (sizeof(uint64_t)*EV_TPOLL_MAXFDS*thread_size+10000);/*sizeof fds in EventTPoolThreadInfo*/
	info = malloc(size);
	if(!info) {
		DEBUG_ERRPRINT("Failed to get instance threads!\n" );
		return NULL;
	}
	memset(info, 0, size);

	/*set place for fds*/
	void *current_p = info + thread_size;

	size_t i=0;
	for( i = 0; i < thread_size; i ++ ) {
		//keep 64bit * EV_TPOLL_MAXFDS => 4096 fds place
		DEBUG_ERRPRINT("info[%d]=%p!\n", i, current_p );
		info[i].fds = (uint64_t *)current_p;
		current_p = info[i].fds + EV_TPOLL_MAXFDS;

		/*create thread instance*/
		info[i].tinstance = event_tpool_thread_new(thread_size);
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

static int event_tpool_manager_get_default_thrednum(void) {
	cpu_set_t child_set;
	CPU_ZERO(&child_set);
	sched_getaffinity(0, sizeof(child_set), &child_set);
	return CPU_COUNT(&child_set)*2;
}

static int event_tpool_manager_search_insert_thread(EventTPoolManager this, int fd, int *has_fd) {
	int threadid=-1;
	size_t i=0;
	*has_fd = 0;
	for( i = 0; i < this->thread_size; i ++ ) {
		//already add fd?
		if(event_tpool_thread_has_fd(&this->threads[i], fd)) {
			*has_fd = 1;
			threadid=i;
			DEBUG_ERRPRINT("event_tpool_thread_has_fd, in %d\n", threadid);
			break;
		}

		//add to other place
		if( 0<= threadid && this->threads[threadid].fdcnt < this->threads[i].fdcnt) {
			continue;
		}

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
//DEBUG_INIT
	EventTPoolManager instance = (EventTPoolManager)calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance!\n" );
		return NULL;
	}

	//get lock instance
	if(is_threadsafe) {
		instance->lock = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
		if(!instance->lock) {
			DEBUG_ERRPRINT("Failed to get instance lock!\n" );
			goto err;
		}
		pthread_mutex_init(instance->lock, NULL);
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
//DEBUG_EXIT
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
	lock = this->lock;
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

event_tpool_add_result_t event_tpool_add(EventTPoolManager this, EventSubscriber subscriber, void * arg) {
	event_tpool_add_result_t result={-1, NULL};
	int id=-1;
	if(!this || !subscriber) {
		return result;
	}
EVT_TPOOL_MNG_LOCK(this);

	int has_fd=0;
	id = event_tpool_manager_search_insert_thread(this, subscriber->fd, &has_fd);

	/*reject  there is same fd*/
	if(has_fd) {
		DEBUG_ERRPRINT("There is a setting for fd %d, please use event_tpool_update!\n", subscriber->fd);
		goto end;
	}

	/*accept if there is no same fd*/
	event_tpool_thread_insert_fddata(&this->threads[id], subscriber->fd);
	result.event_handle = &this->threads[id];
	result.result = id;
end:

EVT_TPOOL_MNG_UNLOCK;
	/*after unlock, call to add API on event_thread*/
	if(result.event_handle) event_tpool_thread_add(result.event_handle->tinstance, subscriber, arg);
	return result;
}

event_tpool_add_result_t event_tpool_add_thread(EventTPoolManager this, int threadid, EventSubscriber subscriber, void * arg) {
	event_tpool_add_result_t result={.result=-1, .event_handle=NULL};
	if(!this || !subscriber) {
		return result;
	}
	if(threadid < 0 || this->thread_size <= threadid) {
		return result;
	}

EVT_TPOOL_MNG_LOCK(this);
	int id, has_fd=0;
	id = event_tpool_manager_search_insert_thread(this, subscriber->fd, &has_fd);
	/*reject  there is same fd*/
	if(has_fd) {
		DEBUG_ERRPRINT("There is a setting for fd %d, please use event_tpool_update!\n", subscriber->fd);
		goto end;
	}

	/*accept if there is no same fd*/
	event_tpool_thread_insert_fddata(&this->threads[threadid], subscriber->fd);
	result.event_handle = &this->threads[threadid];
	result.result = threadid;

end:
EVT_TPOOL_MNG_UNLOCK;
	/*after unlock, call to add API on event_thread*/
	if(result.event_handle) event_tpool_thread_add(result.event_handle->tinstance, subscriber, arg);
	return result;
}

event_tpool_add_result_t event_tpool_update(EventTPoolManager this, EventTPoolThreadInfo event_handle, EventSubscriber subscriber, void * arg) {
	event_tpool_add_result_t result={.result=-1, .event_handle=NULL};
	int id = -1;
	if(!this || !subscriber || !event_handle) {
		return result;
	}

EVT_TPOOL_MNG_LOCK(this);

	/*Check is it deleted?*/
	int has_fd;
	id = event_tpool_manager_search_insert_thread(this, subscriber->fd, &has_fd);
	if(!has_fd) {
		DEBUG_ERRPRINT("There is no setting for fd %d, please use event_tpool_add!\n", subscriber->fd);
		goto end;
	}

	result.event_handle = event_handle;
	result.result = id;

end:
EVT_TPOOL_MNG_UNLOCK;
	/*after unlock, call to add API on event_thread*/
	if(result.event_handle) event_tpool_thread_update(result.event_handle->tinstance, subscriber, arg);
	return result;
}

void event_tpool_del(EventTPoolManager this, int fd) {
	int id=0, has_fd=0;
	if(!this) {
		return;
	}
EVT_TPOOL_MNG_LOCK(this);

	id = event_tpool_manager_search_insert_thread(this, fd, &has_fd);
	if(!has_fd) {
		DEBUG_ERRPRINT("There is no setting for fd %d, please use event_tpool_update!\n", fd);
		goto end;
	}

	event_tpool_thread_delete_thread(&this->threads[id], fd);
end:
EVT_TPOOL_MNG_UNLOCK;

	/*after unlock, call to delete API on event_thread*/
	if(has_fd) {
		event_tpool_thread_del(this->threads[id].tinstance, fd);
	}
}

void event_tpool_atfork_child(EventTPoolManager this) {
	if(this->lock) pthread_mutex_init(this->lock, NULL);

	size_t i=0;
	for(i = 0; i < this->thread_size; i++ ) {
		event_thread_atfork_child(this->threads[i].tinstance);
	}
}
