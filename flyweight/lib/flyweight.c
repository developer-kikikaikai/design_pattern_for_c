/**
 * @file flyweight.c
 *  @brief   Implement of Flyweight design petten library API, defined in flyweight.h
**/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "flyweight.h"
#include "dp_util.h"

/*************
 * public define
*************/
/*! @name Error definition */
/* @{ */
#define FLYWEIGHT_FAILED (-1) /*! error */
#define FLYWEIGHT_SUCCESS (0) /*! success */
/* @} */

/*! @struct flyweight_instance_t
 * @brief instance data definition, instance storaged by list
*/
struct flyweight_instance_t;
typedef struct flyweight_instance_t * FlyweightInstance;

struct flyweight_instance_t {
	FlyweightInstance next;
	void * instance;
};

/* ! new flyweight_instance_t */
static FlyweightInstance flyweight_instance_new(size_t size);	
/* ! free flyweight_instance_t */
static void flyweight_instance_free(FlyweightInstance instance);

/*! @struct class_factory
 * @brief instance data definition.
*/
struct flyweight_factory_t {
	//private member
	flyweight_methods_t methods;/*! methods */
	size_t class_size;/*! size of class */
	FlyweightInstance class_instances; /*! list of instance */
	pthread_mutex_t *lock;/*! lock pointer, if != null, lock at getter and setter */
};

/*! @name public API with FlyweightInstance */
/* @{ */
/*! Check has instance. */
static FlyweightInstance flyweight_factory_get_storaged_instance(FlyweightFactory class_factory, void * constructor_parameter);
/*! allocate. */
static FlyweightInstance flyweight_factory_instance_new(FlyweightFactory class_factory, void *constructor_parameter);
/*! push instance into list. */
static void flyweight_factory_push_instance(FlyweightInstance instance, FlyweightFactory class_factory);
/*! pop instance from list. */
static FlyweightInstance flyweight_factory_pop_instance(FlyweightFactory class_factory);
/*! Getter. */
static FlyweightInstance flyweight_factory_get(FlyweightFactory class_factory, void * constructor_parameter);
/* @} */

/*! @name private API for FlyweightFactory */
/* @{ */
/*! Default constructor. */
static inline void flyweight_class_default_constructor(void *this, size_t size, void *input_parameter);
/*! Default equall operand. */
static inline int flyweight_class_default_equall_operand(void *this, size_t size, void *input_parameter);
/*! Default setter. */
static inline int flyweight_class_default_setter(void *this, size_t size, void *input_parameter);
/*! Set methods. */
static void flyweight_class_set_methods(FlyweightMethodsIF methods, FlyweightFactory instance);
#define FLYWEIGHT_CLASS_LOCK(instance) DPUTIL_LOCK(instance->lock)
#define FLYWEIGHT_CLASS_UNLOCK DPUTIL_UNLOCK
/* @} */

/*************
 * for FlyweightInstance API
*************/
static FlyweightInstance flyweight_instance_new(size_t size) {

	FlyweightInstance instance = (FlyweightInstance )calloc(1, sizeof(struct flyweight_instance_t) );
	if( !instance ) {
		return NULL;	
	}

	//allocate
	instance->instance = calloc(1, size);
	if( !instance->instance ) {
		free(instance);
		return NULL;
	}

	return instance;
}

static void flyweight_instance_free(FlyweightInstance instance) {
	if( instance && instance->instance ) {
		free(instance->instance);
	}
	free(instance);
}

/*************
 * for FlyweightFactory API
*************/
static FlyweightInstance flyweight_factory_get_storaged_instance(FlyweightFactory class_factory, void * constructor_parameter) {
	//fail safe, if equall_operand == NULL, there is no case to store same instance
	if( !class_factory->methods.equall_operand ) {
		DEBUG_ERRPRINT("operand is NULL\n");
		return NULL;
	}

	FlyweightInstance instance = class_factory->class_instances;
	while(instance) {
		if( class_factory->methods.equall_operand(instance->instance, class_factory->class_size, constructor_parameter) ) {
			DEBUG_ERRPRINT("address %p instance is same\n", instance);
			break;
		}
		instance=instance->next;
	}

	return instance;
}

static FlyweightInstance flyweight_factory_instance_new(FlyweightFactory class_factory, void *constructor_parameter) {
	FlyweightInstance instance = flyweight_instance_new(class_factory->class_size);
	if( !instance ) {
		return NULL;
	}

	if(class_factory->methods.constructor) {
		class_factory->methods.constructor(instance->instance, class_factory->class_size, constructor_parameter);
	}
	return instance;
}

static void flyweight_factory_push_instance(FlyweightInstance instance, FlyweightFactory class_factory) {
	instance->next = class_factory->class_instances;
	class_factory->class_instances = instance;
}

static FlyweightInstance flyweight_factory_pop_instance(FlyweightFactory class_factory) {
	FlyweightInstance instance = class_factory->class_instances;
	if( instance ) {
		class_factory->class_instances = class_factory->class_instances->next;
	}

	return instance;
}

