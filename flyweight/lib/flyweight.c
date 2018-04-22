/**
 *    @brief      Implement of Flyweight design petten library API, defined in flyweight.h
**/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "flyweight.h"

/*************
 * define debug
*************/
//#define DBGFLAG
#ifdef DBGFLAG
#include <errno.h>
#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#define DEBUG_ERRPRINT_(fmt, ...)  \
        fprintf(stderr, "%s(%d): "fmt"%s", __FUNCTION__,__LINE__, __VA_ARGS__)
#else
#define DEBUG_ERRPRINT(...) 
#endif

/*************
 * public define
*************/
/*! @name Error definition */
/* @{ */
#define FLYWEIGHT_FAILED (-1) /*! error */
#define FLYWEIGHT_SUCCESS (0) /*! success */
/* @} */

/*! @brief default max register size */
#define FLYWEIGHT_MAXREGISTER_SIZE (1024)

/*! @struct flyweight_instance_s
 * @brief instance data definition.
*/
struct flyweight_instance_s {
	//private member
	struct flyweight_init_s methods;/*! methods */
	size_t instance_size;/*! size of instance */
	void * instance;/*! allocated instance size */
	pthread_mutex_t *lock;/*! lock pointer, if != null, lock at getter and setter */

	pthread_mutex_t lock_instance;/*! real lock instance */
};

/*! lock mutex */
static inline void flyweight_mutex_lock(void *handle);
/*! unlock mutex */
static inline void flyweight_mutex_unlock(void *handle);

/*! @name public API for flyweight_instance_s */
/* @{ */
/*! Create. instance is allocated data, this API allocate buffer in flyweight_instance_s */
static int flyweight_instance_regist(size_t class_size, int is_threadsafe, struct flyweight_init_s *methods, struct flyweight_instance_s * instance);
/*! Delete. instance is allocated data, this API only free buffer in in flyweight_instance_s. Please pop data from mng list before calling this API. */
static void flyweight_instance_unregist(struct flyweight_instance_s * instance);
/*! Check has instance. */
static inline int has_flyweight_instance(struct flyweight_instance_s * instance);
/*! Getter. */
static void * flyweight_instance_get_instance(struct flyweight_instance_s * instance);
/*! Setter. */
static int flyweight_instance_set_instance(struct flyweight_instance_s * instance, void *data, int (*setter)(void *src, size_t srcsize, void *dist));
/* @} */

/*! @name private API for flyweight_instance_s */
/* @{ */
/*! Default setter. */
static inline int flyweight_instance_default_setter(void *src, size_t srcsize, void *dist);
/*! Set methods. */
static inline void flyweight_instance_methods(struct flyweight_init_s *methods, struct flyweight_instance_s *instance);
#define FLYWEIGHT_INSTANCE_LOCK(instance) \
	flyweight_mutex_lock(instance->lock);\
	pthread_cleanup_push(flyweight_mutex_unlock, instance->lock);


#define FLYWEIGHT_INSTANCE_UNLOCK(instance) pthread_cleanup_pop(1);
/* @} */

/*! @struct flyweight_mng_s
 * @brief instance list manager definition.
*/
struct flyweight_mng_s {
#ifdef STRICT_ENSUCE_THREADSAGE
	pthread_mutex_t lock_all;/*! if ensure thread safe strictly, always lock data to access flyweight_mng_s */
#endif
	int max_num;/*! max_registor num */
	int current_num;/*! current registored num */
	struct flyweight_instance_s *instances;/*! instance list */
} flyweight_mng_g = {
#ifdef STRICT_ENSUCE_THREADSAGE
	.lock_all = PTHREAD_MUTEX_INITIALIZER,
#endif
	.max_num=FLYWEIGHT_MAXREGISTER_SIZE,
	.current_num=0,
	.instances=NULL,
};

/*! @name private API for flyweight_mng_s
*/
static int flyweight_mng_get_next_index(struct flyweight_mng_s *mng);
static int flyweight_mng_is_valid_index(int id);
#ifdef STRICT_ENSUCE_THREADSAGE
#define FLYWEIGHT_MNG_LOCK \
	flyweight_mutex_lock(&flyweight_mng_g.lock_all);\
	pthread_cleanup_push(flyweight_mutex_unlock, &flyweight_mng_g.lock_all);
#define FLYWEIGHT_MNG_UNLOCK pthread_cleanup_pop(1);
#else
#define FLYWEIGHT_MNG_LOCK
#define FLYWEIGHT_MNG_UNLOCK 
#endif
/* @} */

