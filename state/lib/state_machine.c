/**
 * @file state_machine.c
 * @brief      Implement of StateMachine library API, defined in state_machine.h.
 **/
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "state_machine.h"
#include "dp_util.h"

/*************
 * public define
*************/

/*! @struct state_manager_list_data_t
 * @brief information of StateMachine, to use as list
*/
struct state_manager_list_data_t;
typedef struct state_manager_list_data_t * StateManagerListData;
struct state_manager_list_data_t {
	StateManagerListData next;
	StateManagerListData prev;
	int event;
	StateManager states;
};

/*! @name private API for state_manager_t */
/* @{ */
/*! Add new state. */
static StateManagerListData state_machine_manager_list_new(const state_event_info_t * event_info);
static void state_machine_manager_list_free(StateManagerListData this);
/* }@ */

struct state_machine_msg_t;
typedef struct state_machine_msg_t state_machine_msg_t , *StateMachineMsg;
/*! information of state machine message list */
struct state_machine_msg_list_t {
	StateMachineMsg head;
	StateMachineMsg tail;
	pthread_mutex_t lock;
	int eventfd;
};
typedef struct state_machine_msg_list_t state_machine_msg_list_t;
/*! @struct state_manager_list_data_t
 * @brief information of StateMachine, to use as list
*/
struct state_machine_t {
	StateManagerListData head;
	StateManagerListData tail;
	int (*call_event)(StateMachine this, int event, void *arg, int arglen, void (*response)(int result));/* call event API, to switch single/multi thread*/
	int (*call_single_event)(StateMachine this, int event, void *arg, int arglen, void (*response)(int result));/* call event API, to call own thread*/
	void (*free)(StateMachine this);/* call free API, to switch single/multi thread*/
	/*data for multi thread mode*/
	EventTPoolManager threadpool;
	pthread_t tid;

	/*for recv event */
	state_machine_msg_list_t msglist;
};

#define SMACHINE_LOCK(this) DPUTIL_LOCK(&this->msglist.lock);
#define SMACHINE_UNLOCK DPUTIL_UNLOCK

/*! @struct state_machine_msg_t
 * @brief message definition for multi thread
*/
struct state_machine_msg_t {
	StateMachineMsg next;
	StateMachineMsg prev;
	int event;
	void * args;
	void (*response)(int result);
};

/*! @name private API for StateMachine */
/* @{ */
/*! Add new state. */
static int state_machine_add_new_states(StateMachine this, const state_event_info_t * event_info);
/*! free normally */
static void state_machine_free_states_normally(StateMachine this);
/*! Find state. */
StateManagerListData state_machine_find_states(StateMachine this, int event);
/*! for multi thread, initialize */
static int state_machine_initial_thread(StateMachine this);
/*! for multi thread, free */
static void state_machine_free_states_multithread(StateMachine this);
/*! for single thread, call event */
static int state_machine_call_event_normally(StateMachine this, int event, void *arg, int arglen, void (*response)(int result));
static inline int state_machine_call_event_normal(StateMachine this, int event, void *arg);
/*! for multi thread, call event */
static int state_machine_call_event_multithread(StateMachine this, int event, void *arg, int arglen, void (*response)(int result));
/*! for multi thread, thread main callback for threadpool */
static void state_machine_thread_main(evutil_socket_t socketfd, short eventflag, void * event_arg);
/*! for multi thread, open socket */
static inline int state_machine_open_socket(StateMachine this);
/*! for multi thread, close socket */
static void state_machine_close_socket(StateMachine this);
/*! for multi thread, get read sock */
static inline int state_machine_get_read(StateMachine this);
/*! for multi thread, write */
static inline int state_machine_write(StateMachine this, state_machine_msg_t *msg);
/* @} */
/*************
 * private API for StateManagerListData
*************/
/*! @name private API for StateManagerListData */
/* @{ */
static StateManagerListData state_machine_manager_list_new(const state_event_info_t * event_info) {
	StateManagerListData state_manager = calloc(1, sizeof(*state_manager));
	if(!state_manager) {
		DEBUG_ERRPRINT("allocate StateManagerListData error\n");
		return NULL;
	}

	state_manager->states = state_manager_new(event_info->state_num, event_info->state_infos);
	if(!state_manager->states) {
		DEBUG_ERRPRINT("allocate StateManager error\n");
		free(state_manager);
		return NULL;
	}

	state_manager->event = event_info->event;
	return state_manager;
}

