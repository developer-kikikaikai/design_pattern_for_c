#ifndef OSSUPPORT_DEVICE_PLUGIN_
#define OSSUPPORT_DEVICE_PLUGIN_
//interface definition is here
#include "device_plugin_if.h"

/*! @struct 
 * @brief ossupport_device, have to use ssid and key
*/
typedef struct ossupport_device_ {
	char device_name[64];
	char ipaddress[64];
} ossupport_device_s;
#endif
