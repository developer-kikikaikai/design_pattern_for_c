#ifndef LOWER_LAYER_BUILDER_LIB_
#define LOWER_LAYER_BUILDER_LIB_
#define LL_BUILDER_SUCCESS (0)
#define LL_BUILDER_FAILED (-1)
/**
 * @brief new builder interface
 * @param[in] none
 * @retval !=NULL  this lower interface class if lower library has it.
 * @retval NULL not implement interface
 */
void * lower_layer_builder_instance_new(void);
#define LL_BUILDER_NEWNAME "lower_layer_builder_instance_new"

/**
 * @brief free builder interface
 * @param[in] interfaceClass
 */
void lower_layer_builder_instance_free(void *interfaceClass);
#define LL_BUILDER_FREENAME "lower_layer_builder_instance_free"
#endif
