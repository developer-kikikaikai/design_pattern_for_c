/**
 * @file chain_of_responsibility.h
 * @brief This is API for Chain of Responsibility design pettern class
**/
#ifndef CHAIN_OF_RESPONSIBILITY_H_
#define CHAIN_OF_RESPONSIBILITY_H_

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
 */
void cor_clear(void);
#endif
