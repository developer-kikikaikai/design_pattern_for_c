/**
 * @file state_machine.c
 * @brief      Implement of StateMachine library API, defined in state_machine.h.
 **/
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
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

/*! @struct state_manager_list_data_t
 * @brief information of StateMachine, to use as list
*/
struct state_machine_t {
	StateManagerListData head;
	StateManagerListData tail;
	int (*call_event)(StateMachine this, int event, void *arg, void (*response)(int result));/* call event API, to switch single/multi thread*/
	void (*free)(StateMachine this);/* call free API, to switch single/multi thread*/
	/*data for multi thread mode*/
	pthread_t tid;
	int sockpair[2];
};

/*! @struct state_machine_msg_t
 * @brief message definition for multi thread
*/
typedef struct state_machine_msg_t {
	int is_stop;/*! is_stop when stop thread, send msg with is_stop=1*/
	int event;
	void * args;
	void (*response)(int result);
} state_machine_msg_t;

/*! @name private API for StateMachine */
/* @{ */
/*! Add new state. */
static int state_machine_add_new_states(StateMachine this, const state_event_info_t * event_info);
/*! free normally */
static void state_machine_free_states_normally(StateMachine this);
/*! call state method normally */
static int state_machine_call_event_normally(StateMachine this, int event, void *arg, void (*response)(int result));
/*! Find state. */
StateManagerListData state_machine_find_states(StateMachine this, int event);
/*! for multi thread, initialize */
static int state_machine_initial_thread(StateMachine this);
/*! for multi thread, free */
static void state_machine_free_states_multithread(StateMachine this);
/*! for multi thread, call event */
static int state_machine_call_event_multithread(StateMachine this, int event, void *arg, void (*response)(int result));
/*! for multi thread, thread main */
static void * state_machine_thread_main(void *arg);
/*! for multi thread, open socket */
static int state_machine_open_socket(StateMachine this);
/*! for multi thread, close socket */
static void state_machine_close_socket(StateMachine this);
/*! for multi thread, read */
static inline int state_machine_read(StateMachine this, state_machine_msg_t *msg);
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

/*! call state method normally */
static int state_machine_call_event_normally(StateMachine this, int event, void *arg, void (*response)(int result)) {
	/* not use response , only for use same format of multi thread*/
	(void)response;

	StateManagerListData state_manager = state_machine_find_states(this, event);
	if(!state_manager) {
		return STATE_MNG_FAILED;
	}

	return state_manager_call(state_manager->states, arg);
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
	if(state_machine_open_socket(this)) {
		DEBUG_ERRPRINT("Failed to create socket pair!\n");
		return STATE_MNG_FAILED;
	}

	if(pthread_create(&this->tid, NULL, state_machine_thread_main, this)) {
		state_machine_close_socket(this);
		DEBUG_ERRPRINT("Failed to create socket pair!\n");
		return STATE_MNG_FAILED;
	}

	return STATE_MNG_SUCCESS;
}

/*! for multi thread, free */
static void state_machine_free_states_multithread(StateMachine this) {
	/* send exit message*/
	state_machine_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.is_stop = 1;

	if(state_machine_write(this, &msg) < 0) {
		DEBUG_ERRPRINT("...failed to send, errno=%s\n", strerror(errno));
		/* cancel force */
		pthread_cancel(this->tid);
	}
	/* wait to stop thread */
	pthread_join(this->tid, NULL);

	/*close socket*/
	state_machine_close_socket(this);

	/* free normally */
	state_machine_free_states_normally(this);
}

/*! for multi thread, call event */
static int state_machine_call_event_multithread(StateMachine this, int event, void *args, void (*response)(int result)) {
	state_machine_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.event = event;
	msg.args = args;
	msg.response = response;

	int ret = STATE_MNG_SUCCESS;
	if(state_machine_write(this, &msg) < 0) {
		DEBUG_ERRPRINT("...failed to send, errno=%s\n", strerror(errno));
		ret = STATE_MNG_FAILED;
	}
	return ret;
}

