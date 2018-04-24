#ifndef FLYWEIGHT_
#define FLYWEIGHT_
/**
 * @brief This is API as Flyweight design petten
**/

/*! @struct flyweight_init_s
 * @brief flyweight initialize structure
*/
struct flyweight_init_s {
	void (*constructor)(void *src); /*! constructor for this class, src is instance of this class */
	int (*setter)(void *src, size_t srcsize, void *dist); /*! setters for this class, src is instance of this class, dist is input set data */
	void (*destructor)(void *src); /*! destructor for this class, src is instance of this class */
};

/**
 * @brief flyweight_set_storagemax, if you want to regist many class, please set max registerd number.
 *        Please call it before call flyweight_register_class.
 *        If this API is not called before calling flyweight_register_class, max registerd number is default.
 *
 * @param[in] max_reg_num max size
 * @retval !=0 success to set
 * @retval 0 failed to set, because max registor num is big
 */
void flyweight_set_storagemax(int max_reg_num);

/**
 * @brief register class
 *
 * @param[in] class_length size of class, C have to know size to allocate memory, 
 * @param[in] is_threadsafe  if !=0, ensure threadsafe by please set !=0 if you want to use this instance on multi thread
 * @param[in] methods prime method for this class.
 *            If NULL, constructor->no effect, setter->memcpy size, destructor->no effect.
 *            If not NULL but parameter is NULL, this paramter is no effect at constructor/set/destructor/
 *            destructor is called at flyweight_unregister_class.
 * @retval 0<  this class id
 * @retval other error
 */
int flyweight_register_class(size_t class_size, int is_threadsafe, struct flyweight_init_s *methods);

/**
 * @brief unregister class
 *
 * @param[in] id class id returned at flyweight_register_class
 * @return none, call destructor at this API and allocate memory after calling destructor
 */
void flyweight_unregister_class(int id);

/**
 * @brief getter
 *
 * @param[in] id class id returned at flyweight_register_class, first time to call get, allocate class, memset 0 and call constructor
 * @retval !NULL class instance
 * @retval NULL id is invalid
 */
void * flyweight_get(int id);

/**
 * @brief setter
 *
 * @param[in] id class id returned at flyweight_register_class
 * @param[in] data set data pointer
 * @param[in] setter setter if you want to change setter, if NULL, use setter related to flyweight_register_class input)
 * @return return value related to setter, default: always return 0
 * @note  setter change common instance data, so change value of instance by getting flyweight_get if this API call after getting it. Please care about global data.
 */
int flyweight_set(int id, void * data, int (*setter)(void *src, size_t srcsize, void *set_data));

/**
 * @brief exit, free all alocated data
 *
 * @param none
 * @retval none
 */
void flyweight_exit(void);

/**
 * @brief lock method, if you want to lock data between using flyweight_get instance, please call it and flyweight_unlock after using.
 *
 * @param[in] id class id
 * @note  Maybe it's better only to use getter for Flyweight pattern.
 */
void flyweight_lock(int id);
/**
 * @brief unlock method, if you call flyweight_lock, you "MUST" call it.
 *
 * @param[in] id class id
 * @note  Maybe it's better only to use getter for Flyweight pattern.
 */
void flyweight_unlock(int id);
#endif/*FLYWEIGHT_*/
