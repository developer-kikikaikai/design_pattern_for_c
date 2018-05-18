/**
 * @file chain_of_responsibility.h
 * @brief This is API for Chain of Responsibility design pettern class
**/
#ifndef CHAIN_OF_RESPONSIBILITY_
#define CHAIN_OF_RESPONSIBILITY_

/*! @name CoR result */
/* @{ */
#define COR_SUCCESS (0)
#define COR_FAILED (-1)
/* }@ */

/*! 
 * @brief chain_api result type
*/
typedef enum {
	CoR_GONEXT,/*! go to next */
	CoR_RETURN,/*! return */
} cor_result_e;

/**
 * @brief chain func
 *
 * @param[in] arg input parameter pointer, related to function
 * @retval CoR_GONEXT -> call next chain_api
 * @retval CoR_GONEXT
 */
typedef cor_result_e (*chain_func)(void *arg);

/**
 * @brief add to chain api
 *
 * @param[in] name key name related to chain api
 * @param[in] func chain func
 * @retval COR_SUCCESS -> Success to add
 * @retval COR_FAILED -> Faled to add
 */
int cor_add_function(const char *name, chain_func func);

/**
 * @brief call chain api
 *
 * @param[in] name key name related to chain api
 * @param[in] arg input parameter pointer, related to function
 * @return none. If you want to get result, please define input parameter to know result
 */
void cor_call(const char *name, void *arg);

/**
 * @brief remove to chain api
 *
 * @param[in] name key name related to chain api
 * @param[in] func chain api func
 * @return none
 */
void cor_remove_function(const char *name, chain_func func);

/**
 * @brief clear all list
 *
 * @return none
 */
void cor_clear(void);
#endif
