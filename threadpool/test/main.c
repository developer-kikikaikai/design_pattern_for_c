#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include "event_threadpool.h"
#define DBGFLAG
#include "dp_debug.h"


int test_tpoll_failsafe() {
	event_tpool_manager_free(NULL);

	if(-1 != event_tpool_manager_get_threadnum(NULL)) {
		DEBUG_ERRPRINT("####Failed to check NULL EventTPoolManager for event_tpool_manager_get_threadnum\n");
		return -1;
	}

	event_subscriber_t subscriber;
	if(0<event_tpool_add(NULL, &subscriber, NULL)) {
		DEBUG_ERRPRINT("####Failed to check NULL EventTPoolManager\n");
		return -1;
	}

	if(0<event_tpool_add((EventTPoolManager)&subscriber, NULL, NULL)) {
		DEBUG_ERRPRINT("####Failed to check NULL subscriber\n");
		return -1;
	}

	if(0<event_tpool_add_thread((EventTPoolManager)&subscriber, -1, &subscriber, NULL)) {
		DEBUG_ERRPRINT("####Failed to check call before event_tpool_manager_new\n");
		return -1;
	}

	EventTPoolManager tpool = event_tpool_manager_new(-1, 0);
	if(!tpool) {
		DEBUG_ERRPRINT("####Failed to create tpoll manager\n");
		return -1;
	}

	size_t size = event_tpool_manager_get_threadnum(tpool);
	//check CPU size
	FILE * fp = popen(" cat /proc/cpuinfo | grep processor | wc -l", "r");
	if(!fp) return -1;
	char buffer[16]={0};
	if(fgets(buffer, sizeof(buffer), fp) ==NULL) return -1;
	pclose(fp);
	size_t cpu=(size_t)atoi(buffer);
	if(size != cpu*2) {
		DEBUG_ERRPRINT("####Default thread num is different! return %u and cpuinfo is %u (so it will be %u*2)\n", (unsigned int)size, (unsigned int)cpu, (unsigned int)cpu);
		return -1;
	}

	if(0<event_tpool_add_thread((EventTPoolManager)tpool, size, &subscriber, NULL)) {
		DEBUG_ERRPRINT("####Failed to check over size");
		return -1;
	}
	if(0<event_tpool_add_thread((EventTPoolManager)tpool, -1, &subscriber, NULL)) {
		DEBUG_ERRPRINT("####Failed to check under size");
		return -1;
	}

	if(0<event_tpool_add_thread((EventTPoolManager)tpool, size-1, NULL, NULL)) {
		DEBUG_ERRPRINT("####Failed to check subscriber NULL");
		return -1;
	}

	event_tpool_manager_free(tpool);
	return 0;
}

#define TESTDATA (4)
#define SUBSCRIBER_FD (0)
#define TEST_FD (1)

typedef struct testdata{
	int callcnt;
	pthread_t tid;
	int sockpair[2];
	int checkresult;
	const char *funcname;
} testdata_t;

static void common(evutil_socket_t fd, short eventflag, testdata_t * testdata) {
	int tmp;
	read(fd, &tmp, sizeof(tmp));
	testdata->tid = pthread_self();
	testdata->callcnt++;
	if(fd != testdata->sockpair[SUBSCRIBER_FD]) {
		testdata->checkresult=-1;
	}
}

