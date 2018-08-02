/**
 * @file mement.h
 * @brief This is API for Prototype design pattern
**/
#ifndef MEMENT_H
#define MEMENT_H
#include "dp_define.h"
DP_H_BEGIN

/*! @struct mement_manager_t
 * MementRegister class member definition, defined in mement_manager.c
*/
struct mement_register_t;
/*! MementRegister class definition, member is defined in mement_manager_t */
typedef struct mement_register_t *MementRegister;

/*! @struct mement_factory_t
 * PrototypeFactory methods interface definition, to set mement_register.
*/
struct mement_factory_t;
/*! PrototypeFactory class definition, member is defined in mement_factory_t */
typedef struct mement_factory_t *PrototypeFactory;

/*! @struct mement_method_t
 * MementMethod methods interface definition, to set mement_register.
*/
struct mement_method_t {

	/**
	 * constructor
	 * @param[out] instance new instance
	 * @param[in] base base data pointer
	 * @param[in] base_length base data length
	 * @return new cloned pointer
	 * @note default: memcopy, this is sharrow copy
	 */
	void (*constructor) (void *instance, void * base, size_t base_length);

	/**
	 * copy api of a base data, to do deep copy
	 * @param[out] broken_instance broken instance pointer
	 * @param[in] base base data pointer
	 * @param[in] base_length base data length
	 * @return new cloned pointer
	 * @note default: memcopy, this is sharrow copy
	 */
	void (*copy) (void *broken_instance, void * base, size_t base_length);

	/**
	 * free api of a clone data, to do deep copy
	 * @param[in] cloned_data cloned data by using clone function pointer in this mement_factory_method_t structure
	 * @param[in] base_length base data length
	 * @return none
	 * @note default: none, free base data with MementRegister instance
	 */
	void (*free) (void * base, size_t base_length);
};

typedef struct mement_method_t mement_method_t;

/**
 * Create MementRegister class
 *
 * @param[in] base base data. 
 * @param[in] base_length base data length
 * @param[in] method method ( if NULL or member is NULL, use default. It's better to use own method)
 * @retval !NULL MementRegister instance 
 * @retval NULL error
 */
MementRegister mement_register(void * base, size_t base_length, mement_method_t * method);
/**
 * Unregister MementRegister class
 *
 * @param[in] this MementRegister instance returned at mement_manager_new,
 * @return none
 */
void mement_unregister(MementRegister this);
/**
 * Rememter mement information into broken_instance
 *
 * @param[in] this MementRegister instance returned at mement_register.
 * @param[out] broken_instance broken instance data.
 * @param[in] is_unregister_mement unregister mement or not. if !=0, unregister this data same as calling mement_unregister
 * @return none
 */
void mement_remember(MementRegister this, void * broken_instance, int is_unregister_mement);
DP_H_END
#endif
