/**
 * @file memorypool.h
 * This is API for memory pool
**/
#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include "dp_define.h"
DP_H_BEGIN
/*! @struct memorypool_t
 * @brief MemoryPool class member definition, detail is defined in C file.
*/
struct memorypool_t;
/*! MemoryPool class definition, to use API */
typedef struct memorypool_t * MemoryPool;

/**
 * create MemoryPool class
 *
 * @param[in] max_size max of allocated memory size, to allocate fast, this value will update 2^n in library.
 * @param[in] max_cnt number of allocated memory
 * @param[in] is_multithread  threadsafe flag. If you want to use it on multi thread, please set 1.
 * @param[in] constructor if you want to initialize memory first.
 * @param[in] constructor_parameter constructor parameter if you want to initialize memory first.
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
MemoryPool mpool_create(size_t max_size, size_t max_cnt, int is_multithread, void (*constructor)(void * this, void *constructor_parameter), void *constructor_parameter);
/**
 * free MemoryPool class
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @param[in] destructor if you want to finialize memory
 * @return none
 */
void mpool_delete(MemoryPool this, void (*destructor)(void *));

/**
 * get memory from pool
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @retval !=NULL  allocated pointer
 * @retval NULL    max_size of allocated memory
 * @note If already get all allocated pointer, return NULL
 */
void * mpool_get(MemoryPool this);
/**
 * get used memory
 *
 * @param[in] this MemoryPool instance return,
 * @param[in] ptr  used pointer
 * @retval !=NULL  get next
 * @retval NULL    this pointer is end
 */
void * mpool_get_next_usedmem(MemoryPool this, void * ptr);

/*! define for used loop */
#define FOR_ALL_USEDMEM(this,ptr) for(ptr=mpool_get_next_usedmem(this, NULL); ptr!=NULL; ptr = mpool_get_next_usedmem(this, ptr ))

/**
 * get used cnt
 *
 * @param[in] this MemoryPool instance return,
 * @return used count
 */
size_t mpool_get_usedcnt(MemoryPool this);
/**
 * release memory to pool
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @param[in] ptr allocated pointer which get from mpool_malloc
 * @return none
 * @note not initialize memory
 */
void mpool_release(MemoryPool this, void * ptr);
DP_H_END
#endif
