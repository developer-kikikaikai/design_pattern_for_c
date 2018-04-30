#include "ossupport_device_plugin.h"
#include "lower_layer_builder.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define DEBUG_PRINT(...)  printf("[ethernet] ");printf(__VA_ARGS__);

typedef struct osssupport_dp_interface {
DEVICE_PLUGIN_INTERFACE
	char gateway[64];
	int retry_time;
	int outsize;
} osssupport_device_plugin_interface_t, *OSSSupportDPInterface;

static void ossupport_device_connect(void * handle) {
	OSSSupportDPInterface this = (OSSSupportDPInterface) handle;

	DEBUG_PRINT("set gateway %s!!\n", this->gateway);
	DEBUG_PRINT("check connection to WAN!\n");
	int i=0;
	for(i = 0; i < this->retry_time; i++ ) {
		if( this->outsize ) {
			DEBUG_PRINT("Success to connect!\n");
			return;
		} else {
			DEBUG_PRINT("Failed to connect, try [%d/%d]\n", i+1 , this->retry_time);
			sleep(1);
		}
	}

	DEBUG_PRINT("Failed to connect, retry time %d\n", i );
	return;
}

static void ossupport_device_disconnect(void * handle) {
	OSSSupportDPInterface this = (OSSSupportDPInterface) handle;
	DEBUG_PRINT("remove gateway %s!!\n", this->gateway);
}

//initialize device, as set IP, ...
int llbuilder_initial_device(void * arg) {
	ossupport_device_s *device = (ossupport_device_s *)arg;
	DEBUG_PRINT("initialize device!!\n");
	sprintf(device->device_name, "eth0");
	DEBUG_PRINT("ifup %s!!\n", device->device_name);
	sprintf(device->ipaddress, "192.168.56.100");
	DEBUG_PRINT("set ip address %s!!\n", device->ipaddress);
	return 0;
}

//don't need load device, not implement!

void * lower_layer_builder_instance_new(void) {
	OSSSupportDPInterface instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->connect = ossupport_device_connect;
	instance->disconnect = ossupport_device_disconnect;
	sprintf(instance->gateway, "192.168.56.1");

	instance->retry_time = 3;
	instance->outsize = time(NULL)%2;
	return instance;
}

void lower_layer_builder_instance_free(void *interfaceClass) {
	free(interfaceClass);
}
