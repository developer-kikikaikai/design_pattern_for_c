/**
 *    @brief      Implement of Flyweight design petten library API, defined in flyweight.h
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

/*! @struct flyweight_instance_s
 * @brief instance data definition, instance storaged by list
*/
struct flyweight_instance_s {
	struct flyweight_instance_s * next;
	void * instance;
};

/* ! new  flyweight_instance_s */
static struct flyweight_instance_s * flyweight_instance_new(size_t size);	
/* ! free flyweight_instance_s */
static void flyweight_instance_free(struct flyweight_instance_s * instance);

/*! @struct class_factory
 * @brief instance data definition.
*/
struct flyweight_class_factory_s {
	//private member
	struct flyweight_class_methods_s methods;/*! methods */
	size_t class_size;/*! size of class */
	struct flyweight_instance_s *class_instances; /*! list of instance */
	pthread_mutex_t *lock;/*! lock pointer, if != null, lock at getter and setter */
};

/*! @name public API for flyweight_instance_s */
/* @{ */
/*! Check has instance. */
static struct flyweight_instance_s * flyweight_factory_get_storaged_instance(ClassHandle class_factory, void * constructor_parameter);
/*! allocate. */
static struct flyweight_instance_s * flyweight_factory_instance_new(ClassHandle class_factory, void *constructor_parameter);
/*! push instance into list. */
static void flyweight_factory_push_instance(struct flyweight_instance_s * instance, ClassHandle class_factory);
/*! pop instance from list. */
static struct flyweight_instance_s * flyweight_factory_pop_instance(ClassHandle class_factory);
/*! Getter. */
static struct flyweight_instance_s * flyweight_factory_get(ClassHandle class_factory, void * constructor_parameter);
/* @} */

/*! @name private API for flyweight_class_factory_s */
/* @{ */
/*! Default constructor. */
static inline void flyweight_class_default_constructor(void *this, size_t size, void *input_parameter);
/*! Default equall operand. */
static inline int flyweight_class_default_equall_operand(void *this, size_t size, void *input_parameter);
/*! Default setter. */
static inline int flyweight_class_default_setter(void *this, size_t size, void *input_parameter);
/*! Set methods. */
static void flyweight_class_set_methods(struct flyweight_class_methods_s *methods, ClassHandle instance);
#define FLYWEIGHT_CLASS_LOCK(instance) DPUTIL_LOCK(instance->lock)
#define FLYWEIGHT_CLASS_UNLOCK DPUTIL_UNLOCK
/* @} */

/*************
 * for flyweight_instance_s API
*************/
static struct flyweight_instance_s * flyweight_instance_new(size_t size) {
ENTERLOG

	struct flyweight_instance_s * instance = (struct flyweight_instance_s *)calloc(1, sizeof(struct flyweight_instance_s) );
	if( !instance ) {
		return NULL;	
	}

	//allocate
	instance->instance = calloc(1, size);
	if( !instance->instance ) {
		free(instance);
		return NULL;
	}

EXITLOG
	return instance;
}

static void flyweight_instance_free(struct flyweight_instance_s * instance) {
ENTERLOG
	if( instance && instance->instance ) {
		free(instance->instance);
	}
	free(instance);
EXITLOG
}

/*************
 * for flyweight_class_factory_s API
*************/
static struct flyweight_instance_s * flyweight_factory_get_storaged_instance(ClassHandle class_factory, void * constructor_parameter) {
ENTERLOG
	//fail safe, if equall_operand == NULL, there is no case to store same instance
	if( !class_factory->methods.equall_operand ) {
		DEBUG_ERRPRINT("operand is NULL\n");
		return NULL;
	}

	struct flyweight_instance_s * instance=class_factory->class_instances;
	while(instance) {
		if( class_factory->methods.equall_operand(instance->instance, class_factory->class_size, constructor_parameter) ) {
			DEBUG_ERRPRINT("address %p instance is same\n", instance);
			break;
		}
		instance=instance->next;
	}
EXITLOG

	return instance;
}

static struct flyweight_instance_s * flyweight_factory_instance_new(ClassHandle class_factory, void *constructor_parameter) {
ENTERLOG
	struct flyweight_instance_s * instance = flyweight_instance_new(class_factory->class_size);
	if( !instance ) {
		return NULL;
	}

	if(class_factory->methods.constructor) {
		class_factory->methods.constructor(instance->instance, class_factory->class_size, constructor_parameter);
	}
EXITLOG
	return instance;
}

static void flyweight_factory_push_instance(struct flyweight_instance_s * instance, ClassHandle class_factory) {
ENTERLOG
	instance->next = class_factory->class_instances;
	class_factory->class_instances = instance;
EXITLOG
}

static struct flyweight_instance_s * flyweight_factory_pop_instance(ClassHandle class_factory) {
ENTERLOG
	struct flyweight_instance_s * instance = class_factory->class_instances;
	if( instance ) {
		class_factory->class_instances = class_factory->class_instances->next;
	}
EXITLOG

	return instance;
}

static struct flyweight_instance_s * flyweight_factory_get(ClassHandle class_factory, void * constructor_parameter) {
ENTERLOG
	struct flyweight_instance_s * instance = flyweight_factory_get_storaged_instance(class_factory, constructor_parameter);
	if( instance ) {
		//if already keep method, return it;
		return instance;
	}

