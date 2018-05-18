/**
 * @file state_machine.h
 * @brief This is API for Sate machine
**/
#ifndef STATE_MACHINE_
#define STATE_MACHINE_
#include "state_manager.h"

/*! @struct state_event_info_t
 * @brief event ID and related state functions
*/
 typedef struct state_event_info_t {
	int event;/*! event event id */
	size_t state_num;/*! state num*/
	state_info_t *state_infos;/*! state list, please see state_manager.h defition*/
} state_event_info_t;

struct state_machine_t;
typedef struct state_machine_t *StateMachineClass;


/**
 * @brief define class for state machine
 * @param[in] event_num event size
 * @param[in] event_infos list of event state data
 * @param[in] is_multithread if you want to run function on otherthread, please set 1.
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
StateMachineClass state_machine_new(size_t event_num, const state_event_info_t * event_infos, int is_multithread);
/**
 * @brief update sate
 *
 * @param[in] this class handle returned at state_machine_new
 * @param[in] event_info update list of event state data.
 * @retval STATE_MNG_SUCCESS success
 * @retval other failed
*/
int state_machine_update_machine(StateMachineClass this, const state_event_info_t * event_info);
/**
 * @brief set state
 *
 * @param[in] this class handle returned at state_manager_new
 * @param[in] state update state, if there is no state in set list, state is changed to latest order.
 * @return none
 */
void state_machine_set_state(StateMachineClass this, int state);
/**
 * @brief call event trigger
 *
 * @param[in] this class handle returned at state_manager_new
 * @param[in] event event id related to this function
 * @param[in] arg event argument
 * @param[in] response response callback method. If you set is_multithread=true , you must set this response callback,
 * @retval return value of method if you set by single thread mode
 * @retval STATE_MNG_SUCCESS and result is in callback you set callback if you set by multi thread mode.
 */
int state_machine_call_event(StateMachineClass this, int event, void *arg, void (*response)(int result));
/**
 * @brief set state
 *
 * @param[in] this class handle returned at state_manager_new
 * @return none
 */
void state_machine_show(StateMachineClass this);
/**
 * @brief set state
 *
 * @param[in] this class handle returned at state_manager_new
 * @return none
 */
void state_machine_free(StateMachineClass this);
#endif