static void test_1(evutil_socket_t fd, short eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void test_2(evutil_socket_t fd, short eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void test_3(evutil_socket_t fd, short eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void test_4(evutil_socket_t fd, short eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}

int test_tpoll_standard(EventTPoolManager tpool, int separatecheck) {
	testdata_t testdata[TESTDATA];
	memset(testdata, 0, sizeof(testdata));

	for(int i=0;i<TESTDATA;i++) {
		socketpair(AF_UNIX, SOCK_DGRAM, 0, testdata[i].sockpair);
	}

	if(event_tpool_manager_get_threadnum(tpool) != 3) {
		DEBUG_ERRPRINT("####Failed to size event_tpool_manager_get_threadnum\n");
		return -1;
	}

	event_subscriber_t subscriber[TESTDATA]={
		{.fd = testdata[0].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ, .event_callback = test_1},
		{.fd = testdata[1].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_2},
		{.fd = testdata[2].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_3},
		{.fd = testdata[3].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_4},
	};

	int tid[TESTDATA];
	for(int i=0; i<TESTDATA-1; i++) {
		printf("add[%d] fd:%d\n", i, subscriber[i].fd);
		tid[i] = event_tpool_add(tpool, &subscriber[i], &testdata[i]);
		if(tid[i] < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_add[%d]\n", i);
			return -1;
		
		}
	}

	if(separatecheck) {
	if(tid[0]==tid[1] || tid[1]==tid[2] || tid[0]==tid[2]) {
		DEBUG_ERRPRINT("####Failed to separate thread, %d %d %d\n", tid[0], tid[1], tid[2]);
		return -1;
	}
	}

	if(event_tpool_add_thread(tpool, tid[1], &subscriber[3], &testdata[3]) != tid[1]) {
		DEBUG_ERRPRINT("####Failed to add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}

	//call write
	for(int i=0;i<TESTDATA;i++) {
		int tmp=0;
		write(testdata[i].sockpair[TEST_FD], &tmp, sizeof(tmp));
	}

	sleep(5);
	//check 0
	if(testdata[0].callcnt != 1 || testdata[0].checkresult == -1 || strcmp(testdata[0].funcname, "test_1") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[0]\n");
		return -1;
	}
	//check 1
	if(testdata[1].callcnt != 1 || testdata[1].checkresult == -1 || strcmp(testdata[1].funcname, "test_2") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[1]\n");
		return -1;
	}
	//check 2
	if(testdata[2].callcnt != 1 || testdata[2].checkresult == -1 || strcmp(testdata[2].funcname, "test_3") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[2]\n");
		return -1;
	}
	//check 3
	if(testdata[3].callcnt != 1 || testdata[3].checkresult == -1 || strcmp(testdata[3].funcname, "test_4") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[3]\n");
		return -1;
	}
	if(separatecheck) {
	//check thread
	if(testdata[0].tid == testdata[1].tid || testdata[1].tid == testdata[2].tid || testdata[0].tid == testdata[2].tid || testdata[3].tid != testdata[1].tid) {
		DEBUG_ERRPRINT("####Failed to separate thread, %d,%d,%d,%d\n", (int)testdata[0].tid, (int)testdata[1].tid, (int)testdata[2].tid, (int)testdata[3].tid);
		return -1;
	}
	}

	//delete
	event_tpool_del(tpool, subscriber[1].fd);
	subscriber[3].event_callback = test_1;
	printf("add[3] fd:%d\n", subscriber[3].fd);
	event_tpool_add(tpool, &subscriber[3], &testdata[3]);

	//rewrite
	for(int i=0;i<TESTDATA;i++) {
		int tmp=0;
		write(testdata[i].sockpair[TEST_FD], &tmp, sizeof(tmp));
	}

	sleep(5);
	//check 0
	if(testdata[0].callcnt != 1 || testdata[0].checkresult == -1 || strcmp(testdata[0].funcname, "test_1") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[0]\n");
		return -1;
	}
	//check 1
	if(testdata[1].callcnt != 1 || testdata[1].checkresult == -1 || strcmp(testdata[1].funcname, "test_2") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[1]\n");
		return -1;
	}
	//check 2
	if(testdata[2].callcnt != 2 || testdata[2].checkresult == -1 || strcmp(testdata[2].funcname, "test_3") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[2]\n");
		return -1;
	}
	//check 3
	if(testdata[3].callcnt != 2 || testdata[3].checkresult == -1 || strcmp(testdata[3].funcname, "test_1") != 0) {
		DEBUG_ERRPRINT("####Failed to call testdata[3]\n");
		return -1;
	}

	for(int i=0;i<TESTDATA;i++) {
		close(testdata[i].sockpair[0]);
		close(testdata[i].sockpair[1]);
	}
	return 0;
}

int test_tpoll_normally() {
	EventTPoolManager tpool = event_tpool_manager_new(3, 0);
	if(!tpool) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_manager_new\n");
		return -1;
	}

	if(test_tpoll_standard(tpool, 1)) {
		DEBUG_ERRPRINT("####Failed to test standard\n");
		return -1;
	}

	printf("Success to call test_tpoll_standard, free manager\n");
	event_tpool_manager_free(tpool);
	return 0;
}

typedef struct testinfo {
	EventTPoolManager pool;
	int result;
} testinfo_t;

void * thread(void *arg) {
	testinfo_t *info = (testinfo_t *)arg;
	info->result = test_tpoll_standard(info->pool, 0);
	pthread_exit(NULL);
}

#define MAXTHREAD (3)
int test_tpoll_thread_safe() {
	EventTPoolManager tpool = event_tpool_manager_new(3, 1);
	testinfo_t data[MAXTHREAD];
	pthread_t tid[MAXTHREAD];
	memset(&data, 0, sizeof(data));
	int i=0;
	for(i=0;i<MAXTHREAD;i++) {
		data[i].pool = tpool;
		pthread_create(&tid[i], NULL, thread, &data[i]);
	}

	for(i=0;i<MAXTHREAD;i++) {
		pthread_join(tid[i], NULL);
		if(data[i].result) {
			DEBUG_ERRPRINT("##Failed to check thread %d\n", i);
			return -1;
		}
	}

	event_tpool_manager_free(tpool);
	return 0;
}

int test_tpoll_free() {
	EventTPoolManager tpool = event_tpool_manager_new(3, 1);
	testdata_t testdata[TESTDATA];
	memset(testdata, 0, sizeof(testdata));

	for(int i=0;i<TESTDATA;i++) {
		socketpair(AF_UNIX, SOCK_DGRAM, 0, testdata[i].sockpair);
	}

	event_subscriber_t subscriber[TESTDATA]={
		{.fd = testdata[0].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ, .event_callback = test_1},
		{.fd = testdata[1].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_2},
		{.fd = testdata[2].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_3},
		{.fd = testdata[3].sockpair[SUBSCRIBER_FD], .eventflag=EV_READ|EV_PERSIST, .event_callback = test_4},
	};

	int tid[TESTDATA];
	for(int i=0; i<TESTDATA-1; i++) {
		printf("add[%d] fd:%d\n", i, subscriber[i].fd);
		tid[i] = event_tpool_add(tpool, &subscriber[i], &testdata[i]);
		if(tid[i] < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_add[%d]\n", i);
			return -1;
		
		}
	}

	//free before delete all
	event_tpool_manager_free(tpool);
	for(int i=0; i<TESTDATA-1; i++) {
		close(testdata[i].sockpair[0]);
		close(testdata[i].sockpair[1]);
	}
	return 0;
}

int main() {
	if(test_tpoll_failsafe()) {
		DEBUG_ERRPRINT("Failed to check fail safe\n");
		return -1;
	}

	if(test_tpoll_normally()) {
		DEBUG_ERRPRINT("Failed to check normally usage\n");
		return -1;
	}

	if(test_tpoll_thread_safe()) {
		DEBUG_ERRPRINT("Failed to check thread safe\n");
		return -1;
	}

	if(test_tpoll_free()) {
		DEBUG_ERRPRINT("Failed to check thread safe\n");
		return -1;
	}

	printf("Succecc to all test !!!\n");
	return 0;
}
