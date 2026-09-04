#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <pthread.h>
#include "memorypool.h"

/*! @name index slide API */
/* @{ */
/*! round size up to a power of two without overflowing size_t */
static int mpool_round_block_size(size_t requested, size_t *rounded, size_t *slide_bit) {
	size_t value;
	size_t bit = 0;

	if(!requested || !rounded || !slide_bit) return -1;

	value = requested - 1;
	while(value) {
		value >>= 1;
		bit++;
	}
	if(bit >= sizeof(size_t) * CHAR_BIT) return -1;

	*rounded = ((size_t)1) << bit;
	*slide_bit = bit;
	return 0;
}

/* @} */

/*! @struct memorypool_t
 *  definition of malloc manage data, to search data fast */
/* @{ */
typedef struct maloc_data_t {
	struct maloc_data_t * next;
	struct maloc_data_t * prev;
	int used;
	void * mem;
} malloc_data_t;

typedef struct memorypool_t {
	malloc_data_t * head;
	malloc_data_t * tail;
	size_t max_size;
	size_t max_cnt;
	size_t cur_cnt;
	size_t slide_bit;/*!< keep slide bit size related to max_size, to search buffer fast*/
	uint8_t * buf;/*!< list of buffer for user + list of malloc_data_t*/
	uint8_t * user_buf;/*!< list of buffer for user */
	pthread_mutex_t *lock;/*!< mutex */
} memorypool_t;

static inline void mpool_list_push(MemoryPool this, malloc_data_t * data);
static inline void mpool_list_pull(MemoryPool this, malloc_data_t * data);
static inline void mpool_list_head(MemoryPool this, malloc_data_t * data);
static inline void * mpool_get_memory(MemoryPool this);
static inline void * mpool_get_next_memory(MemoryPool this, void * ptr);
static inline void mpool_unuse_memory(MemoryPool this, malloc_data_t *memory);
static inline malloc_data_t *mpool_find_memory(MemoryPool this, void *ptr);

/*define for pthread_cleanup_push*/
static inline void pthread_mutex_unlock_(void *arg) {
	if(arg) pthread_mutex_unlock((pthread_mutex_t *)arg);
}

static inline void pthread_mutex_lock_(MemoryPool this) {
	if(this->lock)  pthread_mutex_lock(this->lock);
}
#define MPOOL_LOCK(this) \
	pthread_mutex_lock_(this);\
	pthread_cleanup_push(pthread_mutex_unlock_, this->lock);
#define MPOOL_UNLOCK pthread_cleanup_pop(1);

/*! @name private API for memorypool_t*/
/* @{ */

static inline void mpool_list_push(MemoryPool this, malloc_data_t * data) {
	/* add to tail */
	data->prev = this->tail;
	data->next = NULL;
	//slide tail
	if(this->tail) this->tail->next = data;

	this->tail = data;
	/* if head is null, set to head */
	if(!this->head) this->head = data;
}

static inline void mpool_list_pull(MemoryPool this, malloc_data_t * data) {
	if(!data) return;

	/* update content */
	if(this->head == data) this->head = data->next;
	/* else case, account is not head. So there is a prev. */
	else data->prev->next = data->next;

	if(this->tail == data) this->tail = data->prev;
	/* else case, account is not tail. So there is a next. */
	else data->next->prev = data->prev;
}

static inline void mpool_list_head(MemoryPool this, malloc_data_t * data) {

	//there is a prev.
	data->next = this->head;
	data->prev = NULL;
	this->head->prev = data;
	this->head = data;
}

static inline void * mpool_get_memory(MemoryPool this) {
	malloc_data_t * memory=NULL;
	if(this->head && !this->head->used) {
		memory = this->head;
		mpool_list_pull(this,  memory);
		memory->used=1;
		mpool_list_push(this, memory);
		return memory->mem;
	} else {
		printf("used\n");
		return NULL;
	}
}

static inline void * mpool_get_next_memory(MemoryPool this, void * ptr) {
	malloc_data_t * memory=NULL;

	/* get memory place */
	if(!ptr) {
		memory=this->tail;
	} else {
		memory = mpool_find_memory(this, ptr);
		if(!memory || !memory->used) return NULL;
		memory = memory->prev;
	}

	/* check used flag */
	if(memory && memory->used) return memory->mem;
	else return NULL;
}

static inline void mpool_unuse_memory(MemoryPool this, malloc_data_t *memory) {
	mpool_list_pull(this, memory);
	memory->used=0;
	mpool_list_head(this, memory);
}

