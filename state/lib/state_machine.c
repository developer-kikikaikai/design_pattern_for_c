#include <stdlib.h>
#include "state_machine.h"
struct state_machine_t {
	int data;
};

StateMachineClass state_machine_new(int event_num, const state_event_info_t * event_state, int is_multithread) {
	return NULL;
}

int state_machine_update_machine(StateMachineClass this, const state_event_info_t * state) {
	return 0;
}

void state_machine_set_state(StateMachineClass this, int state) {
	return;
}

int state_machine_call_event(StateMachineClass this, int event, void *arg, void (*response)(int result)) {
	return 0;
}

void state_machine_show(StateMachineClass this) {
	return;
}

void state_machine_free(StateMachineClass this) {
	return;
}