static void state_machine_manager_list_free(StateManagerListData this) {
	if(!this) {
		return;
	}

	state_manager_free(this->states);
	free(this);
}
/* @}*/

/*************
 * private API for StateMachine
*************/
/*! @name private API for StateMachine */
/* @{ */
/*! Add new state. */
static int state_machine_add_new_states(StateMachine this, const state_event_info_t * event_info) {
	StateManagerListData state_manager = state_machine_manager_list_new(event_info);
	if(!state_manager) {
		DEBUG_ERRPRINT("allocate StateManagerListData error\n");
		return STATE_MNG_FAILED;
	}

	/*push to list*/
	dputil_list_push((DPUtilList)this, (DPUtilListData)state_manager);
	return STATE_MNG_SUCCESS;
}

/*! free normally */
static void state_machine_free_states_normally(StateMachine this) {
	StateManagerListData state_manager=(StateManagerListData)dputil_list_pop((DPUtilList)this);
	while(state_manager) {
		state_machine_manager_list_free(state_manager);
		state_manager=(StateManagerListData)dputil_list_pop((DPUtilList)this);
	}
}

/*! Find state. */
StateManagerListData state_machine_find_states(StateMachine this, int event) {
	StateManagerListData state_manager = this->head;
	while(state_manager) {
		if(state_manager->event == event) {
			break;
		}
		state_manager=state_manager->next;
	}
	return state_manager;
}

/*! for multi thread, initialize */
static int state_machine_initial_thread(StateMachine this) {
	/* create socket */
	if(-1==state_machine_open_socket(this)) {
		DEBUG_ERRPRINT("Failed to create socket pair!\n");
		return STATE_MNG_FAILED;
	}

	event_subscriber_t subscriber={
		.fd = state_machine_get_read(this),
		.eventflag = EV_READ | EV_PERSIST,
		.event_callback = state_machine_thread_main,
	};

	event_tpool_add_result_t result =  event_tpool_add(this->threadpool, &subscriber, this);
	return result.result;
}

/*! for multi thread, free */
static void state_machine_free_states_multithread(StateMachine this) {
	/* exit event */
	event_tpool_del(this->threadpool, state_machine_get_read(this));

	/*close socket*/
	state_machine_close_socket(this);

	/* free normally */
	state_machine_free_states_normally(this);
}

/*! for single thread, call event */
static int state_machine_call_event_normally(StateMachine this, int event, void *arg, int arglen, void (*response)(int result)) {
	(void)response;
	(void)arglen;
	return state_machine_call_event_normal(this, event, arg);
}

static inline int state_machine_call_event_normal(StateMachine this, int event, void *arg) {
	StateManagerListData state_manager = state_machine_find_states(this, event);
	if(!state_manager) {
		return STATE_MNG_FAILED;
	}

	return state_manager_call(state_manager->states, arg);
}

/*! for multi thread, call event */
static int state_machine_call_event_multithread(StateMachine this, int event, void *arg, int arglen, void (*response)(int result)) {
	StateMachineMsg msg = calloc(1, sizeof(*msg) + arglen);
	if(!msg) {
		return STATE_MNG_FAILED;
	}
	msg->event = event;
	msg->args = (void *)(msg + 1);

	memcpy(msg->args, arg, arglen);
	msg->response = response;

	int ret = STATE_MNG_SUCCESS;
	if(state_machine_write(this, msg) < 0) {
		DEBUG_ERRPRINT("...failed to send, errno=%s\n", strerror(errno));
		free(msg);
		ret = STATE_MNG_FAILED;
	}
	return ret;
}

/*! for multi thread, thread main */
static void state_machine_thread_main(evutil_socket_t socketfd, short eventflag, void * event_arg) {
	StateMachine this = (StateMachine)event_arg;
	int ret = 0;
	StateMachineMsg msg;
	eventfd_t cnt=0;

	//set threadid
	if(!this->tid) {
		this->tid = pthread_self();
	}

	//read event
	ret = eventfd_read(socketfd, &cnt);
	if(ret < 0) {
		DEBUG_ERRPRINT("failed to read, %s\n",  strerror(errno));
		return;
	}

SMACHINE_LOCK(this);
	while(1) {
		msg = (StateMachineMsg)dputil_list_pop((DPUtilList)&this->msglist);
		if(!msg) {
			break;
		}
		/* call */
		ret = state_machine_call_event_normal(this, msg->event, msg->args);
		if(msg->response) {
			msg->response(ret);
		}

		free(msg);
	}
SMACHINE_UNLOCK(this);
}

