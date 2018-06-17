#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <elf.h>
#include <pthread.h>
#include "memorypool.h"

#define MAXSIZE 1024
#define MAXNUM 1024

typedef struct mpool_result_t {
	MemoryPool this;
	int is_multithread;
	int maxsize;
	int result;
} mpool_result_t;

void *handle;
void * test_mpool_malloc_thread(void *arg) {
	mpool_result_t * data = (mpool_result_t *)arg;

	void *mem_list[MAXNUM];
	int i=0;
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = mpool_calloc(data->this, 1, MAXSIZE);
	}

	uint64_t size=0;
	if(!data->is_multithread) {
	//memory check
	for(i=0;i<MAXNUM-1;i++) {
		size=(uint64_t)mem_list[i+1] - (uint64_t) mem_list[i];
		//printf("size[%d] %d\n", i,  size);
		if(size != MAXSIZE) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}
	}

	void * local = mpool_calloc(data->this, 1, MAXSIZE);
	for(i=0;i<MAXNUM;i++) {
		//printf("0x%x\n", mem_list[i]);
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}
//	printf("0x%x\n", local);

	mpool_free(data->this, local);

	//no free tmp data
	for(i=0;i<MAXNUM;i++) {
		memset(mem_list[i], 0, MAXSIZE);
	}

	//free data
	mpool_free(data->this, mem_list[10]);
	mpool_free(data->this, mem_list[50]);

	local = mpool_calloc(data->this, 1, data->maxsize+1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}

	void * local2 = mpool_calloc(data->this, 2, data->maxsize/2 + 1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}

	mpool_free(data->this, local2);
	mpool_free(data->this, local);

	local = mpool_calloc(data->this, 1, MAXSIZE);
	local2 = mpool_calloc(data->this, 1, MAXSIZE);
	if(!data->is_multithread) {
	if(local != mem_list[10] && local != mem_list[50]) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}

	if((local2 != mem_list[10] && local2 != mem_list[50]) || local == local2) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}
	} else {
		mem_list[10]=local;
		mem_list[50]=local2;
	}

	for(i=0;i<MAXNUM;i++) {
		mpool_free(data->this, mem_list[i]);
	}

	//exit
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	data->result = 0;
end:
	if(data->is_multithread) {pthread_exit(NULL);}
	return NULL;
}

int test_mpool_malloc() {
	mpool_result_t result={NULL, 0, MAXSIZE, -1};
	result.this = mpool_create(MAXSIZE, MAXNUM, result.is_multithread);

	test_mpool_malloc_thread(&result);
	mpool_delete(result.this);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	mpool_result_t result2;
	result.maxsize = MAXSIZE*2;
	result.is_multithread = 1;
	result.this = mpool_create(MAXSIZE*2, MAXNUM, result.is_multithread);
	memcpy(&result2, &result, sizeof(result));

	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, test_mpool_malloc_thread, &result);
	pthread_create(&tid2, NULL, test_mpool_malloc_thread, &result2);
	pthread_join(tid1, NULL);
	pthread_join(tid2,NULL);

	mpool_delete(result.this);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	if(result2.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	return 0;
}

int main(int argc, char *argv[]) {
	if(test_mpool_malloc()) {
		printf( "###(%d)failed\n", __LINE__);
	}

	printf( "(%d)Finish to test!!\n", __LINE__);
	return 0;
}