static FlyweightInstance flyweight_factory_get(FlyweightFactory class_factory, void * constructor_parameter) {
	FlyweightInstance instance = flyweight_factory_get_storaged_instance(class_factory, constructor_parameter);
	if( instance ) {
		//if already keep method, return it;
		return instance;
	}

	//allocate
	instance = flyweight_factory_instance_new(class_factory, constructor_parameter);
	if(instance) {
		flyweight_factory_push_instance(instance, class_factory);
	}

	return instance;
}

/*private API*/
/*! Default constructor. */
static inline void flyweight_class_default_constructor(void *this, size_t size, void *input_parameter) {
	if(input_parameter==NULL) {
		return;
	}

	memcpy(this, input_parameter, size);
}

/*! Default equall operand. */
static inline int flyweight_class_default_equall_operand(void *this, size_t size, void *input_parameter) {
	if(input_parameter==NULL) {
		return 0;
	}

	return (memcmp(this, input_parameter, size) == 0);
}

/*! Default setter. */
static inline int flyweight_class_default_setter(void *this, size_t size, void *input_parameter) {
	if(input_parameter==NULL) {
		return FLYWEIGHT_FAILED;
	}

	memcpy(this, input_parameter, size);
	return FLYWEIGHT_SUCCESS;
}

/*! Set methods. */
static void flyweight_class_set_methods(FlyweightMethodsIF methods, FlyweightFactory class_factory) {
	if(!methods) {
		//set default. destrctor is NULL
		class_factory->methods.constructor = flyweight_class_default_constructor;
		class_factory->methods.equall_operand = flyweight_class_default_equall_operand;
		class_factory->methods.setter = flyweight_class_default_setter;
	} else {
		memcpy((void *)&class_factory->methods, (void *)methods, sizeof(class_factory->methods));
	}
}

/*************
 * public interface API implement
*************/
FlyweightFactory flyweight_factory_new(size_t class_size, int is_threadsafe, FlyweightMethodsIF methods) {

	if(class_size<=0) {
		return NULL;
	}

	//allocate class_factory
	FlyweightFactory class_factory = (FlyweightFactory) calloc(1, sizeof(*class_factory));
	if( !class_factory ) {
		DEBUG_ERRPRINT("calloc instance list error:%s\n", strerror(errno));
		return NULL;
	}

	//when use threadsafe, allocate mutex lock
	if( is_threadsafe ) {
		class_factory->lock = (pthread_mutex_t *) calloc(1, sizeof(pthread_mutex_t));
		if( !class_factory->lock ) {
			DEBUG_ERRPRINT("class instance lock error:%s\n", strerror(errno));
			free(class_factory);
			return NULL;
		}

		pthread_mutex_init(class_factory->lock, NULL);
	}

	class_factory->class_size = class_size;
	//set methods
	flyweight_class_set_methods(methods, class_factory);

	return class_factory;
}

void * flyweight_get(FlyweightFactory this, void * constructor_parameter) {
	//fail safe
	if(!this) {
		return NULL;
	}

	void *ret=NULL;
	FLYWEIGHT_CLASS_LOCK(this);

	//get instance
	FlyweightInstance instance = flyweight_factory_get(this, constructor_parameter);
	if( instance ) {
		ret = instance->instance;
	}

	FLYWEIGHT_CLASS_UNLOCK

	return ret;
}

int flyweight_set(FlyweightFactory this, void * constructor_parameter, void * data, int (*setter)(void *this, size_t size, void *input_parameter)) {

	//fail safe
	if(!this) {
		return FLYWEIGHT_FAILED;
	}

	int ret=FLYWEIGHT_FAILED;

	FLYWEIGHT_CLASS_LOCK(this);

	//get and set instance
	FlyweightInstance instance = flyweight_factory_get(this, constructor_parameter);

	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance! handle[%p]\n", this );
	} else {
		if( setter ) {
			ret = setter(instance->instance, this->class_size, data);
		} else if ( this->methods.setter ) {
			ret = this->methods.setter(instance->instance, this->class_size, data);
		}
	}

	FLYWEIGHT_CLASS_UNLOCK

	return ret;
}

void flyweight_factory_free(FlyweightFactory this) {
	//fail safe
	if(!this) {
		return;
	}

	pthread_mutex_t *keep_mutex_for_free=this->lock;

	FLYWEIGHT_CLASS_LOCK(this);

	//pop and free
	FlyweightInstance instance=NULL;
	while( (instance = flyweight_factory_pop_instance(this)) != NULL ) {
		//call destcuctor
		if(this->methods.destructor) {
			this->methods.destructor(instance->instance);
		}
		flyweight_instance_free(instance);
	}
	free(this);

	//initialize
	FLYWEIGHT_CLASS_UNLOCK

	//free API care NULL
	free(keep_mutex_for_free);
}