/*************
 * implement
*************/
/*! lock handle */
static inline void flyweight_mutex_lock(void *handle) {
	if(!handle) {
		return;
	}

	pthread_mutex_t * lock=(pthread_mutex_t *)handle;
	pthread_mutex_lock(lock);
}

/*! unlock handle */
static inline void flyweight_mutex_unlock(void *handle) {
	if(!handle) {
		return;
	}

	pthread_mutex_t * lock=(pthread_mutex_t *)handle;
	pthread_mutex_unlock(lock);
}

/*************
 * for flyweight_instance_s API
*************/
static int flyweight_instance_regist(size_t class_size, int is_threadsafe, struct flyweight_init_s *methods, struct flyweight_instance_s * instance) {
	//clear memory
	memset(instance, 0, sizeof(struct flyweight_instance_s));

	//set data
	instance->instance_size = class_size;
	if(is_threadsafe) {
		//if is_threadsafe=> lock is not NULL, so lock/unlock by flyweight_mutex_lock/unlock
		instance->lock = &instance->lock_instance;
	}

	//always initialize lock_instance because it is used on flyweight_mutex_lock
	pthread_mutex_init(&instance->lock_instance, NULL);

	//set methods
	flyweight_instance_methods(methods, instance);
}

static void flyweight_instance_unregist(struct flyweight_instance_s * instance) {
	//already pop data from flyweight_mng_s, so it's OK not to lock
	if(instance->methods.destructor) {
		instance->methods.destructor(instance->instance);
	}
	pthread_mutex_destroy(&instance->lock_instance);
	free(instance->instance);
}

static inline int has_flyweight_instance(struct flyweight_instance_s * instance) {
	return (instance->instance_size != 0);
}

static void * flyweight_instance_get_instance(struct flyweight_instance_s * instance) {
	void *ret=NULL;

	//lock if thread safe
	FLYWEIGHT_INSTANCE_LOCK(instance);

	//if already allocated, only return instance
	if(!instance->instance) {
		//allocate
		instance->instance = calloc(1, instance->instance_size);
		//success to allocated, and there is a constructor
		if(instance->instance && instance->methods.constructor) {
			instance->methods.constructor(instance->instance);
		}
	}
	ret=instance->instance;

	//unlock
	FLYWEIGHT_INSTANCE_UNLOCK(instance);

	return ret;
}

static int flyweight_instance_set_instance(struct flyweight_instance_s * instance, void *data, int (*setter)(void *src, size_t srcsize, void *dist)) {

	int ret=FLYWEIGHT_FAILED;

	//lock if thread safe
	FLYWEIGHT_INSTANCE_LOCK(instance);

	if(!instance->instance) {
		//not allocate yet
		goto end;
	}

	if(setter) {
		ret = setter(instance->instance, instance->instance_size, data);
	} else if (instance->methods.setter) {
		ret = instance->methods.setter(instance->instance, instance->instance_size, data);
	} else {
		ret = FLYWEIGHT_SUCCESS;
	}
end:
	FLYWEIGHT_INSTANCE_UNLOCK(instance);
	return ret;
}

/*private API*/

/*! Default setter. */
static inline int flyweight_instance_default_setter(void *src, size_t srcssrcsize, void *dist) {
	//case: instance data already allocated
	memcpy(src, dist, srcssrcsize);
	return 0;
}

/*! Set methods. */
static inline void flyweight_instance_methods(struct flyweight_init_s *methods, struct flyweight_instance_s *instance) {
	if(!methods) {
		instance->methods.setter = flyweight_instance_default_setter;
	} else {
		memcpy(&instance->methods, methods, sizeof(instance->methods));
	}
}

/* @} */

/*************
 * for flyweight_mng_s API
*************/
//call after FLYWEIGHT_MNG_LOCK, and always keep max
static int flyweight_mng_get_next_index(struct flyweight_mng_s *mng) {
	int index=mng->current_num;
	int cnt=0;//check size
	while(has_flyweight_instance(&mng->instances[index])) {
		cnt++;
		/*fale safe.
		 * If you keep instance max_num when you call flyweight_mng_get_next_index,
		 * There is no case of this.
		 */
		if( mng->max_num <= cnt ) {
			//full
			index=cnt;
			break;
		}

		index++;
		if(index==mng->max_num) {
			//loop
			index = 0;
		}
	}

	return index;
}