static inline malloc_data_t *mpool_find_memory(MemoryPool this, void *ptr) {
	uintptr_t start;
	uintptr_t address;
	size_t offset;
	size_t place;

	if(!this || !ptr) return NULL;
	start = (uintptr_t)this->user_buf;
	address = (uintptr_t)ptr;
	if(address < start) return NULL;

	offset = (size_t)(address - start);
	if(offset >= this->max_cnt * this->max_size || offset % this->max_size) return NULL;

	place = offset >> this->slide_bit;
	if(place >= this->max_cnt) return NULL;
	return ((malloc_data_t *)this->buf) + place;
}
/* @} */

/*! @name public API*/
/* @{ */
MemoryPool mpool_create(size_t max_size, size_t max_cnt, int is_multithread, void (*constructor)(void *this, void *parameter), void * constructor_parameter) {
	size_t slide_bit;
	size_t metadata_size;
	size_t user_size;
	size_t allocation_size;
	MemoryPool instance;
	size_t mutex_size=0;
	if(is_multithread) mutex_size = sizeof(pthread_mutex_t);

	if(!max_cnt || mpool_round_block_size(max_size, &max_size, &slide_bit)) return NULL;
	if(max_cnt > SIZE_MAX / sizeof(malloc_data_t)) return NULL;
	metadata_size = max_cnt * sizeof(malloc_data_t);
	if(max_cnt > SIZE_MAX / max_size) return NULL;
	user_size = max_cnt * max_size;
	if(sizeof(*instance) > SIZE_MAX - mutex_size) return NULL;
	allocation_size = sizeof(*instance) + mutex_size;
	if(allocation_size > SIZE_MAX - metadata_size) return NULL;
	allocation_size += metadata_size;
	if(allocation_size > SIZE_MAX - user_size) return NULL;
	allocation_size += user_size;

	instance = calloc(1, allocation_size);
	if(!instance) return NULL;

	instance->slide_bit = slide_bit;
	instance->max_size = max_size;	
	instance->max_cnt = max_cnt;
	uint8_t *current = (uint8_t *)(instance + 1);
	//set lock
	if(is_multithread) {
		instance->lock=(pthread_mutex_t *) (current);
		current = (uint8_t *)(instance->lock + 1);
		if(pthread_mutex_init(instance->lock, NULL)) {
			free(instance);
			return NULL;
		}
	} else {
		instance->lock = NULL;
	}

	//keep pointer line, head is list of malloc_data_t
	instance->buf = (uint8_t *)(current);
	current = instance->buf + metadata_size;

	//set user pointer list
	instance->user_buf = current;

	//set user pointer list
	malloc_data_t * memory;
	size_t i=0;
	for(i=0;i<max_cnt;i++) {
		memory = (malloc_data_t *)(instance->buf + (sizeof(malloc_data_t) * i));
		memory->mem = current;
		current = (uint8_t *)memory->mem + max_size;
		if(constructor) constructor(memory->mem, constructor_parameter);
		mpool_list_push(instance, memory);
	}

	return instance;
}

void mpool_delete(MemoryPool this, void (*destructor)(void *)) {
	pthread_mutex_t *lock;
	if(!this) return;
	lock = this->lock;
MPOOL_LOCK(this)
	if(destructor) {
		malloc_data_t * memory = this->head;
		while(memory) {
			destructor(memory->mem);
			memory = memory->next;
		}
	}
MPOOL_UNLOCK
	if(lock) {
		pthread_mutex_destroy(lock);
	}
	free(this);
}

void * mpool_get(MemoryPool this) {
	void * mem=NULL;
	if(!this) return NULL;
MPOOL_LOCK(this)
	mem = mpool_get_memory(this);
	if(mem) this->cur_cnt++;
MPOOL_UNLOCK
	return mem;
}

void * mpool_get_next_usedmem(MemoryPool this, void * ptr) {
	void * mem=NULL;
	if(!this) return NULL;
MPOOL_LOCK(this)
	mem = mpool_get_next_memory(this, ptr);
MPOOL_UNLOCK
	return mem;
}

size_t mpool_get_usedcnt(MemoryPool this) {
	if(!this) return 0;
	size_t retval=0;
MPOOL_LOCK(this)
	retval = this->cur_cnt;
MPOOL_UNLOCK
	return retval;
}

void mpool_release(MemoryPool this, void * ptr) {
	malloc_data_t *memory;
	if(!this || !ptr) return;
MPOOL_LOCK(this)
	memory = mpool_find_memory(this, ptr);
	if(memory && memory->used) {
		mpool_unuse_memory(this, memory);
		this->cur_cnt--;
	}
MPOOL_UNLOCK
}

void mpool_show(MemoryPool this) {
	if(!this) return;
MPOOL_LOCK(this)
	malloc_data_t *memory=this->head;
	while(memory) {
		fprintf(stderr, "used:%d\n", memory->used);
		memory = memory->next;
	}
MPOOL_UNLOCK
}
/* @} */
