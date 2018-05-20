/**
 * @file lower_layer_director.h
 * @brief This is API for director class action as  design petten
 *        In this case, Builder interface is included by conf file, and interface implement class is dynamic library.
 *        Please see conf/sample.conf
**/

#ifndef LOWER_LAYER_DIRECTOR_H_
#define LOWER_LAYER_DIRECTOR_H_
#include "lower_layer_builder.h"

/*! @struct director_t
 * @brief director class member definition
*/
struct director_t;
/** @brief Typedef class Director, member is defined in struct director_t */
typedef struct director_t *Director;

/*! @struct lower_layer_director_t
 * @brief LowerLayerDirector class member definition
*/
struct lower_layer_director_t {
	void * lower_layer_interface;/*!< interface which has lower layer(builder). If no interface, it is NULL*/
	Director director;/*!< director class instance */
};

/** @brief LowerLayerDirector class definition */
typedef struct lower_layer_director_t lower_layer_director_t, *LowerLayerDirector;

/**
 * @brief director new
 *
 * @param[in] builder_lib_name library name implement builder interface
 * @param[in] builder_interface_conf  conf file write interface definition
 * @retval !=NULL  LowerLayerDirector class instance.
 * @retval NULL error
 */
LowerLayerDirector lower_layer_director_new(char * builder_lib_name, char * builder_interface_conf);

/**
 * @brief director cconstruct
 *
 * @param[in] director  LowerLayerDirector class instance returned at lower_layer_director_construct
 * @param[in] initial_parameter initialize parameter if you have
 * @param[in] initial_result initialize callback, result is in here
 * @return none
 * @note please keep initial_parameter on static field (define static or allocate memory)
 */
void lower_layer_director_construct(LowerLayerDirector director, void * initial_parameter, void (*initial_result)(int result));

/**
 * @brief director denstruct
 *
 * @param[in] director  LowerLayerDirector class instance returned at lower_layer_director_construct
 */
void lower_layer_director_free(LowerLayerDirector director);
#endif

