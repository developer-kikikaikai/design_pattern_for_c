/**
 * @file lower_layer_director.c
 *    @brief      Implement of lower_layer_director (related to builder design petten) library API, defined in flyweight.h
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "director.h"
#include "lower_layer_builder.h"
#include "dp_debug.h"

/*************
 * public interface API implement
*************/
LowerLayerDirector lower_layer_director_new(char * builder_lib_name, char * builder_interface_conf) {
	LowerLayerDirector instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->director = director_new(builder_lib_name, builder_interface_conf);
	if(!instance->director) {
		DEBUG_ERRPRINT("failed to new director\n");
		goto err;
	}

	instance->lower_layer_interface = director_interface_class_new(instance->director);
	return instance;
err:
	lower_layer_director_free(instance);
	return NULL;
}

void lower_layer_director_construct(LowerLayerDirector this, void * initial_parameter, void (*initial_result)(int result)) {
	director_construct(this->director, initial_parameter, initial_result);
}

void lower_layer_director_free(LowerLayerDirector this) {
	if(!this) {
		return;
	}

	Director director =  this->director;

	//destruct
	director_destruct(director);

	//interface free
	director_interface_class_free(director, this->lower_layer_interface);

	//director free
	director_free(director);
	free(this);
}
