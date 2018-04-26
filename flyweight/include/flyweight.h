#ifndef FLYWEIGHT_
#define FLYWEIGHT_
/**
 * @brief This is API as Flyweight design petten
**/
#include <stddef.h>

/*! @struct flyweight_class_method_s
 * @brief flyweight class method definition
*/
struct flyweight_class_methods_s {
	/**
	 * @brief constructor of class
	 * @param[in] this this allocated memory
	 * @param[in] size size of this instance
	 * @param[in] input_parameter input parameter related to flyweight_get
	 */
	void (*constructor)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief operand == 
	 * @param[in] this this allocated memory
	 * @param[in] size size of this instance
	 * @param[in] input_parameter input parameter related to flyweight_get
	 * @note if you set function which return always 1, this class is same as Singleton.
	 * @note if you set function which return always 0, this class always allocate new instance
	 */
	int (*equall_operand)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief setter
	 * @param[in] this this allocated memory
	 * @param[in] size size of this instance
	 * @param[in] input_parameter
	 */
	int (*setter)(void *this, size_t size, void *input_parameter);
	/**
	 * @brief destructor
	 * @param[in] this this allocated memory
	 * @note allocated memory will free into library, please free members in class
	 */
	void (*destructor)(void *this);
};

/**
 * @brief define class for flyweight
 *
 * @param[in] class_length size of class, C have to know size to allocate memory, 
 * @param[in] is_threadsafe  if !=0, ensure threadsafe to create new class instace, please set !=0 if you want to use this API on multi thread
 * @param[in] methods prime method for this class.
 *            If NULL, use defautlt. If not NULL, override methods. override NULL, this method is no effect.
 *            destructor is called at flyweight_unregister_class or exit
 * @retval !=NULL  this class handle
 * @retval NULL error
 */
void * flyweight_define_class(size_t class_size, int is_threadsafe, struct flyweight_class_methods_s *methods);

/**
 * @brief getter
 *
 * @param[in] classHandle class handle returned at flyweight_register_class, first time to call get, allocate class, memset 0 and call constructor
 * @retval !NULL class instance
 * @retval NULL id is invalid
 */
void * flyweight_get(void * classHandle, void * constructor_parameter);

/**
 * @brief setter
 *
 * @param[in] classHandle class handle returned at flyweight_register_class
 * @param[in] data set data pointer
 * @param[in] setter setter if you want to change setter ( if NULL, use setter related to flyweight_register_class input)
 */
int flyweight_set(void * classHandle, void * constructor_parameter, void * data, int (*setter)(void *this, size_t size, void *input_parameter));

/**
 * @brief clear class handle
 * @brief exit
 *
 * @param none
 * @retval none
 */
void flyweight_clear(void * classHandle);
#endif/*FLYWEIGHT_*/
