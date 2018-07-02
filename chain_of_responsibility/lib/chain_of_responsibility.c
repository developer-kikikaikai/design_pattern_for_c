/**
 * @file chain_of_responsibility.c
 *    @brief      Implement of Chain of Responsibility design petten library API
 **/
#include "chain_of_responsibility.h"
#include "flyweight.h"
#include "chain_element.h"
#include "dp_util.h"

/*! @name ChainOfResponsibility class */
/* @{ */
/*! @struct ChainOfResponsibility
 * @brief ChainOfResponsibility class instance definition
*/
typedef struct chain_of_resp_t * ChainOfResponsibility;
struct chain_of_resp_t {
	int id;
	ChainElement element;
};

/*! new API */
static void chain_of_resp_new(void *this, size_t size, void *input_parameter);
/*! @brief equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter);
/*! @brief setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter);
/*! @brief free member resource */
static void chain_of_resp_free(void *this);
/* @} */

/*! @struct chain_of_resp_mng_t
 * @brief management parameter of this class API, to use flyweight
*/
struct chain_of_resp_mng_t {
	flyweight_methods_t method;
	FlyweightFactory handle;
	int is_threadsafe;
} cor_mng_g = {
	.method ={
		.constructor=chain_of_resp_new,
		.equall_operand=chain_of_resp_equall_operand,
		.setter=chain_of_resp_setter,
		.destructor=chain_of_resp_free,
	},
	.handle = NULL,
	.is_threadsafe = 0,
};

/*************
 * ChainOfResponsibility method definition
*************/
/*! new API */
/*input is name*/
static void chain_of_resp_new(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	int * id = (int *)input_parameter;
	instance->id = *id;
	instance->element = chain_element_new(cor_mng_g.is_threadsafe);
}

/*! equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	int * id = (int *)input_parameter;

	//check name is same
	return instance->id == *id;
}

/*! setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_element_data_t * data = input_parameter;
	return chain_element_add_function(instance->element, data);
}

/*! free member resource */
static void chain_of_resp_free(void *this) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_element_delete(instance->element);
}

/*************
 * public interface API implement
*************/
void cor_set_threadsafe(int is_threadsafe) {
	cor_mng_g.is_threadsafe = is_threadsafe;
}

int cor_add_function(const int id, chain_func func, void *ctx) {
	if(!func) {
		return COR_FAILED;
	}

	if(!cor_mng_g.handle) {
		/* get handle with thread safe */
		cor_mng_g.handle = flyweight_factory_new(sizeof(struct chain_of_resp_t), cor_mng_g.is_threadsafe, &cor_mng_g.method);
		if(!cor_mng_g.handle) {
			return COR_FAILED;
		}
	}

	chain_element_data_t data={func, ctx};
	return flyweight_set(cor_mng_g.handle, (void *)&id, &data, NULL);
}

void cor_call(const int id, void *arg) {
	if(!cor_mng_g.handle) {
		return;
	}
	ChainOfResponsibility cor_instance = flyweight_get(cor_mng_g.handle, (void *)&id);
	if(cor_instance) {
		chain_element_call(cor_instance->element, arg);
	}
}

void cor_remove_function(const int id, chain_func func) {
	if(!cor_mng_g.handle || func==NULL) {
		return;
	}

	ChainOfResponsibility cor_instance = flyweight_get(cor_mng_g.handle, (void *)&id);
	if(cor_instance) {
		chain_element_remove_function(cor_instance->element, func);
	}
}

void cor_clear(void) {
	flyweight_factory_free(cor_mng_g.handle);
	cor_mng_g.handle=NULL;
}
