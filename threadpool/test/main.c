#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include "event_threadpool.h"
#include <sys/eventfd.h>

#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#define DEBUG_ERRPRINT_(fmt, ...)  \
        fprintf(stderr, "[%s(%s:%u)thread:%x]: "fmt"%s", __func__,__FILE__,__LINE__,(unsigned int)pthread_self(), __VA_ARGS__)

int test_tpoll_failsafe() {
	event_tpool_manager_free(NULL);

	if(-1 != event_tpool_manager_get_threadnum(NULL)) {
		DEBUG_ERRPRINT("####Failed to check NULL EventTPoolManager for event_tpool_manager_get_threadnum\n");
		return -1;
	}

	event_subscriber_t subscriber;
	EventTPoolManager tpool = calloc(1, 1024);
	memset(&subscriber, 0, sizeof(subscriber));
	event_tpool_add_result_t result;
	result = event_tpool_add(NULL, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check NULL EventTPoolManager\n");
		return -1;
	}

	result = event_tpool_add(tpool, NULL, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check NULL subscriber\n");
		return -1;
	}

	result = event_tpool_add_thread(tpool, -1, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check call before event_tpool_manager_new\n");
		return -1;
	}
	EventTPoolThreadInfo instance = calloc(1, 1024);
	result = event_tpool_update(NULL, (EventTPoolThreadInfo)instance, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check call before event_tpool_manager_new\n");
		return -1;
	}
	result = event_tpool_update(tpool, (EventTPoolThreadInfo)instance, NULL, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check call before event_tpool_manager_new\n");
		return -1;
	}
	free(instance);
	result = event_tpool_update(NULL, NULL, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check call before event_tpool_manager_new\n");
		return -1;
	}

	free(tpool);

	event_tpool_set_stack_size(25*1024);
	tpool = event_tpool_manager_new(-1, 0);
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

	result = event_tpool_add_thread((EventTPoolManager)tpool, size, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check over size");
		return -1;
	}

	result = event_tpool_add_thread((EventTPoolManager)tpool, -1, &subscriber, NULL);
	if(0<result.result) {
		DEBUG_ERRPRINT("####Failed to check under size");
		return -1;
	}

	result = event_tpool_add_thread((EventTPoolManager)tpool, size-1, NULL, NULL);
	if(0<result.result) {
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
	EventTPoolManager tpool;
	int callcnt;
	pthread_t tid;
	int sockpair[2];
	int checkresult;
	const char *funcname;
} testdata_t;

static void common(evutil_socket_t fd, int eventflag, testdata_t * testdata) {
	int tmp;
	DEBUG_ERRPRINT("read start %d\n", eventflag);
	tmp=read(fd, &tmp, sizeof(tmp));
	DEBUG_ERRPRINT("read end\n");
	testdata->tid = pthread_self();
	testdata->callcnt++;
	if(fd != testdata->sockpair[SUBSCRIBER_FD]) {
		testdata->checkresult=-1;
	}
}
static void common2(evutil_socket_t fd, int eventflag, testdata_t * testdata) {
	uint64_t tmp;
	if(!(eventflag&EV_TPOOL_READ)) return;
	eventfd_read(fd, &tmp);
	DEBUG_ERRPRINT("read %lu", tmp);
	testdata->tid = pthread_self();
	testdata->callcnt++;
	if(fd != testdata->sockpair[SUBSCRIBER_FD]) {
		testdata->checkresult=-1;
	}
}

static void test_1(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %x, %d, %s\n", testdata->callcnt, (unsigned int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void test_2(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %x, %d, %s\n", testdata->callcnt, (unsigned int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void test_3(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %x, %d, %s\n", testdata->callcnt, (unsigned int)testdata->tid, testdata->checkresult, testdata->funcname);
}

static void test_4(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	DEBUG_ERRPRINT("exit, %d, %x, %d, %s\n", testdata->callcnt, (unsigned int)testdata->tid, testdata->checkresult, testdata->funcname);
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
		{.fd = testdata[0].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_1},
		{.fd = testdata[1].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_2},
		{.fd = testdata[2].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_3},
		{.fd = testdata[3].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_4},
	};

	event_tpool_add_result_t tid[TESTDATA];
	for(int i=0; i<TESTDATA-1; i++) {
		printf("add[%d] fd:%d\n", i, subscriber[i].fd);
		tid[i] = event_tpool_add(tpool, &subscriber[i], &testdata[i]);
		if(tid[i].result < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_add[%d]\n", i);
			return -1;
		
		}
	}

	if(separatecheck) {
	if(tid[0].result==tid[1].result || tid[1].result==tid[2].result || tid[0].result==tid[2].result) {
		DEBUG_ERRPRINT("####Failed to separate thread, %d %d %d\n", tid[0].result, tid[1].result, tid[2].result);
		return -1;
	}
	}

	printf("###add[%d] fd:%d to same as %d\n", 3, subscriber[3].fd, subscriber[1].fd);
	tid[3]=event_tpool_add_thread(tpool, tid[1].result, &subscriber[3], &testdata[3]);
	if(separatecheck) {
	if(tid[3].result != tid[1].result) {
		DEBUG_ERRPRINT("####Failed to add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	} else {
	if(tid[3].result == -1) {
		DEBUG_ERRPRINT("####Failed to add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	}

	//wait to add event
	//call write
	for(int i=0;i<TESTDATA;i++) {
		int tmp=0;
		tmp=write(testdata[i].sockpair[TEST_FD], &tmp, sizeof(tmp));
	}

	sleep(2);
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
	{
	event_tpool_add_result_t tmp_result=event_tpool_add(tpool, &subscriber[3], &testdata[3]);
	if(tmp_result.result != -1) {
		DEBUG_ERRPRINT("####Failed to reject re-add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	}
	{
	event_tpool_add_result_t tmp_result=event_tpool_add_thread(tpool, tid[1].result, &subscriber[3], &testdata[3]);
	if(tmp_result.result != -1) {
		DEBUG_ERRPRINT("####Failed to reject re-add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	}
	{
	event_tpool_add_result_t tmp_result=event_tpool_add_thread(tpool, tid[3].result, &subscriber[3], &testdata[3]);
	if(tmp_result.result != -1) {
		DEBUG_ERRPRINT("####Failed to reject re-add event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	}
	{
	printf("add[3] fd:%d\n", subscriber[3].fd);
	event_tpool_add_result_t tmp_result=event_tpool_update(tpool, tid[3].event_handle, &subscriber[3], &testdata[3]);
	if(tmp_result.result == -1) {
		DEBUG_ERRPRINT("####Failed to update event_tpool_add_thread[%d]\n", 3);
		return -1;
	}
	}
	//sleep(2);
	//rewrite
	for(int i=0;i<TESTDATA;i++) {
		int tmp=0;
		tmp=write(testdata[i].sockpair[TEST_FD], &tmp, sizeof(tmp));
	}

	sleep(2);
	//check 0
	if(testdata[0].callcnt != 2 || testdata[0].checkresult == -1 || strcmp(testdata[0].funcname, "test_1") != 0) {
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
	event_tpool_del(tpool, subscriber[0].fd);
	event_tpool_del(tpool, subscriber[2].fd);
	event_tpool_del(tpool, subscriber[3].fd);

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


testdata_t testdata_g[TESTDATA];
event_subscriber_t subscriber_g[TESTDATA];

static void own_test_1(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	event_tpool_add_result_t result;
	result = event_tpool_add(testdata->tpool, &subscriber_g[1], &testdata_g[1]);
	testdata_g[1].tid = result.result;
	result = event_tpool_add(testdata->tpool, &subscriber_g[2], &testdata_g[2]);
	testdata_g[2].tid = result.result;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void own_test_2(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	event_tpool_add_result_t result;
	result = event_tpool_add_thread(testdata->tpool, testdata_g[2].tid, &subscriber_g[3], &testdata_g[3]);
	testdata_g[3].tid = result.result;
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}
static void own_test_3(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	event_tpool_del(testdata->tpool, subscriber_g[1].fd);
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}

static void own_test_4(evutil_socket_t fd, int eventflag, void * arg) {
	DEBUG_ERRPRINT("enter\n");
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	event_tpool_manager_free(testdata->tpool);
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}

int test_tpoll_fo_ownthread() {
	int i=0;
	EventTPoolManager tpool = event_tpool_manager_new(3, 0);
	if(!tpool) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_manager_new\n");
		return -1;
	}
	void (*functable[])(evutil_socket_t fd, int eventflag, void * arg) = {
		own_test_1,own_test_2,own_test_3,own_test_4
	};

	for(int i=0;i<TESTDATA;i++) {
		testdata_g[i].tpool = tpool;
		testdata_g[i].sockpair[SUBSCRIBER_FD] = eventfd(0,0);
		subscriber_g[i].fd = testdata_g[i].sockpair[SUBSCRIBER_FD];
		subscriber_g[i].eventflag=EV_TPOOL_READ;
		subscriber_g[i].event_callback = functable[i];
	}
	//add 1
	event_tpool_add_result_t result;
	result = event_tpool_add(tpool, &subscriber_g[0], &testdata_g[0]);
	testdata_g[0].tid = result.result;
	if(testdata_g[0].tid < 0) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_add[0]\n");
		return -1;
	}

	eventfd_write(testdata_g[0].sockpair[SUBSCRIBER_FD], 1);
	sleep(1);
	if(testdata_g[0].callcnt != 1 || testdata_g[0].checkresult== -1 || strcmp(testdata_g[0].funcname, "own_test_1") != 0) {
		
		DEBUG_ERRPRINT("####Failed to call event_tpool_add[0]\n");
		return -1;
	}

	eventfd_write(testdata_g[1].sockpair[SUBSCRIBER_FD], 1);
	sleep(1);
	if(testdata_g[1].callcnt != 1 || testdata_g[1].checkresult== -1 || strcmp(testdata_g[1].funcname, "own_test_2") != 0 || testdata_g[2].tid != testdata_g[3].tid) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_add[1]\n");
		return -1;
	}

	eventfd_write(testdata_g[2].sockpair[SUBSCRIBER_FD], 1);
	sleep(1);
	if(testdata_g[2].callcnt != 1 || testdata_g[2].checkresult== -1 || strcmp(testdata_g[2].funcname, "own_test_3") != 0) {
		
		DEBUG_ERRPRINT("####Failed to call event_tpool_add[2]\n");
		return -1;
	}

	//delete check
	eventfd_write(testdata_g[1].sockpair[SUBSCRIBER_FD], 1);
	if(testdata_g[1].callcnt != 1 || testdata_g[1].checkresult== -1 || strcmp(testdata_g[1].funcname, "own_test_2") != 0 ) {
		
		DEBUG_ERRPRINT("####Failed to call delete[1]\n");
		return -1;
	}

	//free
	eventfd_write(testdata_g[3].sockpair[SUBSCRIBER_FD], 1);
	sleep(1);
	if(testdata_g[3].callcnt != 1 || testdata_g[3].checkresult== -1 || strcmp(testdata_g[3].funcname, "own_test_4") != 0) {
		
		DEBUG_ERRPRINT("####Failed to call event_tpool_add[1]\n");
		return -1;
	}
	//wait to stop, because no sync
	sleep(2);
	for(i=0;i<TESTDATA;i++) {
		eventfd_write(testdata_g[i].sockpair[SUBSCRIBER_FD], 1);
	}
	sleep(2);
	for(i=0;i<TESTDATA;i++) {
		if(testdata_g[1].callcnt != 1) {
			DEBUG_ERRPRINT("####Failed to free event_tpool[%d]\n", i);
			return -1;
			
		}
	}
	return 0;
}

#define TESTDATA_MAX (1000)
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
static void testfunc(evutil_socket_t fd, int eventflag, void * arg) {
	if(!(eventflag&EV_TPOOL_READ)) return;
	pthread_mutex_lock(&lock);
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	testdata->funcname = __FUNCTION__;
	pthread_mutex_unlock(&lock);
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
}

static void testfunc_end(evutil_socket_t fd, int eventflag, void * arg) {
	if(!(eventflag&EV_TPOOL_READ)) return;
	pthread_mutex_lock(&lock);
	testdata_t * testdata = (testdata_t *)arg;
	common2(fd, eventflag, testdata);
	DEBUG_ERRPRINT("exit, %d, %d, %d, %s\n", testdata->callcnt, (int)testdata->tid, testdata->checkresult, testdata->funcname);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

int test_tpoll_maxfd() {
	testdata_t testdata={0};
	int fd = 0;
	event_subscriber_t subscriber[TESTDATA_MAX];
	EventTPoolManager tpool = event_tpool_manager_new(4, 1);
	testdata.tpool = tpool;

	for(int i=0;i<TESTDATA_MAX;i++) {
		subscriber[i].fd = eventfd(0,0);;
		if(subscriber[i].fd < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_add[%d]\n", i);
			return -1;
		}
		subscriber[i].eventflag=EV_TPOOL_READ;
		subscriber[i].event_callback = testfunc;
	}
	subscriber[TESTDATA_MAX-1].event_callback = testfunc_end;

	//add all event
	event_tpool_add_result_t result[TESTDATA_MAX];
	for(int i=0;i<TESTDATA_MAX-1;i++) {
		result[i] = event_tpool_add(tpool, &subscriber[i], &testdata);
		if(result[i].result < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_add[%d]\n", i);
			return -1;
		}
//		usleep(10000);
		eventfd_write(subscriber[i].fd, 1);
	}

	result[TESTDATA_MAX-1] = event_tpool_add(tpool, &subscriber[TESTDATA_MAX-1], &testdata);
	pthread_mutex_lock(&lock);
	eventfd_write(subscriber[TESTDATA_MAX-1].fd, 1);

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 10;
	int ret = pthread_cond_timedwait(&cond, &lock, &timeout);
	pthread_mutex_unlock(&lock);
	if(ret == ETIMEDOUT) {
		DEBUG_ERRPRINT("####Failed to call end\n");
		return -1;
	}
	
	if(testdata.callcnt != TESTDATA_MAX || strcmp(testdata.funcname, "testfunc") != 0) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_add callback event, cnt=%d\n",testdata.callcnt);
		return -1;
	}

	//update check
	for(int i=0;i<TESTDATA_MAX;i++) {
		subscriber[i].eventflag=EV_TPOOL_HUNGUP;
		result[i] = event_tpool_update(tpool, result[i].event_handle, &subscriber[i], &testdata);
		if(result[i].result < 0) {
			DEBUG_ERRPRINT("####Failed to call event_tpool_update[%d]\n", i);
			return -1;
		}
		eventfd_write(subscriber[i].fd, 1);
	}
	if(testdata.callcnt != TESTDATA_MAX || strcmp(testdata.funcname, "testfunc") != 0) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_udate callback event, cnt=%d\n",testdata.callcnt);
		return -1;
	}

	//clean
	for(int i=0;i<TESTDATA_MAX;i++) {
		subscriber[i].eventflag=0;
		result[i] = event_tpool_update(tpool, result[i].event_handle, &subscriber[i], &testdata);
		eventfd_write(subscriber[i].fd, 1);
	}
	if(testdata.callcnt != TESTDATA_MAX || strcmp(testdata.funcname, "testfunc") != 0) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_update(re-read) callback event, cnt=%d\n",testdata.callcnt);
		return -1;
	}

	//delete
	for(int i=0;i<TESTDATA_MAX;i++) {
		event_tpool_del(tpool, subscriber[i].fd);
		eventfd_write(subscriber[i].fd, 1);
	}
	if(testdata.callcnt != TESTDATA_MAX || strcmp(testdata.funcname, "testfunc") != 0) {
		DEBUG_ERRPRINT("####Failed to call event_tpool_delete callback event, cnt=%d\n",testdata.callcnt);
		return -1;
	}

	//end
	event_tpool_manager_free(tpool);
	for(int i=0;i<TESTDATA_MAX;i++) {
		close(subscriber[i].fd);
	}
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
		{.fd = testdata[0].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_1},
		{.fd = testdata[1].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_2},
		{.fd = testdata[2].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_3},
		{.fd = testdata[3].sockpair[SUBSCRIBER_FD], .eventflag=EV_TPOOL_READ, .event_callback = test_4},
	};

	int tid[TESTDATA];
	for(int i=0; i<TESTDATA-1; i++) {
		printf("add[%d] fd:%d\n", i, subscriber[i].fd);
		event_tpool_add_result_t result;
		result = event_tpool_add(tpool, &subscriber[i], &testdata[i]);
		tid[i] = result.result;
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

	if(test_tpoll_fo_ownthread()) {
		DEBUG_ERRPRINT("Failed to check do own thread\n");
		return -1;
	}

	if(test_tpoll_maxfd()) {
		DEBUG_ERRPRINT("Failed to check maxfd usage\n");
		return -1;
	}

	if(test_tpoll_free()) {
		DEBUG_ERRPRINT("Failed to check thread safe\n");
		return -1;
	}

	printf("Succecc to all test !!!\n");
	return 0;
}
