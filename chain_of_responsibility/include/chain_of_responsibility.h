/**
 * @file chain_of_responsibility.h
 * @brief This is API for Chain of Responsibility design pettern class
**/
#ifndef CHAIN_OF_RESPONSIBILITY_H_
#define CHAIN_OF_RESPONSIBILITY_H_

/*! @name CoR result */
/* @{ */
#define COR_SUCCESS (0)
#define COR_FAILED (-1)
/* @} */

/*! 
 * @brief chain_api result type
*/
typedef enum {
	CoR_GONEXT,/*!< go to next */
	CoR_RETURN,/*!< exit to call chain_api*/
} cor_result_e;

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
 * @brief set thredsafe
 *
 * @param [in] is_threadsafe 1 if you want to use threadsafe.
 * @return none
 */
void cor_set_threadsafe(int is_threadsafe);

/**
 * @brief add to chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] func chain func
 * @param[in] ctx user defined context information
 * @retval COR_SUCCESS -> Success to add
 * @retval COR_FAILED -> Faled to add
 */
int cor_add_function(const int id, chain_func func, void *ctx);

/**
 * @brief call chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] arg input parameter pointer, related to function
 * @return none. If you want to get result, please define input parameter to know result
 */
void cor_call(const int id, void *arg);

/**
 * @brief remove to chain api
 *
 * @param[in] id key id related to chain api
 * @param[in] func chain api func
 * @return none
 */
void cor_remove_function(const int id, chain_func func);

/**
 * @brief clear all list
 *
 * @return none
 */
void cor_clear(void);
#endif