static int flyweight_mng_is_valid_index(int id) {
	//area check
	if(flyweight_mng_g.max_num <= id) {
		return FLYWEIGHT_FAILED;
	}
	//registered check
	if(!has_flyweight_instance(&flyweight_mng_g.instances[id])) {
		return FLYWEIGHT_FAILED;
	}
	return FLYWEIGHT_SUCCESS;
}

/*************
 * public interface API implement
*************/
void flyweight_set_storagemax(int max_reg_num) {
FLYWEIGHT_MNG_LOCK
	if (!flyweight_mng_g.instances) {
		flyweight_mng_g.max_num = max_reg_num;
	}
FLYWEIGHT_MNG_UNLOCK
}

int flyweight_register_class(size_t class_size, int is_threadsafe, struct flyweight_init_s *methods) {
	int ret=FLYWEIGHT_FAILED;
	int index =0;
FLYWEIGHT_MNG_LOCK

	//check size
	if(flyweight_mng_g.max_num <= flyweight_mng_g.current_num) {
		goto err;
	}

	//new instance if flyweight_register_class call first.
	if(!flyweight_mng_g.instances) {
		flyweight_mng_g.instances = (struct flyweight_instance_s *) calloc(flyweight_mng_g.max_num, sizeof(struct flyweight_instance_s));
		if(!flyweight_mng_g.instances) {
			DEBUG_ERRPRINT("calloc instance list error, registror size=%u:%s\n",flyweight_mng_g.max_num, strerror(errno));
			goto err;
		}
	}

	index = flyweight_mng_get_next_index(&flyweight_mng_g);

	//index is always under maxnum
	struct flyweight_instance_s * instance = &flyweight_mng_g.instances[index];

	DEBUG_ERRPRINT("register[%d]: size=%d\n", index, class_size);
	ret = flyweight_instance_regist(class_size, is_threadsafe, methods, instance);
	if(ret == FLYWEIGHT_FAILED) {
		DEBUG_ERRPRINT("class instance create error:%s\n",  strerror(errno));
		goto err;
	}

	//keep return value
	ret = index;

	//count up num and index
	flyweight_mng_g.current_num++;
err:
FLYWEIGHT_MNG_UNLOCK
	return ret;
}

void flyweight_unregister_class(int id) {

FLYWEIGHT_MNG_LOCK
	//validate id check
	if(flyweight_mng_is_valid_index(id) != FLYWEIGHT_FAILED) {
		struct flyweight_instance_s *instance_p = &flyweight_mng_g.instances[id];

		//to pop data, copy first
		flyweight_mng_g.current_num--;

		//slide current index, to care max
		flyweight_instance_unregist(instance_p);
		memset(instance_p, 0, sizeof(struct flyweight_instance_s));
	}
FLYWEIGHT_MNG_UNLOCK

}

void * flyweight_get(int id) {
	void *ret=NULL;

FLYWEIGHT_MNG_LOCK
	//validate id check
	if(flyweight_mng_is_valid_index(id) == FLYWEIGHT_FAILED) {
		goto end;
	}

	//get instance
	DEBUG_ERRPRINT("get[%d]:\n", id);
	ret = flyweight_instance_get_instance(&flyweight_mng_g.instances[id]);
FLYWEIGHT_MNG_UNLOCK

end:
	return ret;
}

int flyweight_set(int id, void * data, int (*setter)(void *src, size_t srcsize, void *set_data)) {

	int ret=FLYWEIGHT_FAILED;
FLYWEIGHT_MNG_LOCK
	//validate id check
	if(flyweight_mng_is_valid_index(id) == FLYWEIGHT_FAILED) {
		goto end;
	}

	//set instance
	DEBUG_ERRPRINT("set[%d]:\n", id);
	ret = flyweight_instance_set_instance(&flyweight_mng_g.instances[id], data, setter);
end:
FLYWEIGHT_MNG_UNLOCK

	return ret;
}

void flyweight_lock(int id) {
FLYWEIGHT_MNG_LOCK
	//validate id check
	if(flyweight_mng_is_valid_index(id) != FLYWEIGHT_FAILED) {
		flyweight_mutex_lock(&flyweight_mng_g.instances[id].lock_instance);
	}
FLYWEIGHT_MNG_UNLOCK
}

void flyweight_unlock(int id) {
FLYWEIGHT_MNG_LOCK
	//validate id check
	if(flyweight_mng_is_valid_index(id) != FLYWEIGHT_FAILED) {
		flyweight_mutex_unlock(&flyweight_mng_g.instances[id].lock_instance);
	}
FLYWEIGHT_MNG_UNLOCK
}
