#include "publisher.h"
#include "event_threadpool.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static void event_publish(int socketfd, int eventflag, void * event_arg) {
	char buf[256]={0};
	if(eventflag|EV_TPOOL_READ)
		read(socketfd, buf, sizeof(buf));
	publisher_publish(1, socketfd, buf);
}

static void add_event_publisher(EventTPoolManager tpool) {
	event_subscriber_t stdout_subscriber ={STDOUT_FILENO, EV_TPOOL_READ, event_publish};
	event_tpool_add(tpool, &stdout_subscriber, NULL);
}

void publisher_notify(int publish_type, void * detail, void *ctx) {
	fprintf(stderr, "enter publisher_notify\n");
	char * str = (char *)detail;
	char * str_ctx = (char *)ctx;
	printf("input:%s(context:%s)\n", str, str_ctx);
}

int main() {
	int maxfd=1, ret = 0;;
	EventTPoolManager tpool = event_tpool_manager_new(1,0);
	add_event_publisher(tpool);
	publisher_new(1);

	publisher_subscribe(1, STDOUT_FILENO, publisher_notify, "for stdout");//for stdout
	while(1) {
		ret = select(maxfd+1, NULL, NULL, NULL, NULL);
		if(ret<0) {
			printf("exit\n");
			break;
		}
	}
	return 0;
}
