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
typedef struct chain_of_resp * ChainOfResponsibility;
struct chain_of_resp {
	char *name;
	ChainElement element;
};

/*! new API */
static void chain_of_resp_new(void *this, size_t size, void *input_parameter);
/*! equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter);
/*! setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter);
/*! free member resource */
static void chain_of_resp_free(void *this);
/* }@ */

/*! @struct chain_of_resp_mng
 * @brief management parameter of this class API, to use flyweight
*/
struct chain_of_resp_mng {
	struct flyweight_class_methods_s method;
	ClassHandle handle;
} cor_mng_g = {
	.method ={
		.constructor=chain_of_resp_new,
		.equall_operand=chain_of_resp_equall_operand,
		.setter=chain_of_resp_setter,
		.destructor=chain_of_resp_free,
	},
	.handle = NULL,
};

/*************
 * ChainOfResponsibility method definition
*************/
/*! new API */
/*input is name*/
static void chain_of_resp_new(void *this, size_t size, void *input_parameter) {
ENTERLOG
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	char * name = (char *)input_parameter;
	instance->name = calloc(1, strlen(name)+ 1);
	if(!instance->name) {
		return;
	}

	strcpy(instance->name, name);
	instance->element = chain_element_new();
EXITLOG
}

/*! equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter) {
ENTERLOG
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	char * name = (char *)input_parameter;

	//check name is same
EXITLOG
	return (strcmp(instance->name, name)==0);
}

/*! setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter) {
ENTERLOG
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_func func = (chain_func) input_parameter;
EXITLOG
	return chain_element_add_function(instance->element, func);
}

/*! free member resource */
static void chain_of_resp_free(void *this) {
ENTERLOG
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	free(instance->name);
	chain_element_delete(instance->element);
EXITLOG
}

/*************
 * public interface API implement
*************/
int cor_add_function(const char *name, chain_func func) {
ENTERLOG
	if(name==NULL || strlen(name)==0 || func == NULL) {
		return COR_FAILED;
	}

	if(!cor_mng_g.handle) {
		/* get handle with thread safe */
		cor_mng_g.handle = flyweight_define_class(sizeof(struct chain_of_resp), 1, &cor_mng_g.method);
		if(!cor_mng_g.handle) {
			return COR_FAILED;
		}
	}

EXITLOG
	return flyweight_set(cor_mng_g.handle, (void *)name, func, NULL);
}

void cor_call(const char *name, void *arg) {
ENTERLOG
	if(!cor_mng_g.handle || name==NULL || strlen(name)==0) {
		return;
	}
	ChainOfResponsibility cor_instance = flyweight_get(cor_mng_g.handle, (void *)name);
	chain_element_call(cor_instance->element, arg);
EXITLOG
}

void cor_remove_function(const char *name, chain_func func) {
ENTERLOG
	if(!cor_mng_g.handle || name==NULL || strlen(name)==0 || func==NULL) {
		return;
	}

	ChainOfResponsibility cor_instance = flyweight_get(cor_mng_g.handle, (void *)name);
	chain_element_remove_function(cor_instance->element, func);
EXITLOG
}

void cor_clear(void) {
ENTERLOG
	flyweight_clear(cor_mng_g.handle);
	cor_mng_g.handle=NULL;
EXITLOG
}
