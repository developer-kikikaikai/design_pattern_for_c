/**
 * @file director.c
 *    @brief      Implement of director API, defined in director.h
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include "builder_action.h"
#include "director.h"
#include "dp_debug.h"

/*************
 * public define
*************/
#define LLD_METHODSIZE_MAX (64)

/*! @struct Director
 * @brief director class
*/
struct director_t {
	//public
	pthread_t tid;
	int is_sync;
	int builder_method_cnt;
	int (** builder_methods)(void * initial_parameter);
	//private
	void * dlhandle;
	void * (*builder_instance_new)(void);
	void (*builder_instance_free)(void *interface);
};

/*! @name Director private method */
/* @{ */
static int director_open_library(Director this, char * builder_lib_name);
static int director_load_methods(char * conffile, char ***methodname);
static void director_free_methods(int methodcnt, char **methodname);
static int director_add_methods(Director this, int methodcnt, char **methodname);
static void director_load_interface_method(Director this);
/* @} */

/*************
 * for Director private method
*************/
/*! @name Director private method implement*/
/* @{ */
static int director_open_library(Director this, char * builder_lib_name) {
	//load library
	void * handle = dlopen(builder_lib_name, RTLD_NOW);
	if(!handle) {
		DEBUG_ERRPRINT("failed to open library %s\n", builder_lib_name);
		return LL_BUILDER_FAILED;
	}
	this->dlhandle = handle;
	return LL_BUILDER_SUCCESS;
}

static int director_load_methods(char * conffile, char ***methodname) {
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
	return ret_num;
err:
	fclose(fp);
	director_free_methods(ret_num, *methodname);
	return LL_BUILDER_FAILED;
}

static void director_free_methods(int methodcnt, char **methodname) {
	if(!methodname) {
		return;
	}

	int i=0;
	for(i = 0; i < methodcnt; i ++ ) {
		free(methodname[i]);
	}
	free(methodname);
}

static int director_add_methods(Director this, int methodcnt, char **methodname) {
	this->builder_methods = calloc(methodcnt, sizeof(void (*)(void *)));
	if(!this->builder_methods) {
		return LL_BUILDER_FAILED;
	}

	int i=0;
	for( i = 0; i < methodcnt; i ++ ) {
		//don't care NULL
		this->builder_methods[i] = dlsym(this->dlhandle, methodname[i]);
		if(!this->builder_methods) {
			DEBUG_ERRPRINT("library API %s is not implement, error detail =%s\n", methodname[i], dlerror());
		}
	}
	this->builder_method_cnt = methodcnt;
	return LL_BUILDER_SUCCESS;
}

static void director_load_interface_method(Director this) {
	this->builder_instance_new = dlsym(this->dlhandle, LL_BUILDER_NEWNAME);
	if( this->builder_instance_new ) {
		this->builder_instance_free = dlsym(this->dlhandle, LL_BUILDER_FREENAME);
	}
}

/* @} */

/*! @name Director public method implement*/
/* @{ */
Director director_new(char * builder_lib_name, char * builder_interface_conf) {
	int ret = LL_BUILDER_FAILED;
	int methodcnt;
	char **methodnames=NULL;

	Director instance = (Director) calloc(1, sizeof(*instance));
	if(!instance) {
		DEBUG_ERRPRINT("failed to calloc\n");
		return NULL;
	}

	//load library
	ret = director_open_library(instance, builder_lib_name);
	if( ret == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to open library\n");
		goto err;
	}

	//get methods name from conf
	methodcnt = director_load_methods(builder_interface_conf, &methodnames);
	if( methodcnt == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to load methods conf %s\n", builder_interface_conf);
		goto err;
	}

	//load library method in conf
	ret = director_add_methods(instance, methodcnt, methodnames);
	//free local memory
	director_free_methods(methodcnt, methodnames);
	if( ret == LL_BUILDER_FAILED ) {
		DEBUG_ERRPRINT("failed to load methods conf %s\n", builder_interface_conf);
		goto err;
	}

	//load common library
	director_load_interface_method(instance);
	return instance;

err:
	director_free(instance);
	return NULL;
}

int director_construct(Director director, void * initial_parameter, void (*initial_result)(void * initial_parameter, int result)) {

	int ret = LL_BUILDER_SUCCESS;
	builder_action_parameter_t parameter;
	parameter.initial_parameter = initial_parameter;
	parameter.initial_result = initial_result;
	parameter.builder_method_cnt = director->builder_method_cnt;
	parameter.builder_methods = director->builder_methods;

	if(parameter.initial_result) {
		director->tid = builder_action_construct(&parameter);
	} else {
		director->is_sync = 1;
		ret = builder_action_construct_sync(&parameter);
	}
	return ret;
}

void director_destruct(Director director) {
	//to care constructing now
	if(!director->is_sync) {
		builder_action_destruct(director->tid);
	}
}

void * director_interface_class_new(Director this) {
	if(!this || !this->builder_instance_new) {
		DEBUG_ERRPRINT("No interface\n");
		return NULL;
	}

	return this->builder_instance_new();
}

void director_interface_class_free(Director this, void * instance) {
	if(!instance || !this->builder_instance_free) {
		DEBUG_ERRPRINT("No interface\n");
		return;
	}

	this->builder_instance_free(instance);
}

void director_free(Director this) {
	if(!this) {
		return;
	}

	//free instance
	free(this->builder_methods);

	//close library
	if(this->dlhandle) {
		if(dlclose(this->dlhandle)) {
			DEBUG_ERRPRINT("failed to unload library\n");
		}
	}

	free(this);
}
/* @} */
