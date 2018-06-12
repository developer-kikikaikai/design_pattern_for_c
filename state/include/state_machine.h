/**
 * @file state_machine.h
 * @brief This is API for Sate machine
**/
#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_
#include "state_manager.h"
#include "event_threadpool.h"

/*! @struct state_event_info_t
 * @brief event ID and related state functions
*/
typedef struct state_event_info_t {
	int event;/*!< event event id */
	size_t state_num;/*!< state num*/
	state_info_t *state_infos;/*!< state list, please see state_manager.h defition*/
} state_event_info_t;

/*! @struct state_machine_t
 * @brief StateMachine class member definition
*/
struct state_machine_t;	
/** @brief StateMachine class definition */
typedef struct state_machine_t *StateMachine;

typedef struct state_machine_info {
	StateMachine state_machine;
	int thread_num;
} state_machine_info_t,  *StateMachineInfo;

/**
 * @brief Create StateMachineInfo class
 * @param[in] event_num event size
 * @param[in] event_infos list of event state data
 * @param[in] threadpool event threadpool instance by creating event_threadpool API if you want to use state machine in other threads
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
StateMachineInfo state_machine_new(size_t event_num, const state_event_info_t * event_infos, EventTPoolManager threadpool );
/**
 * @brief update sate
 *
 * @param[in] this StateMachineInfo class instance returned at state_machine_new
 * @param[in] event_info update list of event state data.
 * @retval STATE_MNG_SUCCESS success
 * @retval other failed
*/
int state_machine_update_machine(StateMachineInfo this, const state_event_info_t * event_info);
/**
 * @brief set state
 *
 * @param[in] this StateMachineInfo class instance returned at state_machine_new
 * @param[in] state update state, if there is no state in set list, state is changed to latest order.
 * @return none
 */
void state_machine_set_state(StateMachineInfo this, int state);
/**
 * @brief call event trigger
 *
 * @param[in] this StateMachineInfo class instance returned at state_machine_new
 * @param[in] event event id related to this function
 * @param[in] arg event argument
 * @param[in] response response callback method. If you set is_multithread=true , you must set this response callback,
 * @retval return value of method if you set by single thread mode
 * @retval STATE_MNG_SUCCESS and result is in callback you set callback if you set by multi thread mode.
 */
int state_machine_call_event(StateMachineInfo this, int event, void *arg, void (*response)(int result));

/*! call state method directry, if you want to call state_machine_call_event in the state_machine API, please call it.
 *
 * @param[in] this StateMachine class instance returned at state_machine_new
 * @param[in] event event id related to this function
 * @param[in] arg event argument
 * @retval return value of method
 * @retval STATE_MNG_SUCCESS and result is in callback you set callback if you set by multi thread mode.
 */
int state_machine_call_event_directry(StateMachineInfo this, int event, void *arg);

/**
 * @brief set state
 *
 * @param[in] this StateMachine class instance returned at state_machine_new
 * @return none
 */
void state_machine_show(StateMachineInfo this);
/**
 * @brief free StateMachine class 
 *
 * @param[in] this StateMachine class instance returned at state_machine_new
 * @return none
 */
void state_machine_free(StateMachineInfo this);
#endif
