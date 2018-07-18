#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "memorypool.h"

/*! @name index slide API */
/* @{ */
/*! get right bit index */
static int get_far_right_bit_index(uint64_t data) {
	int index=0;
	for(index=0;index< 64;index++) {
		if((data)&(0x01<<index))break;
	}
	return index;
}

/*! get minimum of x, which 2^x > input_value*/
static int get_bit_digit_index_over_size(uint64_t size) {
	uint64_t current_size=size;
	int digit = 0, tmp_digit=0;
	while(current_size != 0) {
		tmp_digit = get_far_right_bit_index(current_size);
		digit += tmp_digit + 1;
		current_size = current_size >> tmp_digit + 1;
	}

	if(digit == get_far_right_bit_index(size) + 1) digit--;
	return digit;
}

/* @} */

/*! @struct memorypool_t*
 *  definition of malloc manage data, to search data fast, add buffer_unused list and unfull list for check buffer_unused (because buffer_unused will over 
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
static inline void mpool_unuse_memory(MemoryPool this, void *ptr);
static inline uint64_t mpool_get_buffer_place(MemoryPool this, uint8_t * buffer_list, void * ptr);
static inline int mpool_is_not_ptr_in_buf(MemoryPool this, void * ptr);

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
	if(!this->head->used) {
		memory = this->head;
		mpool_list_pull(this,  memory);
		memory->used=1;
		mpool_list_push(this, memory);
		return memory->mem;
	} else {
		printf("nused\n");
		return NULL;
	}
}

static inline void * mpool_get_next_memory(MemoryPool this, void * ptr) {
	malloc_data_t * memory=NULL;

	/* get memory place */
	if(!ptr) {
		memory=this->tail;
	} else {
		uint64_t place = mpool_get_buffer_place(this, this->user_buf, ptr);
		memory= ((malloc_data_t *)(this->buf) + place);
		memory = memory->prev;
	}

	/* check used flag */
	if(memory && memory->used) return memory->mem;
	else return NULL;
}

static inline void mpool_unuse_memory(MemoryPool this, void *ptr) {
	//get plage of memory, and get list from this place
	uint64_t place = mpool_get_buffer_place(this, this->user_buf, ptr);
	malloc_data_t * memory = (malloc_data_t *)(this->buf + (sizeof(malloc_data_t) * place));

	mpool_list_pull(this, memory);
	memory->used=0;
	mpool_list_head(this, memory);
}

static inline uint64_t mpool_get_buffer_place(MemoryPool this, uint8_t * buffer_list, void * ptr) {
	//this place is XXX00000 bit place, slide by using this->slide_bit
	return  ((uint64_t)ptr - (uint64_t)buffer_list) >> this->slide_bit;
}

static inline int mpool_is_not_ptr_in_buf(MemoryPool this, void * ptr) {
	
	return (uint64_t)ptr < (uint64_t)this->user_buf || (uint64_t)(this->user_buf) + this->max_cnt * this->max_size < (uint64_t)ptr;
}
/* @} */

/*! @name public API*/
/* @{ */
MemoryPool mpool_create(size_t max_size, size_t max_cnt, int is_multithread, void (*constructor)(void *this, void *parameter), void * constructor_parameter) {
	//create magic hash table first
	int ret=-1;
	size_t slide_bit;

	MemoryPool instance;
	size_t mutex_size=0;
	if(is_multithread) mutex_size = sizeof(pthread_mutex_t);

	//get slide bit to use hash
	slide_bit = get_bit_digit_index_over_size(max_size);

	//Update max size
	max_size = 0x01 << slide_bit;

	instance = malloc(sizeof(*instance) + mutex_size + max_cnt * (sizeof(malloc_data_t) + max_size));
	if(!instance) return NULL;
	memset(instance, 0, sizeof(*instance) + mutex_size + max_cnt * (sizeof(malloc_data_t) + max_size));

	instance->slide_bit = slide_bit;
	instance->max_size = max_size;	
	instance->max_cnt = max_cnt;
	void *correut = (instance + 1);
	//set lock
	if(is_multithread) {
		instance->lock=(pthread_mutex_t *) (correut);
		correut = instance->lock + 1;
		pthread_mutex_init(instance->lock, NULL);
	} else {
		instance->lock = NULL;
	}

	//keep pointer line, head is list of malloc_data_t
	instance->buf = (uint8_t *)(correut);
	correut = instance->buf + max_cnt * sizeof(malloc_data_t);

	//set user pointer list
	instance->user_buf = correut;

	//set user pointer list
	malloc_data_t * memory;
	int i=0;
	for(i=0;i<max_cnt;i++) {
		memory = (malloc_data_t *)(instance->buf + (sizeof(malloc_data_t) * i));
		memory->mem = correut;
		correut = memory->mem + max_size;
		if(constructor) constructor(memory->mem, constructor_parameter);
		mpool_list_push(instance, memory);
	}

	return instance;
}

void mpool_delete(MemoryPool this, void (*destructor)(void *)) {
MPOOL_LOCK(this)
	if(destructor) {
		malloc_data_t * memory = this->head;
		while(memory) {
			destructor(memory->mem);
			memory = memory->next;
		}
	}
MPOOL_UNLOCK
	free(this);
}

void * mpool_get(MemoryPool this) {
	void * mem=NULL;
MPOOL_LOCK(this)
	mem = mpool_get_memory(this);
	if(mem) this->cur_cnt++;
MPOOL_UNLOCK
	return mem;
}

void * mpool_get_next_usedmem(MemoryPool this, void * ptr) {
	if(ptr && mpool_is_not_ptr_in_buf(this, ptr)) {
		return NULL;
	}

	void * mem=NULL;
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
	if(!ptr) return;

	if(mpool_is_not_ptr_in_buf(this, ptr)) {
		//no info
		return;
	}
MPOOL_LOCK(this)
	mpool_unuse_memory(this, ptr);
	this->cur_cnt--;
MPOOL_UNLOCK
}

void mpool_show(MemoryPool this) {
MPOOL_LOCK(this)
	malloc_data_t *memory=this->head;
	while(memory) {
		fprintf(stderr, "used:%d\n", memory->used);
		memory = memory->next;
	}
MPOOL_UNLOCK
}
/* @} */