/*! for multi thread, thread main */
static void * state_machine_thread_main(void *arg) {
	StateMachine this = (StateMachine)arg;
	int ret = 0;
	state_machine_msg_t msg;
	while(1) {
		ret = state_machine_read(this, &msg);
		if(ret < 0) {
			DEBUG_ERRPRINT("failed to read, %s\n",  strerror(errno));
			break;
		}

		if(msg.is_stop) {
			/* finish to running thread*/
			break;
		}

		/* call */
		ret = state_machine_call_event_normally(this, msg.event, msg.args, NULL);
		if(msg.response) {
			msg.response(ret);
		}
	}

	pthread_exit(NULL);
	return NULL;
}

/*! for multi thread, open socket */
static int state_machine_open_socket(StateMachine this) {
	return socketpair(AF_UNIX, SOCK_DGRAM, 0, this->sockpair);
}

/*! for multi thread, close socket */
static void state_machine_close_socket(StateMachine this) {
	close(this->sockpair[0]);
	close(this->sockpair[1]);
}

#define SOCK_READID (0)
#define SOCK_WRITEID (1)

/*! for multi thread, read */
static inline int state_machine_read(StateMachine this, state_machine_msg_t *msg) {
	return read(this->sockpair[SOCK_READID], msg, sizeof(state_machine_msg_t));
}

/*! for multi thread, write */
static inline int state_machine_write(StateMachine this, state_machine_msg_t *msg) {
	return write(this->sockpair[SOCK_WRITEID], msg, sizeof(state_machine_msg_t));
}
/* @} */

/*************
 * public API
*************/
StateMachine state_machine_new(size_t event_num, const state_event_info_t * event_infos, int is_multithread) {
	StateMachine instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("allocate error\n");
		goto err;
	}

	/* set data */
	int i = 0;
	for( i = 0; i < event_num; i ++ ) {
		if(state_machine_add_new_states(instance, &event_infos[i]) != STATE_MNG_SUCCESS) {
	
			goto err;
		}
	}

	if(is_multithread) {
		/* set information for multi thread */
		instance->call_event = state_machine_call_event_multithread;
		instance->free = state_machine_free_states_multithread;
		if( state_machine_initial_thread(instance) != STATE_MNG_SUCCESS ) {
			/* change free function because starting thread is failed */
			instance->free = state_machine_free_states_normally;
			goto err;
		}
	} else {
		/* only set call_event and free API */
		instance->call_event = state_machine_call_event_normally;
		instance->free = state_machine_free_states_normally;
	}
	return instance;
err:
	state_machine_free(instance);
	return NULL;
}

int state_machine_update_machine(StateMachine this, const state_event_info_t * event_info) {
	if(!this || !event_info) {
		return STATE_MNG_FAILED;
	}

	StateManagerListData state_manager = state_machine_find_states(this, event_info->event);

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
		ret = state_machine_add_new_states(this, event_info);
	}
	return ret;
}

void state_machine_set_state(StateMachine this, int state) {
	if(!this) {
		return;
	}

	/* Set all state_manager's state*/
	StateManagerListData state_manager = this->head;
	while(state_manager) {
		state_manager_set_state(state_manager->states, state);
		state_manager=state_manager->next;
	}
	return;
}

int state_machine_call_event(StateMachine this, int event, void *arg, void (*response)(int result)) {
	if(!this) {
		return STATE_MNG_FAILED;
	}

	return this->call_event(this, event, arg, response);
}

void state_machine_show(StateMachine this) {
	if(!this) {
		return;
	}

	StateManagerListData state_manager=this->head;
	while(state_manager) {
		printf("===[event: %d]===\n", state_manager->event);
		state_manager_show(state_manager->states);
		state_manager=state_manager->next;
	}
}

void state_machine_free(StateMachine this) {
	if(!this) {
		return;
	}

	this->free(this);
	free(this);
}
