/**
 * @file prototype_manager.c
 * @brief      Implement of Prototype library API, defined in prototype.h.
 **/
#include <stdlib.h>
#include "dp_util.h"
#include "prototype_factory.h"

/*************
 * public define
*************/
struct prototype_factory_t {
	PrototypeFactory next;
	PrototypeFactory prev;
	pthread_mutex_t *lock;
	PrototypeFactoryInstance instance;
};

/*! @struct prototype_manager_t
 * @brief information of StateMachine, to use as list
*/
struct prototype_manager_t {
	PrototypeFactory head;
	PrototypeFactory tail;
	pthread_mutex_t *lock;
};

#define prototype_push(this, data) dputil_list_push((DPUtilList)this, (DPUtilListData)data);
#define prototype_pull(this, data) dputil_list_pull((DPUtilList)this, (DPUtilListData)data);
#define prototype_pop(this) (PrototypeFactory)dputil_list_pop((DPUtilList)this);

#define PROTOTYPE_LOCK(this) DPUTIL_LOCK(this->lock);
#define PROTOTYPE_UNLOCK DPUTIL_UNLOCK

PrototypeManager prototype_manager_new(int is_threadsafe) {
	PrototypeManager instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("Failed to create instance\n");
		return NULL;
	}

	if(is_threadsafe) {
		instance->lock = malloc(sizeof(*(instance->lock)));
		if(!instance->lock) {
			DEBUG_ERRPRINT("Fai|led to create lock instance\n");
			free(instance);
			return NULL;
		}
		pthread_mutex_init(instance->lock, NULL);
	}

	return instance;
}

void prototype_manager_free(PrototypeManager this) {
	pthread_mutex_t *lock=NULL;
	if(!this) {
		return;
	}

PROTOTYPE_LOCK(this)
	lock=this->lock;
	PrototypeFactory factory = prototype_pop(this);
	while(factory) {
		prototype_factory_instance_free(factory->instance);
		free(factory);
		factory = prototype_pop(this);
	}
	free(this);
PROTOTYPE_UNLOCK
	free(lock);
}

PrototypeFactory prototype_register(PrototypeManager this, void * base, size_t base_length, prototype_factory_method_t * factory_method) {
	PrototypeFactory factory = NULL;
	if(!this || !base || base_length==0) {
		return NULL;
	}

PROTOTYPE_LOCK(this)
	factory = calloc(1, sizeof(*factory));
	if(!factory) {
		DEBUG_ERRPRINT("Fai|led to create PrototypeFactory\n");
		goto end;
	}

	factory->instance = prototype_factory_instance_new(base, base_length, factory_method);
	if(!factory->instance) {
		DEBUG_ERRPRINT("Fai|led to create PrototypeFactory instance\n");
		free(factory);
		factory=NULL;
		goto end;
	}

	factory->lock=this->lock;
	prototype_push(this, factory);
end:
PROTOTYPE_UNLOCK
	return factory;
}

void prototype_unregister(PrototypeManager this, PrototypeFactory factory) {
	if(!this || !factory) {
		return;
	}

PROTOTYPE_LOCK(this)
	prototype_pull(this, factory);
PROTOTYPE_UNLOCK
	prototype_factory_instance_free(factory->instance);
	free(factory);
}

void * prototype_clone(PrototypeFactory this) {
	if(!this) {
		return NULL;
	}

	void * data;
PROTOTYPE_LOCK(this)
	data = prototype_factory_instance_clone_data(this->instance);
PROTOTYPE_UNLOCK
	return data;
}

void prototype_free(PrototypeFactory this, void * cloned_data) {
	if(!this || !cloned_data) {
		return;
	}

PROTOTYPE_LOCK(this)
	prototype_factory_instance_free_data(this->instance, cloned_data);
PROTOTYPE_UNLOCK
}
