#ifndef USB_WIFI_DEVICE_PLUGIN_
#define USB_WIFI_DEVICE_PLUGIN_
//interface definition is here
#include "device_plugin_if.h"

/*! @struct 
 * @brief wifie_device, have to use ssid and key
*/
typedef struct wifi_device_ {
	char device_name[64];
	char ipaddress[64];
	char ssid[64];
} wifi_device_s;
#endif
