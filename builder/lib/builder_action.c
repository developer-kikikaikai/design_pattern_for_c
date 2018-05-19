/**
 * @file builder_action.c
 *    @brief      Implement of builder_action, running construct on other thread
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builder_action.h"
#include "dp_debug.h"

/*************
 * public define
*************/

/*! @struct BuilderAction
 * @brief action builder, to call builder by other thread, create class for it
*/
typedef builder_action_parameter_t *BuilderAction;

/*! @name BuilderAction private method *
/* @{ */
/* ! run action */
static void * builder_action_run(void * this);
/* }@ */

/*************
 * for BuilderAction method
*************/
static void * builder_action_run(void * arg) {
	BuilderAction this = (BuilderAction) arg;
	int i=0;
	int ret = LL_BUILDER_SUCCESS;
	//call builder methods
	for( i = 0; i < this->builder_method_cnt; i++) {
		if(this->builder_methods[i]) {
			DEBUG_ERRPRINT("methods[%d] call\n", i);
			ret = this->builder_methods[i](this->initial_parameter);
			if(ret == LL_BUILDER_FAILED) {
				break;
			}
		}
	}

	//call result callback
	if(this->initial_result) {
		this->initial_result(ret);
	}
	free(this);
	pthread_exit(NULL);
	return NULL;
}

/*! @name BuilderAction public method */
/* @{ */
void builder_action_destruct(pthread_t tid) {
	if(0<tid) {
		pthread_join(tid, NULL);
	}
}

pthread_t builder_action_construct(builder_action_parameter_t * parameter) {

	pthread_t tid = 0;
	BuilderAction instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("failed to new BuilderAction\n");
		return -1;
	}

	memcpy(instance, parameter, sizeof(*instance));

	pthread_create(&tid, NULL, builder_action_run, instance);
	//free instance in builder_action_run
	return tid;
}
/* }@ */
