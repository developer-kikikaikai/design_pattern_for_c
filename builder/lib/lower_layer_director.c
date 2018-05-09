/**
 *    @brief      Implement of lower_layer_director (related to builder design petten) library API, defined in flyweight.h

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include "lower_layer_director.h"
#include "lower_layer_builder.h"
#include "dp_debug.h"

/*************
 * public define
*************/
#define LLD_METHODSIZE_MAX (64)

/*! @class director class
 * @brief director class
*/
struct lower_layer_director_class {
	//public
	pthread_t tid;
	int builder_method_cnt;
	int (** builder_methods)(void * initial_parameter);
	//private
	void * _dlhandle;
	void * (*_builder_instance_new)(void);
	void (*_builder_instance_free)(void *interface);
};
#define DirectorClassSize sizeof(struct ll_director_class)

/*! @name DirectorClass private method */
/* @{ */
static int ll_director_class_open_library(DirectorClass this, char * builder_lib_name);
static int ll_director_class_load_methods(char * conffile, char ***methodname);
static void ll_director_class_free_methods(int methodcnt, char **methodname);
static int ll_director_class_add_methods(DirectorClass this, int methodcnt, char **methodname);
static void ll_director_class_load_interface_method(DirectorClass this);
/* }@ */

/*! @name DirectorClass public method */
/* @{ */
static void * ll_director_class_new(char * builder_lib_name, char * builder_interface_conf);
static void * ll_director_interface_class_new(DirectorClass this);
static void ll_director_interface_class_free(DirectorClass this, void * instance);
static void ll_director_class_free(DirectorClass this);
/* }@ */

/*! @class builder action class
 * @brief action builder, to call builder by other thread, create class for it
*/
typedef struct ll_builder_action_class {
	//private
	void * _initial_parameter;
	void (*_initial_result)(int result);
	int _builder_method_cnt;
	int (** _builder_methods)(void * initial_parameter);
} ll_builder_action_class_t, *BuilderActionClass;


/*! @name BuilderActionClass private method */
/* @{ */
static void * ll_builder_action_run(void * this);
/* }@ */

/*! @name BuilderActionClass public method */
/* @{ */
static BuilderActionClass ll_builder_action_new(DirectorClass _director, void * initial_parameter, void (*initial_result)(int result));
static void ll_builder_action_destruct(pthread_t tid);
/* }@ */

//It's better to separate file for two classes?

/*************
 * for DirectorClass method
*************/
/*! @name DirectorClass private method implement*/
/* @{ */
static int ll_director_class_open_library(DirectorClass this, char * builder_lib_name) {
ENTERLOG
	//load library
	void * handle = dlopen(builder_lib_name, RTLD_NOW);
	if(!handle) {
		DEBUG_ERRPRINT("failed to open library %s\n", builder_lib_name);
		return LL_BUILDER_FAILED;
	}
	this->_dlhandle = handle;
EXITLOG
	return LL_BUILDER_SUCCESS;
}

static int ll_director_class_load_methods(char * conffile, char ***methodname) {
ENTERLOG
	int ret_num=0;
	FILE *fp = fopen(conffile, "r");
	if(!fp) {
		DEBUG_ERRPRINT("failed to open file %s\n", conffile);
		return LL_BUILDER_FAILED;
	}

	//count max line
	int line=0;
	char buffer[LLD_METHODSIZE_MAX];
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		line++;
	}
	//goto head
	fseek(fp, 0, SEEK_SET);

	//allocate
	*methodname = (char **) calloc(line, sizeof(char *));
	if(!*methodname) {
		goto err;
	}

	//read conf
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		//skip comment
		if(buffer[0]=='#') {
			DEBUG_ERRPRINT("continue comment\n");
			continue;
		}

		//allocate memory
		(*methodname)[ret_num] = calloc(1, LLD_METHODSIZE_MAX);
		if(!methodname[ret_num]) {
			goto err;
		}

		//clear after comment
		char *after_comment = strstr(buffer, "/");
		if(after_comment) {
			*after_comment = 0;
		}

		snprintf((*methodname)[ret_num++], LLD_METHODSIZE_MAX, "%s", buffer);
	}
	fclose(fp);
EXITLOG
	return ret_num;
err:
	fclose(fp);
	ll_director_class_free_methods(ret_num, *methodname);
	return LL_BUILDER_FAILED;
}

static void ll_director_class_free_methods(int methodcnt, char **methodname) {
ENTERLOG
	if(!methodname) {
		return;
	}

	int i=0;
	for(i = 0; i < methodcnt; i ++ ) {
		free(methodname[i]);
	}
	free(methodname);
EXITLOG
}

static int ll_director_class_add_methods(DirectorClass this, int methodcnt, char **methodname) {
ENTERLOG
	this->builder_methods = calloc(methodcnt, sizeof(void (*)(void *)));
	if(!this->builder_methods) {
		return LL_BUILDER_FAILED;
	}

	int i=0;
	for( i = 0; i < methodcnt; i ++ ) {
		//don't care NULL
		this->builder_methods[i] = dlsym(this->_dlhandle, methodname[i]);
		if(!this->builder_methods) {
			DEBUG_ERRPRINT("library API %s is not implement, error detail =%s\n", methodname[i], dlerror());
		}
	}
	this->builder_method_cnt = methodcnt;
	return LL_BUILDER_SUCCESS;
EXITLOG
}

