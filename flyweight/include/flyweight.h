/**
 * @file flyweight.h
 * @brief This is API as Flyweight design petten
**/
#ifndef FLYWEIGHT_H_
#define FLYWEIGHT_H_

#include <stddef.h>
#include "dp_util.h"

/*! @struct FlyweightMethodsIF
 * @brief Flyweight methods interface definition, to set flyweight_factory_new. This interface is used for generating instance into FlyweightFactory.
*/
struct flyweight_methods_t {
	/**
	 * @brief constructor of class
	 * @param[in] this class instance
	 * @param[in] size size of this instance
	 * @param[in] input_parameter input parameter related to flyweight_get
	 * @note default: memcpy size
	 */
	void (*constructor)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief operand == 
	 * @param[in] this class instance
	 * @param[in] size size of this instance
	 * @param[in] input_parameter input parameter related to flyweight_get
	 * @return defined value
	 * @note if you set function which return always 1, this class is same as Singleton.
	 * @note if you set function which return always 0, this class always allocate new instance
	 * @note default: memcmp size
	 */
	int (*equall_operand)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief setter
	 * @param[in] this class instance
	 * @param[in] size size of this instance
	 * @param[in] input_parameter
	 * @note default: memcpy size
	 */
	int (*setter)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief destructor
	 * @param[in] this class instance
	 * @note allocated memory will free into library, please free members in class
	 * @note default: none
	 */
	void (*destructor)(void *this);
};
typedef struct flyweight_methods_t flyweight_methods_t, * FlyweightMethodsIF;

/*! @struct FlyweightFactory
 * @brief FlyweightFactory definition, defined in flyweight.c
*/
struct flyweight_factory_t;
typedef struct flyweight_factory_t * FlyweightFactory;

/**
 * @brief define class for flyweight
 * @param[in] instance_size size of instance which defined in user side.
 * @param[in] is_threadsafe  if !=0, ensure threadsafe to create new class instace, please set !=0 if you want to use this API on multi thread
 * @param[in] methods for generating class instance by using FlyweightFactory.
 *            If NULL, use defautlt. If not NULL, override methods. override NULL, this method is no effect.
 *            destructor is called at free
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
FlyweightFactory flyweight_factory_new(size_t class_size, int is_threadsafe, FlyweightMethodsIF methods);

/**
 * @brief getter
 * @param[in] classHandle class handle returned at flyweight_register_class, first time to call get, allocate class, memset 0 and call constructor
 * @param[in] constructor_parameter constructor parameter
 * @retval !NULL class instance
 * @retval NULL id is invalid
 */
void * flyweight_get(FlyweightFactory this, void * constructor_parameter);

/**
 * @brief setter
 * @param[in] classHandle class handle returned at flyweight_register_class
 * @param[in] constructor_parameter constructor parameter
 * @param[in] data set data pointer
 * @param[in] setter setter if you want to change setter ( if NULL, use setter related to flyweight_register_class input)
 */
int flyweight_set(FlyweightFactory this, void * constructor_parameter, void * data, int (*setter)(void *this, size_t size, void *input_parameter));

/**
 * @brief clear class handle
 * @param [in] classHandle class handle returned at flyweight_register_class
 * @turn none
 */
void flyweight_factory_free(FlyweightFactory this);
#endif/*FLYWEIGHT_*/
