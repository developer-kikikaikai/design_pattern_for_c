#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "publisher.h"
#include "publish_msg.h"
#include "subscriber.h"

static int running_g=1;
static void stop_running(void) {
	running_g=0;
}
static int is_running() {
	return (running_g==1);
}
void sighandler(int signum) {
	printf("call sighandler, id=%d\n", signum);
	stop_running();
	printf("exit process! please enter some key!!\n");
}

int main(int argc, char *argv[]) {
	struct {
		int type;
		char *defail;
	} msgtype[] = {
		{PUB_MSG_PUBLISH_NEW_BOOK, "Release new book!"},
		{PUB_MSG_PUBLISH_DISCOUNT_BOOK, "Discount book!"},
		{PUB_MSG_PUBLISH_STOP_PRODUCTION, "Stop production this book..."},
	};

	//signal setting
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	publisher_new(1);

	printf("Start subscribe!\n");
	subscriber_init();

	int ret=0;
	int i=0;
	int type;
	publish_msg_detail_t msg_defail;
	while(is_running()) {
		memset(&msg_defail, 0, sizeof(msg_defail));
		printf("Please enter witter name\n");
		ret = scanf("%s", msg_defail.writername);
		if(ret != 1 || !is_running()) continue;

		printf("Please enter msg type:\n");
		for(i = 0;i < sizeof(msgtype)/sizeof(msgtype[0]); i ++) {
			printf("type:%d (%s)\n", msgtype[i].type, msgtype[i].defail);
		}

		ret = scanf("%d", &type);
		if(ret != 1 || !is_running()) continue;

		printf("OK, notify to subscriber!\n");
		publisher_publish(PUBLISH_ID, type, &msg_defail);
	}

	subscriber_exit();
	publisher_free();
}
