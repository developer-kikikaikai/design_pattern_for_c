#ifndef LOWER_LAYER_DIRECTOR_
#define LOWER_LAYER_DIRECTOR_
/**
 * @brief This is API for director class action as  design petten
 *        In this case, Builder interface is included by conf file, and interface implement class is dynamic library.
 *        Please see conf/sample.conf
**/

#include "lower_layer_builder.h"

/*! @struct 
 * @brief flyweight class method definition
*/
typedef struct _lower_layer_director_s {
	LowerLayerInterface lower_layer_interface;/*interface which has lower layer(builder). If no interface, it is NULL*/
	void * director;/* director class pointer */
} lower_layer_director_t, *LowerLayerDirector;

/**
 * @brief director new
 *
 * @param[in] builder_lib_name library name implement builder interface
 * @param[in] builder_interface_conf  conf file write interface definition
 * @retval !=NULL  this handle, and lower interface class if lower library has it.
 * @retval NULL error
 */
LowerLayerDirector lower_layer_director_new(char * builder_lib_name, char * builder_interface_conf);

/**
 * @brief director cconstruct
 *
 * @param[in] handle  handle returned at lower_layer_director_construct
 * @param[in] initial_parameter initialize parameter if you have
 * @param[in] initial_resultr initialize callback, result is in here
 * @return none
 * @note please keep initial_parameter on static field (define static or allocate memory)
 */
void lower_layer_director_construct(LowerLayerDirector director, void * initial_parameter, void (*initial_result)(int result));

/**
 * @brief director denstruct
 *
 * @param[in] handle  handle returned at lower_layer_director_construct
 */
void lower_layer_director_free(LowerLayerDirector director);
#endif

