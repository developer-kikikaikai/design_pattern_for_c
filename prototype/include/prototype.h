/**
 * @file prototype.h
 * @brief This is API for Prototype design pattern
**/
#ifndef PROTOTYPE_H
#define PROTOTYPE_H
#include "dp_define.h"
DP_H_BEGIN

/*! @struct prototype_manager_t
 * PrototypeManager class member definition, defined in prototype_manager.c
*/
struct prototype_manager_t;
/*! PrototypeManager class definition, member is defined in prototype_manager_t */
typedef struct prototype_manager_t *PrototypeManager;

/*! @struct prototype_factory_t
 * PrototypeFactory methods interface definition, to set prototype_register.
*/
struct prototype_factory_t;
/*! PrototypeFactory class definition, member is defined in prototype_factory_t */
typedef struct prototype_factory_t *PrototypeFactory;

#define PROTOTYPE_SUCCESS (0)
#define PROTOTYPE_FAILED (-1)

/*! @struct prototype_factory_method_t
 * PrototypeFactory methods interface definition, to set prototype_register.
*/
struct prototype_factory_method_t {
	/**
	 * clone api of a base data, to do deep copy
	 * @param[in] base base data pointer
	 * @param[in] base_length base data length
	 * @return new cloned pointer
	 * @note default: memcopy, this is sharrow copy
	 */
	void * (*clone) (void * base, size_t base_length);
	/**
	 * free api of a clone data, to do deep copy
	 * @param[in] cloned_data cloned data by using clone function pointer in this prototype_factory_method_t structure
	 * @param[in] base_length base data length
	 * @return none
	 * @note default: free
	 */
	void (*free) (void * cloned_data);
	/**
	 * free api of a base data, please free deep data
	 * @param[in] base_data base data registered by prototype_register
	 * @return none
	 * @note default: free
	 */
	void (*free_base) (void * base_data);
};

typedef struct prototype_factory_method_t prototype_factory_method_t;

/**
 * Create PrototypeManager class
 *
 * @param[in] is_threadsafe  if !=0, ensure threadsafe to create new class instace
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
PrototypeManager prototype_manager_new(int is_threadsafe);
/**
 * free class handle
 * @param [in] this PrototypeManager instance returned at prototype_manager_new,
 * @return none
 */
void prototype_manager_free(PrototypeManager this);

/**
 * Register PrototypeFactory class
 *
 * @param[in] this PrototypeManager instance returned at prototype_manager_new,
 * @param[in] base base data. 
 * @param[in] base_length base data length
 * @param[in] factory_method factory method ( if NULL or member is NULL, use default. It's better to use free_basedata)
 * @retval !NULL PrototypeFactory instance 
 * @retval NULL error
 * @note Please keep instance of base. This will free into prototype_unregister API by using free_basedata
 * 
 */
PrototypeFactory prototype_register(PrototypeManager this, void * base, size_t base_length, prototype_factory_method_t * factory_method);

/**
 * Unregister PrototypeFactory class
 *
 * @param[in] this PrototypeManager instance returned at prototype_manager_new,
 * @param[in] factory PrototypeFactory instance returned at prototype_register,
 * @return none
 */
void prototype_unregister(PrototypeManager this, PrototypeFactory factory);
/**
 * Clone base pointer by using PrototypeFactory class
 *
 * @param[in] this PrototypeFactory instance returned at prototype_register.
 * @return cloned data or NULL
 */
void * prototype_clone(PrototypeFactory this);
/**
 * Free cloned pointer
 *
 * @param[in] this PrototypeFactory instance returned at prototype_register.
 * @param[in] cloned_data free data returned at prototype_clone.
 * @return none
 */
void prototype_free(PrototypeFactory this, void * cloned_data);
DP_H_END
#endif
