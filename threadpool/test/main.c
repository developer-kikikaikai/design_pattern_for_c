#include <stdlib.h>
#include "config.h"
#include "test_main.h"

int main(int argc, char *argv[]) {
DPDEBUG_INIT_THREADSAFE
	static const char * pluginlist[] = {
		EVENT_IF_PLUGIN_PATH"/libevent_if_libev.so",
		EVENT_IF_PLUGIN_PATH"/libevent_if_libevent.so",
		EVENT_IF_PLUGIN_PATH"/libevent_if_epoll.so",
		EVENT_IF_PLUGIN_PATH"/libevent_if_select.so",
		NULL,
	};

	size_t i=0;
	for(i = 0; i < sizeof(pluginlist)/sizeof(pluginlist[0]); i++) {
		if(test_main(pluginlist[i])) {
			DEBUG_ERRPRINT("####plugin %s error!!\n", (pluginlist[i]==NULL)?"Default":pluginlist[i]);
			return -1;
		}
	}
	printf("Succecc to all test!!!\n");
	return 0;
}