static void ll_director_class_load_interface_method(DirectorClass this) {
ENTERLOG
	this->_builder_instance_new = dlsym(this->_dlhandle, LL_BUILDER_NEWNAME);
	if( this->_builder_instance_new ) {
		this->_builder_instance_free = dlsym(this->_dlhandle, LL_BUILDER_FREENAME);
	}
EXITLOG
}

/* }@ */

/*! @name DirectorClass public method implement*/
/* @{ */
static void * ll_director_class_new(char * builder_lib_name, char * builder_interface_conf) {
ENTERLOG
	int ret = LL_BUILDER_FAILED;
	int methodcnt;
	char **methodnames=NULL;

	DirectorClass instance = (DirectorClass) calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("failed to calloc\n");
		return NULL;
	}

	//load library
	ret = ll_director_class_open_library(instance, builder_lib_name);
	if( ret == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to open library\n");
		goto err;
	}

	//get methods name from conf
	methodcnt = ll_director_class_load_methods(builder_interface_conf, &methodnames);
	if( methodcnt == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to load methods conf %s\n", builder_interface_conf);
		goto err;
	}

	//load library method in conf
	ret = ll_director_class_add_methods(instance, methodcnt, methodnames);
	//free local memory
	ll_director_class_free_methods(methodcnt, methodnames);
	if( ret == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to load methods conf %s\n", builder_interface_conf);
		goto err;
	}

	//load common library
	ll_director_class_load_interface_method(instance);
EXITLOG
	return instance;

err:
	ll_director_class_free(instance);
EXITLOG
	return NULL;
}

static void * ll_director_interface_class_new(DirectorClass this) {
ENTERLOG
	if(!this || !this->_builder_instance_new) {
		DEBUG_ERRPRINT("No interface\n");
		return NULL;
	}

EXITLOG
	return this->_builder_instance_new();
}

static void ll_director_interface_class_free(DirectorClass this, void * instance) {
ENTERLOG
	if(!instance || !this->_builder_instance_free) {
		DEBUG_ERRPRINT("No interface\n");
		return;
	}

	this->_builder_instance_free(instance);
EXITLOG
}

static void ll_director_class_free(DirectorClass this) {
ENTERLOG
	if(!this) {
		return;
	}

	//free instance
	free(this->builder_methods);

	//close library
	if(this->_dlhandle) {
		if(dlclose(this->_dlhandle)) {
			DEBUG_ERRPRINT("failed to unload library\n");
		}
	}

	free(this);
EXITLOG
}
/* }@ */

/*************
 * for BuilderActionClass method
*************/
static BuilderActionClass ll_builder_action_new(DirectorClass director, void * initial_parameter, void (*initial_result)(int result)) {
ENTERLOG
	BuilderActionClass instance = calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("failed to new BuilderActionClass\n");
		return NULL;
	}

	instance->_initial_parameter = initial_parameter;
	instance->_initial_result = initial_result;
	instance->_builder_method_cnt = director->builder_method_cnt;
	instance->_builder_methods =  director->builder_methods;
EXITLOG
	return instance;
}

static void * ll_builder_action_run(void * arg) {
ENTERLOG
	BuilderActionClass this = (BuilderActionClass) arg;
	int i=0;
	int ret = LL_BUILDER_SUCCESS;
	//call builder methods
	for( i = 0; i < this->_builder_method_cnt; i++) {
		if(this->_builder_methods[i]) {
			DEBUG_ERRPRINT("methods[%d] call\n", i);
			ret = this->_builder_methods[i](this->_initial_parameter);
			if(ret == LL_BUILDER_FAILED) {
				break;
			}
		}
	}

	//call result callback
	if(this->_initial_result) {
		this->_initial_result(ret);
	}
	free(this);
	pthread_exit(NULL);
	return NULL;
EXITLOG
}

static void ll_builder_action_destruct(pthread_t tid) {
ENTERLOG
	if(0<tid) {
		pthread_join(tid, NULL);
	}
EXITLOG
}

/*************
 * public interface API implement
*************/
LowerLayerDirector lower_layer_director_new(char * builder_lib_name, char * builder_interface_conf) {
ENTERLOG
	LowerLayerDirector instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->director = ll_director_class_new(builder_lib_name, builder_interface_conf);
	if(!instance->director) {
		DEBUG_ERRPRINT("failed to new director\n");
		goto err;
	}

	instance->lower_layer_interface = ll_director_interface_class_new(instance->director);
	return instance;
err:
	lower_layer_director_free(instance);
EXITLOG
	return NULL;
}

void lower_layer_director_construct(LowerLayerDirector director, void * initial_parameter, void (*initial_result)(int result)) {
ENTERLOG

	BuilderActionClass instance = ll_builder_action_new(director->director, initial_parameter, initial_result);
	if(!instance) {
		DEBUG_ERRPRINT("failed to new BuilderActionClass\n");
		return;
	}

	DirectorClass director_instance = director->director;
	
	pthread_create(&director_instance->tid, NULL, ll_builder_action_run, instance);
	//free instance in ll_builder_action_run
EXITLOG
}

void lower_layer_director_destruct(LowerLayerDirector director) {
ENTERLOG
	if(!director) {
		return;
	}

	DirectorClass instance = director->director;

	ll_builder_action_destruct(instance->tid);
EXITLOG
}

void lower_layer_director_free(LowerLayerDirector director) {
ENTERLOG
	if(!director) {
		return;
	}

	DirectorClass instance =  director->director;

	//to care constructing now
	ll_builder_action_destruct(instance->tid);

	ll_director_interface_class_free(instance, director->lower_layer_interface);
	ll_director_class_free(instance);
	free(director);
EXITLOG
}
