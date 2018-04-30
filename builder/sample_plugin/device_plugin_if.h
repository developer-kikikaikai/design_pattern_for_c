#ifndef DEVICE_PLUGIN_IF_
#define DEVICE_PLUGIN_IF_

/*! @Class
 * @brief device plugin interface, all plugin use it
*/
typedef struct device_plugin_interface {
	void (*connect)(void * this);
	void (*disconnect)(void * this);
} device_plugin_interface_t, *DevicePluginInterface;

#define DEVICE_PLUGIN_INTERFACE \
	void (*connect)(void * this);\
	void (*disconnect)(void * this);
#endif
