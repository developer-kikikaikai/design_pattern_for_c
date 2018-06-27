#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include "dp_debug.h"

static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static int mutex_data=0;

static int sockpair[2];
static int write_data=0;

static int normal_data=0;
static DPTimeLog handle;

void * mutex_thread(void *arg) {
	int *num = (int *) arg;
	int i=0;
	dp_timelog_print(handle, "start, value=%d\n", mutex_data);
	for(i=0; i< *num; i++) {
		pthread_mutex_lock(&mutex);
		mutex_data++;
		pthread_mutex_unlock(&mutex);
	}
	dp_timelog_print(handle, "end, value=%d\n", mutex_data);
	pthread_exit(NULL);
	return NULL;
}
void * nomutex_thread(void *arg) {
	int *num = (int *) arg;
	int i=0;
	dp_timelog_print(handle, "start, value=%d\n", normal_data);
	for(i=0; i< *num; i++) {
		normal_data++;
	}
	dp_timelog_print(handle, "end, value=%d\n", normal_data);
	pthread_exit(NULL);
	return NULL;
}

void * read_thread(void *arg) {
	int *num = (int *) arg;
	int i=0;
	pthread_t tid=0;
	dp_timelog_print(handle, "start, value=%d\n", write_data);
	for(i=0; i< *num; i++) {
		read(sockpair[1], &tid, sizeof(tid));
		write_data++;
	}
	dp_timelog_print(handle, "end, value=%d\n", write_data);
	pthread_exit(NULL);
	return NULL;
}

int initialize() {
	socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair);
}

static void read_testdata_mutex(int * fsize) {
	pthread_t tid=0;
	pthread_create(&tid, NULL, mutex_thread, fsize);
	pthread_join(tid, NULL);
}

static void read_testdata_socket(int * fsize) {
	pthread_t tid=0;
	pthread_create(&tid, NULL, read_thread, fsize);
	int i=0;
	for(i=0; i< *fsize; i++) {
		write(sockpair[0], &tid, sizeof(tid));
	}
	pthread_join(tid, NULL);
}

static void read_testdata_normal(int * fsize) {
	pthread_t tid=0;
	pthread_create(&tid, NULL, nomutex_thread, fsize);
	pthread_join(tid, NULL);
	dp_timelog_print(handle, "end, value=%d\n", normal_data);
}

int main(int argc, char *argv[]) {
	if(argc < 2) return 0;
	int fsize=atoi(argv[1]);

	/*write id, len*/
	initialize();

	handle = dp_timelog_init(",", 100, 1000, 0);
	if(!handle) return 0;

	dp_timelog_print(handle, "Mutex\n");
	read_testdata_mutex(&fsize);
	dp_timelog_print(handle, "Socket\n");
	read_testdata_socket(&fsize);
	dp_timelog_print(handle, "No lock\n");
	read_testdata_normal(&fsize);

	//exit
	dp_timelog_exit(handle);
	return 0;
}
