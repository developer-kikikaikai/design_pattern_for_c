#include <stdlib.h>
#include "config.h"
#include "test_main.h"

int main(int argc, char *argv[]) {
DPDEBUG_INIT_THREADSAFE
	static const char * pluginlist[] = {
#ifdef TPOOLEVENT_LIBEV
		EVENT_IF_PLUGIN_PATH"/libevent_if_libev.so",
#endif
#ifdef TPOOLEVENT_LIBEVENT
		EVENT_IF_PLUGIN_PATH"/libevent_if_libevent.so",
#endif
#ifdef TPOOLEVENT_EPOLL
		EVENT_IF_PLUGIN_PATH"/libevent_if_epoll.so",
#endif
#ifdef TPOOLEVENT_SELECT
		EVENT_IF_PLUGIN_PATH"/libevent_if_select.so",
#endif
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
