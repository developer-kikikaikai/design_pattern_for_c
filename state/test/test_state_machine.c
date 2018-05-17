#include <stdio.h>
#include <unistd.h>
#include "test_state_machine.h"

enum {
	STATE_BEGIN,
	STATE_END,
	STATE_MAX,
};

enum {
	EVENT_BEGIN,
	EVENT_END,
	EVENT_MAX,
};


/* @name test state API */
/* @{*/
static int get_value(int event, int state) {
	usleep(1000);
	int val = (0x01)<<state;
	val += (0x1)<<(STATE_MAX+event);
	return val;
}

static int test_state_begin_event_begin(void *arg) {
	usleep(1000);
	int * data = (int *)arg;
	*data = get_value(EVENT_BEGIN, STATE_BEGIN);
	return *data;
}

static int test_state_end_event_begin(void *arg) {
	usleep(1000);
	int * data = (int *)arg;
	*data = get_value(EVENT_BEGIN, STATE_END);
	return *data;
}

static int test_state_begin_event_end(void *arg) {
	usleep(1000);
	int * data = (int *)arg;
	*data = get_value(EVENT_END, STATE_BEGIN);
	return *data;
}

static int test_state_end_event_end(void *arg) {
	usleep(1000);
	int * data = (int *)arg;
	*data = get_value(EVENT_END, STATE_END);
	return *data;
}
/* }@*/

static int test_state_machine_failsafe() {
	state_event_info_t event_info;
	if(!state_machine_update_machine(NULL, &event_info) || !state_machine_update_machine((StateMachineClass)&event_info, NULL) ) {
		printf("####Failed to check error state_machine_update_machine\n");
		return -1;
	}

	state_machine_set_state(NULL, STATE_BEGIN);
	if(!state_machine_call_event(NULL, EVENT_BEGIN, NULL, NULL)) {
		printf("####Failed to check error state_machine_call_event\n");
		return -1;
	}

	state_machine_show(NULL);
	state_machine_free(NULL);
	return 0;
}

static int test_state_machine_normally_usage() {
	state_info_t state_for_beginevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_begin),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_begin),
	};

	state_info_t state_for_endevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_end),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_end),
	};

	state_event_info_t event_info[] = {
		{EVENT_BEGIN, sizeof(state_for_beginevent)/sizeof(state_for_beginevent[0]), state_for_beginevent},
		{EVENT_END, sizeof(state_for_endevent)/sizeof(state_for_endevent[0]), state_for_endevent},
	};

	StateMachineClass state_machine = state_machine_new(sizeof(event_info)/sizeof(event_info[0]), event_info, 0);
	if(!state_machine) {
		printf("####Failed to call state_machine_new\n");
		return -1;
	}

	state_machine_show(state_machine);

	if(!state_machine_call_event(state_machine, EVENT_BEGIN, NULL, NULL)) {
		printf("####Failed to check no state\n");
		return -1;
	}

	int response=0;
	int expected_value=0;

	//state begin
	int state, event;
	for(state = STATE_BEGIN; state < STATE_MAX; state++) {
		state_machine_set_state(state_machine, state);
		state_machine_show(state_machine);
		for(event = EVENT_BEGIN; event < EVENT_MAX; event++ ) {
			expected_value = get_value(event, state);
			if(state_machine_call_event(state_machine, event, &response, NULL) != expected_value || response != expected_value) {
				printf("####Failed to call event %d: state %d method(response=%d. expected_value=%d)\n", event, state, response, expected_value);
				return -1;
			}
		}
	}

	state_machine_free(state_machine);
	return 0;
}

static int test_state_machine_update_usage() {
	state_info_t state_for_beginevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_begin),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_begin),
	};

	state_info_t state_for_endevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_begin),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_begin),
	};

	state_info_t state_for_endevent_update[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_end),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_end),
	};

	state_event_info_t event_info[] = {
		{EVENT_BEGIN, sizeof(state_for_beginevent)/sizeof(state_for_beginevent[0]), state_for_beginevent},
		{EVENT_END, sizeof(state_for_endevent)/sizeof(state_for_endevent[0]), state_for_endevent},
	};

	StateMachineClass state_machine = state_machine_new(sizeof(event_info)/sizeof(event_info[0]), event_info, 0);
	if(!state_machine) {
		printf("####Failed to call state_machine_new\n");
		return -1;
	}

	state_machine_show(state_machine);

	state_event_info_t event_info_update = {EVENT_END, sizeof(state_for_endevent_update)/sizeof(state_for_endevent_update[0]), state_for_endevent_update};
	if(state_machine_update_machine(state_machine, &event_info_update)) {
		printf("####Failed to update event info\n");
		return -1;
	}

	int response=0;
	int expected_value=0;

	//state begin
	int state, event;
	for(state = STATE_BEGIN; state < STATE_MAX; state++) {
		state_machine_set_state(state_machine, state);
		state_machine_show(state_machine);
		for(event = EVENT_BEGIN; event < EVENT_MAX; event++ ) {
			expected_value = get_value(event, state);
			if(state_machine_call_event(state_machine, event, &response, NULL) != expected_value || response != expected_value) {
				printf("####Failed to call event %d: state %d method(response=%d. expected_value=%d)\n", event, state, response, expected_value);
				return -1;
			}
		}
	}

	state_machine_free(state_machine);
	return 0;
}

static int test_state_machine_multi_thread() {
	state_info_t state_for_beginevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_begin),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_begin),
	};

	state_info_t state_for_endevent[] = {
		STATE_MNG_SET_INFO_INIT(STATE_BEGIN, test_state_begin_event_end),
		STATE_MNG_SET_INFO_INIT(STATE_END, test_state_end_event_end),
	};

	state_event_info_t event_info[] = {
		{EVENT_BEGIN, sizeof(state_for_beginevent)/sizeof(state_for_beginevent[0]), state_for_beginevent},
		{EVENT_END, sizeof(state_for_endevent)/sizeof(state_for_endevent[0]), state_for_endevent},
	};

	//run by multi thread
	StateMachineClass state_machine = state_machine_new(sizeof(event_info)/sizeof(event_info[0]), event_info, 1);
	if(!state_machine) {
		printf("####Failed to call state_machine_new\n");
		return -1;
	}

	state_machine_show(state_machine);

	int response=0;
	int expected_value=0;

	//state begin
	int state, event;
	for(state = STATE_BEGIN; state < STATE_MAX; state++) {
		state_machine_set_state(state_machine, state);
		state_machine_show(state_machine);
		for(event = EVENT_BEGIN; event < EVENT_MAX; event++ ) {
			expected_value = get_value(event, state);
			if(state_machine_call_event(state_machine, event, &response, NULL) != STATE_MNG_SUCCESS) {
				printf("####Failed to call event %d: state %d\n", event, state);
				return -1;
			}
			//wait to call callback
			sleep(1);
			if(response != expected_value) {
				printf("####Failed to call event %d: state %d method(response=%d. expected_value=%d)\n", event, state, response, expected_value);
				return -1;
			}
		}
	}

	state_machine_free(state_machine);
	return 0;
}

int test_state_machine() {

	if(test_state_machine_failsafe()) {
		printf("Failed to check fail safe\n");
		return -1;
	}

	if(test_state_machine_normally_usage()) {
		printf("Failed to check normally usage\n");
		return -1;
	}

	if(test_state_machine_update_usage()) {
		printf("Failed to check update usage\n");
		return -1;
	}

	if(test_state_machine_multi_thread()) {
		printf("Failed to check multi thread\n");
		return -1;
	}

	return 0;
}
