/**
 * @file mement.c
 * @brief      Implement of mement API, defined in mement.h.
 **/
#include <stdlib.h>
#include <string.h>
#include "mement.h"

/*! mement_factory_instance, PrototypeFactory class instance definition*/
struct mement_register_t {
	void * base;
	size_t base_len;
	mement_method_t method;
};

/*@name mement_factory_instance private API definition*/
/* @{ */
/*! free */
static void mement_default_constructor(void *instance, void * base, size_t base_length);
/*! copy */
static void mement_default_copy(void *broken_instance, void * base, size_t base_length);
/*! free */
static void mement_default_free(void * base, size_t base_length);
/* @} */

/*! define to set default*/
#define MEMENTDEFAULT_METHOD(key) mement_default_ ## key
/*! define to set method*/
#define MEMENTSET_METHOD(this, method, key) \
	if(method && method->key) instance->method.key = method->key;\
	else instance->method.key = MEMENTDEFAULT_METHOD(key)

/*@name mement_factory_instance private API definition*/
/* @{ */
static void mement_default_constructor(void *instance, void * base, size_t base_length) {
	/*shallow copy*/
	memcpy(instance, base, base_length);
}

static void mement_default_copy(void *broken_instance, void * base, size_t base_length) {
	/*shallow copy*/
	memcpy(broken_instance, base, base_length);
}

static void mement_default_free(void * base, size_t base_length) {
	(void)base;
}

/* @} */
/*@name mement_factory_instance public API definition*/
/* @{ */
/*! new */
MementRegister mement_register(void * base, size_t base_length, mement_method_t * method) {
	if(!base || base_length==0) return NULL;

	MementRegister instance = malloc(sizeof(*instance) + base_length);
	if(!instance) {
		return NULL;
	}
	memset(instance, 0, sizeof(*instance) + base_length);

	instance->base = (instance + 1);
	instance->base_len = base_length;

	MEMENTSET_METHOD(instance, method, constructor);
	MEMENTSET_METHOD(instance, method, copy);
	MEMENTSET_METHOD(instance, method, free);

	instance->method.constructor(instance->base, base, base_length);
	return instance;
}

/*! free */
void mement_unregister(MementRegister this) {
	if(!this) return;

	this->method.free(this->base, this->base_len);
	free(this);
}

/*! remembe*/
void mement_remember(MementRegister this, void * broken_instance, int is_unregister_mement) {
	if(!this || !broken_instance) return;

	this->method.copy(broken_instance, this->base, this->base_len);

	/*unregister check*/
	if(is_unregister_mement) mement_unregister(this);
}
/* @} */
