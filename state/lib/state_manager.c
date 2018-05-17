/**
 *    @brief      Implement of State design petten library API, defined in state_manager.h, only care  state_manager class
 **/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state_manager.h"
#include "dp_util.h"

/*************
 * public define
*************/

/*! @struct state_manager_state_info_t
 * @brief state info definition. instance storaged by list
*/
struct state_manager_state_info_t;
typedef struct state_manager_state_info_t * StateManagerStateInfo;
struct state_manager_state_info_t {
	StateManagerStateInfo next;
	StateManagerStateInfo prev;
	state_info_t state;
};

/*! @struct state_manager_t
 * @brief StateManagerClass instance definition.
*/
struct state_manager_t {
	StateManagerStateInfo head;
	StateManagerStateInfo tail;
	StateManagerStateInfo current_state;
};

/*! @name private API for state_manager_t */
/* @{ */
/*! Add new state. */
static int state_manager_add_new_state(StateManagerClass this, const state_info_t * state);
/*! Find state. */
StateManagerStateInfo state_manager_find_state(StateManagerClass this, int state);
/* }@ */
/*************
 * private API
*************/
static int state_manager_add_new_state(StateManagerClass this, const state_info_t * state) {
ENTERLOG
	/*allocate and copy state info*/
	StateManagerStateInfo state_info=calloc(1, sizeof(*state_info));
	if(!state_info) {
		DEBUG_ERRPRINT("allocate state_info error\n");
		return STATE_MNG_FAILED;
	}

	memcpy(&state_info->state, state, sizeof(state_info_t));
	/*push to list*/
	dputil_list_push((DPUtilList)this, (DPUtilListData)state_info);
EXITLOG
	return STATE_MNG_SUCCESS;
}

StateManagerStateInfo state_manager_find_state(StateManagerClass this, int state) {
ENTERLOG
	StateManagerStateInfo state_info=this->head;
	while(state_info) {
		if(state_info->state.state == state) {
			break;
		}
		state_info=state_info->next;
	}

EXITLOG
	return state_info;
}
/*************
 * public interface API implement
*************/
StateManagerClass state_manager_new(size_t state_info_num, const state_info_t * state) {
ENTERLOG
	StateManagerClass instance = calloc(1, sizeof(*instance));
	if( !instance ) {
		DEBUG_ERRPRINT("allocate error\n");
		goto err;
	}

	/*set state info into manager*/
	size_t i=0;
	for( i = 0; i < state_info_num; i ++) {
		/*allocate and copy state info*/
		if(state_manager_add_new_state(instance, &state[i]) != STATE_MNG_SUCCESS) {
			goto err;
		}
	}

EXITLOG
	return instance;
err:
	state_manager_free(instance);
	return NULL;
}

int state_manager_update_method(StateManagerClass this, const state_info_t * state) {
ENTERLOG
	if(!this || !state) {
		return STATE_MNG_FAILED;
	}

	int ret = STATE_MNG_SUCCESS;
	StateManagerStateInfo state_info = state_manager_find_state(this, state->state);
	if(state_info) {
		memcpy(&state_info->state, state, sizeof(state_info_t));
	} else {
		ret = state_manager_add_new_state(this, state);
	}
EXITLOG
	return ret;
}

void state_manager_set_state(StateManagerClass this, int state) {
ENTERLOG
	if(!this) {
		return;
	}
	this->current_state = state_manager_find_state(this, state);
EXITLOG
}

int state_manager_get_current_state(StateManagerClass this) {
ENTERLOG
	if(!this) {
		return STATE_MNG_FAILED;
	}

	int current_state = STATE_MNG_FAILED;
	if(this->current_state) {
		current_state = this->current_state->state.state;
	}
EXITLOG
	return current_state;
}
int state_manager_call(StateManagerClass this, void *arg) {
ENTERLOG
	/*fail safe*/
	if(!this) {
		return STATE_MNG_FAILED;
	}

	if(!this->current_state) {
		DEBUG_ERRPRINT("You don't set state\n");
		return STATE_MNG_FAILED;
	}

EXITLOG
	return this->current_state->state.state_method(arg);
}

void state_manager_show(StateManagerClass this) {
ENTERLOG
	if(!this) {
		return;
	}

	printf("-------- Show state table --------\n");
	if(this->current_state) {
		printf("[Current state: %d] [method: %s] \n", this->current_state->state.state, this->current_state->state.name);
	} else {
		printf("[Currently no set state]\n");
	}

	StateManagerStateInfo state_info=this->head;
	while(state_info) {
		printf("\t[state: %d] [method: %s]\n", state_info->state.state, state_info->state.name);
		state_info=state_info->next;
	}
	printf("----------------------------------\n");

EXITLOG
}

void state_manager_free(StateManagerClass this) {
ENTERLOG
	if(!this) {
		return;
	}

	StateManagerStateInfo state_info=(StateManagerStateInfo)dputil_list_pop((DPUtilList)this);
	while(state_info) {
		free(state_info);
		state_info=(StateManagerStateInfo)dputil_list_pop((DPUtilList)this);
	}
EXITLOG
}