/*! for multi thread, open socket */
static inline int state_machine_open_socket(StateMachine this) {
	pthread_mutex_init(&this->msglist.lock, NULL);
	this->msglist.eventfd = eventfd(0,EFD_CLOEXEC | EFD_NONBLOCK);
	return this->msglist.eventfd;
}

/*! for multi thread, close socket */
static void state_machine_close_socket(StateMachine this) {
	close(this->msglist.eventfd);
}


/*! for multi thread, read */
static inline int state_machine_get_read(StateMachine this) {
	return this->msglist.eventfd;
}

/*! for multi thread, write */
static inline int state_machine_write(StateMachine this, state_machine_msg_t *msg) {
SMACHINE_LOCK(this)
	dputil_list_push((DPUtilList)(&this->msglist), (DPUtilListData)msg);
SMACHINE_UNLOCK
	return eventfd_write(this->msglist.eventfd, 1);
}
/* @} */

/*************
 * public API
*************/
StateMachineInfo state_machine_new(size_t event_num, const state_event_info_t * event_infos, EventTPoolManager threadpool) {
	StateMachineInfo instance_info = calloc(1, sizeof(*instance_info));
	if(!instance_info) {
		DEBUG_ERRPRINT("allocate error\n");
		return NULL;
	}

	StateMachine instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("allocate error\n");
		goto err;
	}
	instance_info->state_machine = instance;

	/* set data */
	int i = 0;
	for( i = 0; i < event_num; i ++ ) {
		if(state_machine_add_new_states(instance, &event_infos[i]) != STATE_MNG_SUCCESS) {
	
			goto err;
		}
	}

	if(threadpool) {
		/* set information for multi thread */
		instance->call_event = state_machine_call_event_multithread;
		instance->call_single_event = state_machine_call_event_normally;
		instance->free = state_machine_free_states_multithread;
		instance->threadpool = threadpool;
		instance_info->thread_num = state_machine_initial_thread(instance);
		if( instance_info->thread_num < 0 ) {
			/* change free function because starting thread is failed */
			instance->free = state_machine_free_states_normally;
			goto err;
		}
	} else {
		/* only set call_event and free API */
		instance->call_event = state_machine_call_event_normally;
		instance->call_single_event = state_machine_call_event_normally;
		instance->free = state_machine_free_states_normally;
	}
	return instance_info;
err:
	state_machine_free(instance_info);
	return NULL;
}

int state_machine_update_machine(StateMachineInfo this, const state_event_info_t * event_info) {
	if(!this || !event_info) {
		return STATE_MNG_FAILED;
	}

	StateManagerListData state_manager = state_machine_find_states(this->state_machine, event_info->event);

	int ret = STATE_MNG_SUCCESS;
	if(state_manager) {
		/* To update StateManager , keep current state */
		int current_state = state_manager_get_current_state(state_manager->states);
		state_manager_free(state_manager->states);

		/* Set new state manager */
		state_manager->states = state_manager_new(event_info->state_num, event_info->state_infos);
		if(!state_manager->states) {
			ret = STATE_MNG_FAILED;
		}

		/* set state */
		state_manager_set_state(state_manager->states, current_state);
	} else {
		/* only add new event */
		ret = state_machine_add_new_states(this->state_machine, event_info);
	}
	return ret;
}

void state_machine_set_state(StateMachineInfo this, int state) {
	if(!this) {
		return;
	}

	/* Set all state_manager's state*/
	StateManagerListData state_manager = this->state_machine->head;
	while(state_manager) {
		state_manager_set_state(state_manager->states, state);
		state_manager=state_manager->next;
	}
	return;
}

int state_machine_call_event(StateMachineInfo this, int event, void *arg, int arglen, void (*response)(int result)) {
	if(!this) {
		return STATE_MNG_FAILED;
	}

	if(this->state_machine->tid && this->state_machine->tid == pthread_self() ) {
		return this->state_machine->call_single_event(this->state_machine, event, arg, arglen, response);
	} else {
		return this->state_machine->call_event(this->state_machine, event, arg, arglen, response);
	}
}

void state_machine_show(StateMachineInfo this) {
	if(!this) {
		return;
	}

	StateManagerListData state_manager=this->state_machine->head;
	while(state_manager) {
		printf("===[event: %d]===\n", state_manager->event);
		state_manager_show(state_manager->states);
		state_manager=state_manager->next;
	}
}

void state_machine_free(StateMachineInfo this) {
	if(!this) {
		return;
	}

	this->state_machine->free(this->state_machine);
	free(this->state_machine);
	free(this);
}
