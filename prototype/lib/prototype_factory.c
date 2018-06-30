/**
 * @file prototype_factory.c
 * @brief      Implement of prototype_factory API, defined in prototype_factory.h.
 **/
#include <stdlib.h>
#include "dp_util.h"
#include "prototype_factory.h"

/*! prototype_factory_instance, PrototypeFactory class instance definition*/
struct prototype_factory_instance_t {
	void * base;
	size_t base_len;
	prototype_factory_method_t factory_method;
};

/*@name prototype_factory_instance private API definition*/
/* @{ */
/*! clone */
static void * prototype_default_clone(void * base, size_t base_length);
/*! free */
static void prototype_default_free(void * clone_base);
/*! free base */
static void prototype_default_free_base(void * base);
/* @} */

/*! define to set default*/
#define PROT_FACT_DEFAULT_METHOD(key) prototype_default_ ## key
/*! define to set method*/
#define PROT_FACT_SET_METHOD(this, factory_method, key) \
	if(factory_method && factory_method->key) instance->factory_method.key = factory_method->key;\
	else instance->factory_method.key = PROT_FACT_DEFAULT_METHOD(key)

/*@name prototype_factory_instance private API definition*/
/* @{ */
static void * prototype_default_clone(void * base, size_t base_length) {
	void * clone_data = malloc(base_length);
	if(!clone_data) return NULL;

	/*sallow copy*/
	memcpy(clone_data, base, base_length);
	return clone_data;
}

static void prototype_default_free(void * clone_base) {
	free(clone_base);
}

static void prototype_default_free_base(void * base) {
	free(base);
}
/* @} */
/*@name prototype_factory_instance public API definition*/
/* @{ */
/*! new */
PrototypeFactoryInstance prototype_factory_instance_new(void * base, size_t base_length, prototype_factory_method_t * factory_method) {
	PrototypeFactoryInstance instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->base = base;
	instance->base_len = base_length;

	PROT_FACT_SET_METHOD(instance, factory_method, clone);
	PROT_FACT_SET_METHOD(instance, factory_method, free);
	PROT_FACT_SET_METHOD(instance, factory_method, free_base);
	return instance;
}

/*! free */
void prototype_factory_instance_free(PrototypeFactoryInstance this) {
	this->factory_method.free_base(this->base);
	free(this);
}

/*! clone data */
void * prototype_factory_instance_clone_data(PrototypeFactoryInstance this) {
	return this->factory_method.clone(this->base, this->base_len);
}

/*! free data */
void prototype_factory_instance_free_data(PrototypeFactoryInstance this, void *data) {
	this->factory_method.free(data);
}
/* @} */
