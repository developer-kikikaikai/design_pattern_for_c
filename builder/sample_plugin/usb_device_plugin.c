#include "usb_device_plugin.h"
#include "lower_layer_builder.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG_PRINT(...)  printf("[wifi] ");printf(__VA_ARGS__);

typedef struct usb_dp_interface {
DEVICE_PLUGIN_INTERFACE
	char gateway[64];
} usb_device_plugin_interface_t, *USBDPInterface;

USBDPInterface usb_dp_interface_g=NULL;

static void usd_device_connect(DevicePluginInterface handle) {
	USBDPInterface this = (USBDPInterface) handle;

	DEBUG_PRINT("check connection to gateway!!");
	DEBUG_PRINT("Success to connect!, gateway IP:[%s]\n", this->gateway);
}

static void usd_device_disconnect(DevicePluginInterface handle) {
	DEBUG_PRINT("disconnect!!");
}

int llbuilder_load_driver(void * arg) {
	wifi_device_s *device = (wifi_device_s *)arg;
	DEBUG_PRINT("load device driver related to %s device!!\n", device->device_name);

	sleep(1);

	DEBUG_PRINT("Success to load device driver, we can use %s device!!\n", device->device_name);
	return 0;
}

int llbuilder_initial_device(void * arg) {
	wifi_device_s *device = (wifi_device_s *)arg;
	DEBUG_PRINT("Scanning access point!!\n");
	int i=0;
	for(i = 0; i < 10; i ++) {
		printf(".");
		usleep(100);
	}

	sprintf(device->ssid, "sampleSSID");
	DEBUG_PRINT("Find access point!! ssid[%s]\n", device->ssid);

	sprintf(device->device_name, "wlan0");
	DEBUG_PRINT("ifup %s!!\n", device->device_name);
	sprintf(device->ipaddress, "192.168.100.100");
	DEBUG_PRINT("get ip address %s from Access point!!\n", device->ipaddress);

	sprintf(usb_dp_interface_g->gateway, "192.168.100.1");
	return 0;
}

void * lower_layer_builder_instance_new(void) {
	usb_dp_interface_g = calloc(1, sizeof(*usb_dp_interface_g));
	usb_dp_interface_g->connect=usd_device_connect;
	usb_dp_interface_g->disconnect=usd_device_disconnect;
	return usb_dp_interface_g;
}

void lower_layer_builder_instance_free(void * interfaceClass) {
	free(interfaceClass);
}
