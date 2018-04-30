#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lower_layer_director.h"
#include "ossupport_device_plugin.h"
#include "usb_device_plugin.h"

#define CONFNAME "../conf/device_plugin.conf"
#define USBPLUGIN_NAME "../sample_plugin/.libs/libusb_device_plugin.so"
#define OSSSUPPORT_PLUGIN_NAME "../sample_plugin/.libs/libossupport_device_plugin.so"

static int result_g;
static void initial_result(int result) {
	printf("<%s> result=%d\n", __FUNCTION__, result);
	result_g = result;
}

int main(int argc, char argv[]) {
	printf("If not set parameter=> try to load %s\n", USBPLUGIN_NAME);
	printf("Otherwise=> try to load %s\n", OSSSUPPORT_PLUGIN_NAME);

	char *libname;
	if(argc==1) {
		libname = USBPLUGIN_NAME;
	} else {
		libname = OSSSUPPORT_PLUGIN_NAME;
	}

	printf("load %s\n", libname);
	LowerLayerDirector director = lower_layer_director_new(libname, CONFNAME);

	ossupport_device_s osssupport_dev;
	wifi_device_s wifi_dev;
	void *device;
	if(argc==1) {
		device=&wifi_dev;
		memset(device, 0, sizeof(wifi_dev));
	} else {
		device=&osssupport_dev;
		memset(device, 0, sizeof(osssupport_dev));
	}

	printf("construct device first, wait result\n");
	lower_layer_director_construct(director, device, initial_result);
	sleep(5);

	printf("Let's try to connect!!\n");

	DevicePluginInterface interface = director->lower_layer_interface;
	interface->connect(interface);
	printf("Disconnect!!\n");
	interface->disconnect(interface);

	printf("Exit\n");
	lower_layer_director_free(director);
	return 0;
}