	//allocate
	instance = flyweight_factory_instance_new(class_factory, constructor_parameter);
	if(instance) {
		flyweight_factory_push_instance(instance, class_factory);
	}
EXITLOG

	return instance;
}

/*private API*/
/*! Default constructor. */
static inline void flyweight_class_default_constructor(void *this, size_t size, void *input_parameter) {
ENTERLOG
	if(input_parameter==NULL) {
		return;
	}

	memcpy(this, input_parameter, size);
}

/*! Default equall operand. */
static inline int flyweight_class_default_equall_operand(void *this, size_t size, void *input_parameter) {
ENTERLOG
	if(input_parameter==NULL) {
		return 0;
	}

	return (memcmp(this, input_parameter, size) == 0);
}

/*! Default setter. */
static inline int flyweight_class_default_setter(void *this, size_t size, void *input_parameter) {
ENTERLOG
	if(input_parameter==NULL) {
		return FLYWEIGHT_FAILED;
	}

	memcpy(this, input_parameter, size);
	return FLYWEIGHT_SUCCESS;
}

/*! Set methods. */
static void flyweight_class_set_methods(struct flyweight_class_methods_s *methods, ClassHandle class_factory) {
ENTERLOG
	if(!methods) {
		//set default. destrctor is NULL
		class_factory->methods.constructor = flyweight_class_default_constructor;
		class_factory->methods.equall_operand = flyweight_class_default_equall_operand;
		class_factory->methods.setter = flyweight_class_default_setter;
	} else {
		memcpy((void *)&class_factory->methods, (void *)methods, sizeof(class_factory->methods));
	}
EXITLOG
}

/*************
 * public interface API implement
*************/
ClassHandle flyweight_define_class(size_t class_size, int is_threadsafe, struct flyweight_class_methods_s *methods) {
ENTERLOG

	if(class_size<=0) {
		return NULL;
	}

	//allocate class_factory
	ClassHandle class_factory = (ClassHandle) calloc(1, sizeof(struct flyweight_class_factory_s));
	if( !class_factory ) {
		DEBUG_ERRPRINT("calloc instance list error:%s\n", strerror(errno));
		return NULL;
	}

	//when use threadsafe, allocate mutex lock
	if( is_threadsafe ) {
		class_factory->lock = (pthread_mutex_t *) calloc(1, sizeof(pthread_mutex_t));
		if( !class_factory->lock ) {
			DEBUG_ERRPRINT("class instance lock error:%s\n",  strerror(errno));
			free(class_factory);
			return NULL;
		}

		pthread_mutex_init(class_factory->lock, NULL);
	}

	class_factory->class_size = class_size;
	//set methods
	flyweight_class_set_methods(methods, class_factory);
EXITLOG
	return class_factory;
}

void * flyweight_get(ClassHandle classHandle, void * constructor_parameter) {
ENTERLOG
	//fail safe
	if(!classHandle) {
		return NULL;
	}

	void *ret=NULL;
	FLYWEIGHT_CLASS_LOCK(classHandle);

	//get instance
	struct flyweight_instance_s * instance = flyweight_factory_get(classHandle, constructor_parameter);
	if( instance ) {
		ret = instance->instance;
	}

	FLYWEIGHT_CLASS_UNLOCK

EXITLOG
	return ret;
}

int flyweight_set(ClassHandle classHandle, void * constructor_parameter, void * data, int (*setter)(void *this, size_t size, void *input_parameter)) {
ENTERLOG

	//fail safe
	if(!classHandle) {
		return FLYWEIGHT_FAILED;
	}

	int ret=FLYWEIGHT_FAILED;

	FLYWEIGHT_CLASS_LOCK(classHandle);

	//get and set instance
	struct flyweight_instance_s * instance = flyweight_factory_get(classHandle, constructor_parameter);

	if(!instance) {
		DEBUG_ERRPRINT("Failed to get instance! handle[%p]\n", classHandle );
		goto end;
	}

	if( setter ) {
		ret = setter(instance->instance, classHandle->class_size, data);
	} else if ( classHandle->methods.setter ) {
		ret = classHandle->methods.setter(instance->instance, classHandle->class_size, data);
	}

end:

	FLYWEIGHT_CLASS_UNLOCK

EXITLOG
	return ret;
}

void flyweight_clear(ClassHandle classHandle) {
ENTERLOG
	//fail safe
	if(!classHandle) {
		return;
	}

	pthread_mutex_t *keep_mutex_for_free=classHandle->lock;

	FLYWEIGHT_CLASS_LOCK(classHandle);

	//pop and free
	struct flyweight_instance_s *instance=NULL;
	while( (instance = flyweight_factory_pop_instance(classHandle)) != NULL ) {
		//call destcuctor
		if(classHandle->methods.destructor) {
			classHandle->methods.destructor(instance->instance);
		}
		flyweight_instance_free(instance);
	}
	free(classHandle);

	//initialize
	FLYWEIGHT_CLASS_UNLOCK

	//free API care NULL
	free(keep_mutex_for_free);
EXITLOG
}

