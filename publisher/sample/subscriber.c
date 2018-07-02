#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "publisher.h"
#include "publish_msg.h"

#define SUBSCRIBER_LOG(...) printf("[subscriber]");printf(__VA_ARGS__);

#define WRITER_NUM (5)
struct subscribe_account {
	int type;
	publish_msg_detail_t writer;
} writer_list_g[WRITER_NUM];

static SubscriberAccount account;
static pthread_t tid;
static int sockpair[2];
#define READ_SOCK sockpair[0]
#define WRITE_SOCK sockpair[1]

typedef struct subscriber_msg {
	int type;
	publish_msg_detail_t detail;
} subscriber_msg_t;

static int running_g=1;
static void stop_running(void) {
	running_g=0;
}
static int is_running() {
	return (running_g==1);
}

static struct subscribe_account * search_writter(subscriber_msg_t *msg) {
	int i=0;
	for(i = 0; i < WRITER_NUM; i ++) {
		if( (msg->type&writer_list_g[i].type) && (memcmp(&writer_list_g[i].writer, &msg->detail, sizeof(msg->detail)) == 0)) {
			return &writer_list_g[i];
		}
	}

	return NULL;
}

static void subscriber_msg_action(subscriber_msg_t *msg) {
	struct subscribe_account *writter = search_writter(msg);
	if(!writter) {
		return;
	}

	switch(msg->type) {
		case PUB_MSG_PUBLISH_NEW_BOOK:
			SUBSCRIBER_LOG("Wow, %s's new book release!! I will buy it!\n", msg->detail.writername);
			break;
		case PUB_MSG_PUBLISH_DISCOUNT_BOOK:
			SUBSCRIBER_LOG("Wow, %s's book is discount!! I may be able to  buy it!\n", msg->detail.writername);
			break;
		case PUB_MSG_PUBLISH_STOP_PRODUCTION:
			SUBSCRIBER_LOG("Oh..., %s's book production is stopped, ...\n", msg->detail.writername);
			publisher_unsubscribe(PUBLISH_ID, account);
			account=NULL;
			stop_running();
			break;
		default:
			break;
	}
}

static void * subscriber_main(void *arg) {

	subscriber_msg_t msg;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(READ_SOCK, &readfds);
	while(is_running()) {
		FD_ZERO(&readfds);
		FD_SET(READ_SOCK, &readfds);
		SUBSCRIBER_LOG("Wait msg...\n");
		if(select(READ_SOCK+1, &readfds, NULL, NULL, NULL) <0 ) {
			SUBSCRIBER_LOG("failed to select, %s\n", strerror(errno));
			continue;
		}
		if (FD_ISSET(READ_SOCK, &readfds)) {
			memset(&msg, 0, sizeof(msg));
			int ret = read(READ_SOCK, &msg, sizeof(msg));
			if(ret < 0) {
				SUBSCRIBER_LOG("failed to select, %s\n", strerror(errno));
			}
			subscriber_msg_action(&msg);
		}
	}

	close(READ_SOCK);
	pthread_exit(NULL);
}

static void create_socket() {
	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair)) {
		SUBSCRIBER_LOG("Failed to create socket pair!\n");
		return;
	}

}
void notification(int publish_type, void * defail, void *ctx) {
	publish_msg_detail_t *msgdetail = (publish_msg_detail_t *) defail;
	subscriber_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = publish_type;
	memcpy(&msg.detail, msgdetail, sizeof(msg.detail));
	int ret = write(WRITE_SOCK, &msg, sizeof(msg));
	if(ret < 0) {
		SUBSCRIBER_LOG("...failed to send, errno=%s\n", strerror(errno));
	}
}

void subscriber_init() {

	account = publisher_subscribe(PUBLISH_ID, PUB_MSG_PUBLISH_NEW_BOOK|PUB_MSG_PUBLISH_STOP_PRODUCTION|PUB_MSG_PUBLISH_DISCOUNT_BOOK, notification, NULL);

	sprintf(writer_list_g[0].writer.writername, "Jeffrey_Deaver");
	writer_list_g[0].type = PUB_MSG_PUBLISH_NEW_BOOK | PUB_MSG_PUBLISH_STOP_PRODUCTION;
	SUBSCRIBER_LOG("About \"Jeffrey_Deaver\", I have all books, so only wait new release!\n");

	sprintf(writer_list_g[1].writer.writername, "Hiroshi_Mori");
	writer_list_g[1].type = PUB_MSG_PUBLISH_NEW_BOOK | PUB_MSG_PUBLISH_DISCOUNT_BOOK | PUB_MSG_PUBLISH_STOP_PRODUCTION;
	SUBSCRIBER_LOG("About \"Hiroshi_Mori\", I have almost books, but I can't buy some books.\n");
	SUBSCRIBER_LOG("I have to buy it before stop production!\n");

	sprintf(writer_list_g[2].writer.writername, "Ryotaro_Shiba");
	writer_list_g[2].type = PUB_MSG_PUBLISH_DISCOUNT_BOOK;
	SUBSCRIBER_LOG("About \"Ryotaro_Shiba\", I have no money to buy all his books...\n");
	SUBSCRIBER_LOG("If there are discount, I will buy it.\n");

	create_socket();

	pthread_create(&tid, NULL, subscriber_main, NULL);
}
void subscriber_exit() {
	//stop thread
	stop_running();
	publish_msg_detail_t msg;
	notification((0x01<<10), &msg, NULL);
	pthread_join(tid, NULL);

	//close socket
	close(WRITE_SOCK);
}
