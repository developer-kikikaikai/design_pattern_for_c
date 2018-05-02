#ifndef DEVICE_PLUGIN_IF_
#define DEVICE_PLUGIN_IF_

/*! @Class
 * @brief device plugin interface, all plugin use it
*/
struct device_plugin_interface;
typedef struct device_plugin_interface device_plugin_interface_t, *DevicePluginInterface;

struct device_plugin_interface {
	void (*connect)(DevicePluginInterface this);
	void (*disconnect)(DevicePluginInterface this);
};

//define LowerLayerInterface by DevicePluginInterface

#define DEVICE_PLUGIN_INTERFACE \
	void (*connect)(DevicePluginInterface this);\
	void (*disconnect)(DevicePluginInterface this);
#endif
