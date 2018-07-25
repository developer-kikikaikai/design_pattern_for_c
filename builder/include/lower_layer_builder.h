/**
 * @file lower_layer_builder.h
 * @brief This is API name definition for lower layer plugin library
lower_layer_builder will load plugin defined this API. (this API will load by libbuilder.so as dynamic library)
 **/
#ifndef LOWER_LAYER_BUILDER_LIB_H_
#define LOWER_LAYER_BUILDER_LIB_H_
#include "dp_define.h"
DP_H_BEGIN

/*! result code : success */
#define LL_BUILDER_SUCCESS (0)
/*! result code : error */
#define LL_BUILDER_FAILED (-1)

/**
 * @brief new builder interface
 * @retval !=NULL  this lower plugin interface class instance if lower library has it.
 * @retval NULL not implement interface
 */
void * lower_layer_builder_instance_new(void);
/**
 * @brief name definition of plugin interface "new"
 */
#define LL_BUILDER_NEWNAME "lower_layer_builder_instance_new"

/**
 * @brief free builder interface
 * @param[in] interfaceClass lower plugin interface class instance 
 */
void lower_layer_builder_instance_free(void * interfaceClass);
/**
 * @brief name definition of plugin interface "free"
 */
#define LL_BUILDER_FREENAME "lower_layer_builder_instance_free"

DP_H_END
#endif
