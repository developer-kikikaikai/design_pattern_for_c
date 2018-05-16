#ifndef STATE_MANAGER_
#define STATE_MANAGER_

//set method
typedef struct state_info_s {
	int state;
	char *name;
	int (*state_method)(void *arg);
} state_info_t;

#define STATE_MNG_SET_INFO_INIT(instate, fname) {.state=(instate), .name=#fname, .state_method = (fname)}
#define STATE_MNG_SET_INFO(info, instate, fname) {(info).state=(instate); (info).name=#fname; (info).state_method = (fname) ; }

struct state_manager_t;
typedef struct state_manager_t *StateManagerClass;

StateManagerClass state_manager_new(int state_info_num, const state_info_t * state);
int state_manager_update_method(StateManagerClass this, const state_info_t * state);
void state_manager_set_state(StateManagerClass this, int state);
int state_manager_call(StateManagerClass this, void *arg);
void state_manager_show(StateManagerClass this);
void state_manager_free(StateManagerClass this);
#endif
