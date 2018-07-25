/**
 * @file director.h
 *    @brief      This is API definition of Director class
**/
#ifndef DIRECTOR_H_
#define DIRECTOR_H_
#include "lower_layer_director.h"

/*! @name DirectorClass public method */
/* @{ */
/*! @brief director new */
Director director_new(char * builder_lib_name, char * builder_interface_conf);
/*! @brief director construct */
int director_construct(Director director, void * initial_parameter, void (*initial_result)(void * initial_parameter, int result));
/*! @brief director destruct */
void director_destruct(Director director);
/*! @brief interface class new */
void * director_interface_class_new(Director this);
/*! @brief interface class free */
void director_interface_class_free(Director this, void * instance);
/*! @brief director free */
void director_free(Director this);
/* }@ */
#endif
