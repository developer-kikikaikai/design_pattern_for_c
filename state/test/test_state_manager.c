#include <stdio.h>
#include "test_state_manager.h"

enum {
	STATE_BEGIN,
	STATE_TESTING,
	STATE_END,
};

/* @name test state API */
/* @{*/
static int test_state_begin(void *arg) {
	int * data = (int *)arg;
	int val = (0x01)<<STATE_BEGIN;
	*data+=val;
	return val;
}

static int test_state_testing(void *arg) {
	int * data = (int *)arg;
	int val = (0x01)<<STATE_TESTING;
	*data+=val;
	return val;
}

static int test_state_end(void *arg) {
	int * data = (int *)arg;
	int val = (0x01)<<STATE_END;
	*data+=val;
	return val;
}

static int test_state_begin_2_testing(void *arg) {
	StateManagerClass manager = (StateManagerClass)arg;
	state_manager_set_state(manager, STATE_TESTING);
	return STATE_BEGIN;
}

static int test_state_testing_2_end(void *arg) {
	StateManagerClass manager = (StateManagerClass)arg;
	state_manager_set_state(manager, STATE_END);
	return STATE_TESTING;
}

static int test_state_end_2_begin(void *arg) {
	StateManagerClass manager = (StateManagerClass)arg;
	state_manager_set_state(manager, STATE_BEGIN);
	return STATE_END;
}
/* }@*/

static int test_state_manager_failsafe() {
	state_info_t state;
	if(!state_manager_update_method(NULL, &state) || !state_manager_update_method((StateManagerClass)&state, NULL) ) {
		printf("####Failed to check error state_manager_update_method\n");
		return -1;
	}

	state_manager_set_state(NULL, STATE_BEGIN);
	if(!state_manager_call(NULL, NULL) ) {
		printf("####Failed to check error state_manager_call\n");
		return -1;
	}

	state_manager_show(NULL);
	state_manager_free(NULL);
	return 0;
}

static int test_state_manager_normally_usage() {
	state_info_t state_info[] = {
		//set state and function by using Macro
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin),
		STATE_MNG_SET_INFO_INIT(STATE_TESTING, test_state_testing),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end),
	};

	StateManagerClass manager = state_manager_new(sizeof(state_info)/sizeof(state_info[0]), state_info);
	if(!manager) {
		printf("####Failed to call state_manager_new\n");
		return -1;
	}

	state_manager_show(manager);

	if(!state_manager_call(manager, NULL)) {
		printf("####Failed to check no state\n");
		return -1;
	}

	int response=0;
	state_manager_set_state(manager, STATE_BEGIN);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_BEGIN || response != ((0x01)<<STATE_BEGIN)) {
		printf("####Failed to call STATE_BEGIN state method\n");
		return -1;
	}

	state_manager_set_state(manager, STATE_TESTING);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_TESTING || response != ((0x01)<<STATE_BEGIN) + ((0x01)<<STATE_TESTING)) {
		printf("####Failed to call STATE_TESTING state method\n");
		return -1;
	}

	state_manager_set_state(manager, STATE_END);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_END || response != ((0x01)<<STATE_BEGIN) + ((0x01)<<STATE_TESTING) + ((0x01)<<STATE_END)) {
		printf("####Failed to call STATE_END state method\n");
		return -1;
	}

	state_manager_show(manager);
	state_manager_free(manager);
	return 0;
}

static int test_state_manager_update_usage() {
	struct state_info_data_t {
		int state;
		int (*state_method)(void *arg);
	} state_info_data[] = {
		{STATE_BEGIN, test_state_begin},
		{STATE_TESTING, test_state_begin},//to check update, change method
		{STATE_END, test_state_end},
	};

	StateManagerClass manager = state_manager_new(0, NULL);
	if(!manager) {
		printf("####Failed to call state_manager_new\n");
		return -1;
	}

	state_manager_show(manager);

	int i=0;
	state_info_t state_info;
	for( i = 0; i < sizeof(state_info_data)/sizeof(state_info_data[0]); i++ ) {
		STATE_MNG_SET_INFO(state_info, state_info_data[i].state, state_info_data[i].state_method);
		if(state_manager_update_method(manager, &state_info)) {
			printf("####Failed to add new state\n");
			return -1;
		}
	}

	state_manager_show(manager);

	STATE_MNG_SET_INFO(state_info, STATE_TESTING, test_state_testing);
	if(state_manager_update_method(manager, &state_info)) {
		printf("####Failed to update state\n");
		return -1;
	}

	int response=0;
	state_manager_set_state(manager, STATE_BEGIN);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_BEGIN || response != ((0x01)<<STATE_BEGIN)) {
		printf("####Failed to call STATE_BEGIN state method\n");
		return -1;
	}

	state_manager_set_state(manager, STATE_TESTING);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_TESTING || response != ((0x01)<<STATE_BEGIN) + ((0x01)<<STATE_TESTING)) {
		printf("####Failed to call STATE_TESTING state method\n");
		return -1;
	}

	state_manager_set_state(manager, STATE_END);
	if(state_manager_call(manager,&response) != (0x01)<<STATE_END || response != ((0x01)<<STATE_BEGIN) + ((0x01)<<STATE_TESTING) + ((0x01)<<STATE_END)) {
		printf("####Failed to call STATE_END state method\n");
		return -1;
	}

	state_manager_show(manager);
	state_manager_free(manager);
	return 0;
}

static int test_state_manager_change_state_in_func() {
	state_info_t state_info[] = {
		//set state and function by using Macro
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_2_testing),
		STATE_MNG_SET_INFO_INIT(STATE_TESTING, test_state_testing_2_end),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_2_begin),
	};
	StateManagerClass manager = state_manager_new(sizeof(state_info)/sizeof(state_info[0]), state_info);
	if(!manager) {
		printf("####Failed to call state_manager_new\n");
		return -1;
	}
	state_manager_set_state(manager, STATE_BEGIN);
	if(state_manager_call(manager, manager) != STATE_BEGIN) {
		printf("####Failed to call begin state func\n");
		return -1;
	}
	if(state_manager_get_current_state(manager) != STATE_TESTING) {
		printf("####Failed to change state\n");
		return -1;
	}

	if(state_manager_call(manager, manager) != STATE_TESTING) {
		printf("####Failed to call testing state func\n");
		return -1;
	}
	if(state_manager_get_current_state(manager) != STATE_END) {
		printf("####Failed to change state\n");
		return -1;
	}

	if(state_manager_call(manager, manager) != STATE_END) {
		printf("####Failed to call testing state func\n");
		return -1;
	}
	if(state_manager_get_current_state(manager) != STATE_BEGIN) {
		printf("####Failed to change state\n");
		return -1;
	}
	state_manager_free(manager);
	return 0;
}

int test_state_manager() {

	if(test_state_manager_failsafe()) {
		printf("Failed to check fail safe\n");
		return -1;
	}

	if(test_state_manager_normally_usage()) {
		printf("Failed to check normally usage\n");
		return -1;
	}

	if(test_state_manager_update_usage()) {
		printf("Failed to check update usage\n");
		return -1;
	}

	if(test_state_manager_change_state_in_func()) {
		printf("Failed to check change_state_in_func\n");
		return -1;
	}

	return 0;
}
