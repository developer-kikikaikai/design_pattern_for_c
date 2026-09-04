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
	int is_threadsafe;
	ChainElement element;
};

typedef struct chain_of_resp_request_t {
	int id;
	int is_threadsafe;
} chain_of_resp_request_t;

/*! new API */
static void chain_of_resp_new(void *this, size_t size, void *input_parameter);
/*! @brief equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter);
/*! @brief setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter);
/*! @brief free member resource */
static void chain_of_resp_free(void *this);
/* @} */

struct cor_manager_t {
	FlyweightFactory handle;
	int is_threadsafe;
};

static flyweight_methods_t cor_methods = {
	.constructor=chain_of_resp_new,
	.equall_operand=chain_of_resp_equall_operand,
	.setter=chain_of_resp_setter,
	.destructor=chain_of_resp_free,
};

static CorManager cor_default_manager;
static int cor_default_threadsafe;
static pthread_mutex_t cor_default_lock = PTHREAD_MUTEX_INITIALIZER;

/*************
 * ChainOfResponsibility method definition
*************/
/*! new API */
/*input is name*/
static void chain_of_resp_new(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_of_resp_request_t *request = input_parameter;
	(void)size;
	instance->id = request->id;
	instance->is_threadsafe = request->is_threadsafe;
	instance->element = chain_element_new(instance->is_threadsafe);
}

/*! equall operand, check name */
static int chain_of_resp_equall_operand(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_of_resp_request_t *request = input_parameter;
	(void)size;

	//check name is same
	return instance->id == request->id;
}

/*! setter, add function to ChainElement */
static int chain_of_resp_setter(void *this, size_t size, void *input_parameter) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_element_req_t * data = input_parameter;
	(void)size;
	if(!instance->element) {
		instance->element = chain_element_new(instance->is_threadsafe);
	}
	if(!instance->element) {
		return COR_FAILED;
	}
	return chain_element_add_function(instance->element, data);
}

/*! free member resource */
static void chain_of_resp_free(void *this) {
	ChainOfResponsibility instance = (ChainOfResponsibility)this;
	chain_element_delete(instance->element);
}

static CorManager cor_get_default_manager(int create) {
	CorManager manager;
	pthread_mutex_lock(&cor_default_lock);
	if(create && !cor_default_manager) {
		cor_default_manager = cor_manager_new(cor_default_threadsafe);
	}
	manager = cor_default_manager;
	pthread_mutex_unlock(&cor_default_lock);
	return manager;
}

/*************
 * public interface API implement
*************/
void cor_set_threadsafe(int is_threadsafe) {
	pthread_mutex_lock(&cor_default_lock);
	if(!cor_default_manager) {
		cor_default_threadsafe = !!is_threadsafe;
	}
	pthread_mutex_unlock(&cor_default_lock);
}

CorManager cor_manager_new(int is_threadsafe) {
	CorManager manager = calloc(1, sizeof(*manager));
	if(!manager) {
		return NULL;
	}
	manager->is_threadsafe = !!is_threadsafe;
	manager->handle = flyweight_factory_new(sizeof(struct chain_of_resp_t), manager->is_threadsafe, &cor_methods);
	if(!manager->handle) {
		free(manager);
		return NULL;
	}
	return manager;
}

void cor_manager_free(CorManager manager) {
	if(!manager) {
		return;
	}
	flyweight_factory_free(manager->handle);
	free(manager);
}

ChainElementPart cor_manager_add_function(CorManager manager, const int id, chain_func func, void *ctx) {
	if(!func) {
		return NULL;
	}
	if(!manager) {
		return NULL;
	}

	chain_element_req_t data={{func, ctx}, NULL};
	chain_of_resp_request_t request = {id, manager->is_threadsafe};
	int ret = flyweight_set(manager->handle, &request, &data, NULL);
	if( ret == COR_FAILED ) {
		return NULL;
	} else {
		return data.result_element_part;
	}
}

void cor_manager_call(CorManager manager, const int id, void *arg) {
	if(!manager) {
		return;
	}
	chain_of_resp_request_t request = {id, manager->is_threadsafe};
	ChainOfResponsibility cor_instance = flyweight_get(manager->handle, &request);
	if(cor_instance) {
		chain_element_call(cor_instance->element, arg);
	}
}

void cor_manager_remove_function(CorManager manager, const int id, chain_func func) {
	if(!manager || !func) {
		return;
	}

	chain_of_resp_request_t request = {id, manager->is_threadsafe};
	ChainOfResponsibility cor_instance = flyweight_get(manager->handle, &request);
	if(cor_instance) {
		chain_element_remove_function(cor_instance->element, func);
	}
}

void cor_manager_remove_chain_element_part(CorManager manager, const int id, ChainElementPart element) {
	if(!manager || !element) {
		return;
	}

	chain_of_resp_request_t request = {id, manager->is_threadsafe};
	ChainOfResponsibility cor_instance = flyweight_get(manager->handle, &request);
	if(cor_instance) {
		chain_element_remove_element_part(cor_instance->element, element);
	}
}

ChainElementPart cor_add_function(const int id, chain_func func, void *ctx) {
	return cor_manager_add_function(cor_get_default_manager(1), id, func, ctx);
}

void cor_call(const int id, void *arg) {
	cor_manager_call(cor_get_default_manager(0), id, arg);
}

void cor_remove_function(const int id, chain_func func) {
	cor_manager_remove_function(cor_get_default_manager(0), id, func);
}

void cor_remove_chain_element_part(const int id, ChainElementPart element) {
	cor_manager_remove_chain_element_part(cor_get_default_manager(0), id, element);
}

void cor_clear(void) {
	CorManager manager;
	pthread_mutex_lock(&cor_default_lock);
	manager = cor_default_manager;
	cor_default_manager = NULL;
	pthread_mutex_unlock(&cor_default_lock);
	cor_manager_free(manager);
}
