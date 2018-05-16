#include <stdlib.h>
#include "state_manager.h"

struct state_manager_t {
	int data;
};

StateManagerClass state_manager_new(int state_info_num, const state_info_t * state) {
	return NULL;
}

int state_manager_update_method(StateManagerClass this, const state_info_t * state) {
	return 0;
}

void state_manager_set_state(StateManagerClass this, int state) {
}

int state_manager_call(StateManagerClass this, void *arg) {
	return 0;
}

void state_manager_show(StateManagerClass this) {
}

void state_manager_free(StateManagerClass this) {
}
