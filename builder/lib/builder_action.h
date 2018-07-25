/**
 * @file builder_action.h
 *    @brief      This is API definition of builder action API
 **/
#ifndef BUILDER_ACTION_H_
#define BUILDER_ACTION_H_

#include <pthread.h>
#include "lower_layer_builder.h"

/*! @struct builder_action_parameter_t
 * @brief builder action parameter
*/
typedef struct builder_action_parameter_t {
	void * initial_parameter;
	void (*initial_result)(void * initial_parameter, int result);
	int builder_method_cnt;
	int (** builder_methods)(void * initial_parameter);
} builder_action_parameter_t;

/*! @name BuilderAction public method */
/* @{ */
/*! @brief construct action */
pthread_t builder_action_construct(builder_action_parameter_t * parameter);
int builder_action_construct_sync(builder_action_parameter_t * parameter);
/** @brief deconstruct action */
void builder_action_destruct(pthread_t tid);
/* @} */
#endif
