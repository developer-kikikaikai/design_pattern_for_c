/**
 * @file memorypool.h
 * This is API for memory pool
**/
#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
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
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
MemoryPool mpool_create(size_t max_size, size_t max_cnt, int is_multithread);
/**
 * free MemoryPool class
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @return none
 */
void mpool_delete(MemoryPool this);
/**
 * get memory from pool as malloc
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @param[in] size allocates size bytes
 * @retval !=NULL  allocated pointer
 * @retval NULL    error
 * @note Difference of malloc: all of return memories are initialize by 0.
 * @note If size > max_size of allocated memory, or all allocated memories are already used, call calloc
 */
void * mpool_malloc(MemoryPool this, size_t size);
/*! get memory from pool as calloc, this is same as mpool_malloc */
#define mpool_calloc(this, nmemb, size) mpool_malloc(this, nmemb*size)
/**
 * free memory as free
 *
 * @param[in] this MemoryPool instance returned at mpool_malloc_new,
 * @param[in] ptr allocated pointer which get from mpool_malloc
 * @return none
 */
void mpool_free(MemoryPool this, void * ptr);
#endif
