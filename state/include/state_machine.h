#ifndef STATE_MACHINE_
#define STATE_MACHINE_

#include "state_manager.h"

//set method
typedef struct state_event_info_t {
	int event;
	int state_num;
	state_info_t *state_infos;
} state_event_info_t;

struct state_machine_t;
typedef struct state_machine_t *StateMachineClass;

StateMachineClass state_machine_new(int event_num, const state_event_info_t * event_state, int is_multithread);
int state_machine_update_machine(StateMachineClass this, const state_event_info_t * state);
void state_machine_set_state(StateMachineClass this, int state);
int state_machine_call_event(StateMachineClass this, int event, void *arg, void (*response)(int result));
void state_machine_show(StateMachineClass this);
void state_machine_free(StateMachineClass this);
#endif
