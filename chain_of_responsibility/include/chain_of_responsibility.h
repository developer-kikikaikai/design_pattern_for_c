/**
 * @file chain_of_responsibility.h
 * @brief This is API for Chain of Responsibility design pettern class
**/
#ifndef CHAIN_OF_RESPONSIBILITY_H_
#define CHAIN_OF_RESPONSIBILITY_H_
#include "dp_define.h"
DP_H_BEGIN

/*! 
 * @brief chain_api result type
*/
typedef enum {
	CoR_GONEXT,/*!< go to next */
	CoR_RETURN,/*!< exit to call chain_api*/
} cor_result_e;

/*! @struct chain_of_resp_t
 * ChainElementPart class instance definition, which is a part of chain. This member is defined in chain_element.c.
*/
struct chain_element_part;
/** ChainElementPart class definition  */
typedef struct chain_element_part * ChainElementPart;

/*! @struct cor_manager_t
 * @brief Independent Chain of Responsibility manager. The member is private.
 */
struct cor_manager_t;
typedef struct cor_manager_t *CorManager;

/**
 * @brief chain func
 *
 * @param[in] arg input parameter pointer, related to function
 * @param[in] ctx context information registered at cor_add_function
 * @retval CoR_GONEXT -> call next chain_api
 * @retval CoR_RETURN -> exit to call chain_api
 */
typedef cor_result_e (*chain_func)(void *arg, void *ctx);

/**
 * @brief create an independent Chain of Responsibility manager
 * @param[in] is_threadsafe if !=0, protect manager and chain operations for multi-thread use
 * @retval !=NULL manager handle
 * @retval NULL allocation error
 */
CorManager cor_manager_new(int is_threadsafe);

/**
 * @brief free an independent manager and all its chain elements
 * @param[in] manager manager returned by cor_manager_new
 * @note This function does not free ctx registered with chain functions.
 * @note The caller must ensure no other API or callback is using manager while this function runs.
 */
void cor_manager_free(CorManager manager);

/**
 * @brief add a function to an independent manager
 * @param[in] manager manager returned by cor_manager_new
 * @param[in] id key identifying the chain
 * @param[in] func chain callback
 * @param[in] ctx caller-owned callback context
 * @retval !=NULL handle for removing this element
 * @retval NULL invalid input or allocation error
 * @note The manager never frees ctx.
 */
ChainElementPart cor_manager_add_function(CorManager manager, const int id, chain_func func, void *ctx);
/**
 * @brief call a chain in an independent manager
 * @param[in] manager manager returned by cor_manager_new
 * @param[in] id key identifying the chain
 * @param[in] arg argument passed to each chain callback
 * @note Chain callbacks run without an internal lock and may call manager APIs recursively.
 * @note If removal races with a call, a callback already selected for that call may still run.
 *       The caller must keep ctx valid until all concurrent calls have completed.
 */
void cor_manager_call(CorManager manager, const int id, void *arg);
/**
 * @brief remove every element with the specified function from an independent manager
 * @param[in] manager manager returned by cor_manager_new
 * @param[in] id key identifying the chain
 * @param[in] func callback to remove
 * @note The manager does not free callback context data.
 */
void cor_manager_remove_function(CorManager manager, const int id, chain_func func);
/**
 * @brief remove one element from an independent manager
 * @param[in] manager manager returned by cor_manager_new
 * @param[in] id key identifying the chain
 * @param[in] element handle returned by cor_manager_add_function for this manager and id
 * @note Passing an element from another manager/id or an element already removed is invalid.
 * @note The manager does not free callback context data.
 */
void cor_manager_remove_chain_element_part(CorManager manager, const int id, ChainElementPart element);

/**
 * @brief set thredsafe
 *
 * @param [in] is_threadsafe 1 if you want to use threadsafe.
 * @return none
 * @note Call this before the first call to cor_add_function. Calls after the
 *       default manager has been created do not change its configuration.
 */
void cor_set_threadsafe(int is_threadsafe);

/**
 * @brief add to chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] func chain func
 * @param[in] ctx user defined context information
 * @retval !=NULL -> Success to add, if you want to remove element, please keep it.
 * @retval NULL -> Faled to add
 */
ChainElementPart cor_add_function(const int id, chain_func func, void *ctx);

/**
 * @brief call chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] arg input parameter pointer, related to function
 * @return none. If you want to get result, please define input parameter to know result
 * @note Chain callbacks run without an internal lock and may call CoR APIs recursively.
 * @note If removal races with a call, a callback already selected for that call may still run.
 *       The caller must keep ctx valid until all concurrent calls have completed.
 */
void cor_call(const int id, void *arg);

/**
 * @brief remove to chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] func chain api func
 * @return none
 * @note This function remove all functions which is same address. So if you set same function by cor_add_function, all of them will remove.
 * @note This function "NOT" free ctx 
 */
void cor_remove_function(const int id, chain_func func);

/**
 * @brief remove to chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] element chain element returned at cor_add_function
 * @return none
 * @note This function only remove element. So if you want to register same functions, and remove only one element, please use it.
 * @note This function "NOT" free ctx 
 */
void cor_remove_chain_element_part(const int id, ChainElementPart element);

/**
 * @brief clear all list
 *
 * @return none
 * @note The caller must ensure no other default-manager API or callback is
 *       running while this function runs.
 */
void cor_clear(void);
DP_H_END
#endif
