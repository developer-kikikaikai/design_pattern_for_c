/**
 * @file state_manager.h
 * @brief This is API for Sate design pattern
**/
#ifndef STATE_MANAGER_
#define STATE_MANAGER_
#include "dp_define.h"
DP_H_BEGIN

#define STATE_MNG_SUCCESS (0)
#define STATE_MNG_FAILED (-1)

/*! @struct state_info_t
 * @brief state method definition, to know detail, add parameter "name". It is set by STATE_MNG_SET_INFO_INIT or STATE_MNG_SET_INFO macro. Please use it.
*/
typedef struct state_info_t {
	int state;/*! state value related function*/
	char *name;/*! name, function name defined by macro. So please use macro*/
	int (*state_method)(void *arg);/*! state method called at state_manager_call */
} state_info_t;

/*! @def STATE_MNG_SET_INFO_INIT
    @brief Macro to initialize state_info_t. Please use this macro, It set function name of method, so it support to know definition relationsip of state and function.
*/
#define STATE_MNG_SET_INFO_INIT(instate, fname) {.state=(instate), .name=#fname, .state_method = (fname)}
/*! @def STATE_MNG_SET_INFO
    @brief Macro to initialize state_info_t after define state_info_t. Please use this macro, It set function name of method, so it support to know definition relationsip of state and function.
*/
#define STATE_MNG_SET_INFO(info, instate, fname) {(info).state=(instate); (info).name=#fname; (info).state_method = (fname) ; }

/*! @struct state_manager_t
 * @brief StateManager class member definition, detail is defined in C file
*/
struct state_manager_t;
/** @brief StateManager class definition, to management state*/
typedef struct state_manager_t *StateManager;

/**
 * @brief Create StateManager class
 *
 * @param[in] state_info_num num of state_info_t, we can set list of state af this API
 * @param[in] state state_info pinters, please define state_info_t's in some function, and set this pointer in here.
 *            this library change state method by state_manager_set_state, and call method when using state_manager_call
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
StateManager state_manager_new(size_t state_info_num, const state_info_t * state);
/**
 * @brief update method related to state
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @param[in] state info of state. If there is no state, it is add to state list
 * @retval STATE_MNG_SUCCESS success
 * @retval other failed to add
 */
int state_manager_update_method(StateManager this, const state_info_t * state);
/**
 * @brief set state
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @param[in] state state, if there is no state in set list, state is changed to latest order.
 * @return none
*/
void state_manager_set_state(StateManager this, int state);
/**
 * @brief get current state
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @return state value or STATE_MNG_FAILED if you don't set state
*/
int state_manager_get_current_state(StateManager this);
/**
 * @brief call state method
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @param[in] arg argument vakue
 * @retval STATE_MNG_FAILED failed
 * @retval other return value of state_method
*/
int state_manager_call(StateManager this, void *arg);
/**
 * @brief show current state table
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @return none
 */
void state_manager_show(StateManager this);
/**
 * @brief free StateManager class
 *
 * @param[in] this StateManager instance returned at state_manager_new,
 * @return none
 */
void state_manager_free(StateManager this);
DP_H_END
#endif
