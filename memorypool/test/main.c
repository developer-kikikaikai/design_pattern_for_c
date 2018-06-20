#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <elf.h>
#include <pthread.h>
#include <unistd.h>
#include "memorypool.h"

#define MAXSIZE 1029
#define MAXSIZE_LEN 2048
#define MAXNUM 1024

typedef struct mpool_result_t {
	MemoryPool this;
	int is_multithread;
	int maxsize;
	int result;
} mpool_result_t;

void * test_mpool_malloc_thread(void *arg) {
	mpool_result_t * data = (mpool_result_t *)arg;
	char buf[MAXSIZE]={1};
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
		if(size != MAXSIZE_LEN) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}
	}
	printf("finish!\n");

	sleep(1);
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

int test_mpool_malloc_single_thread() {
	mpool_result_t result={NULL, 0, MAXSIZE_LEN, -1};
	result.this = mpool_create(MAXSIZE, MAXNUM, result.is_multithread, NULL);

	test_mpool_malloc_thread(&result);
	mpool_delete(result.this, NULL);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	return 0;
}

int test_mpool_malloc_multi_thread() {
	mpool_result_t result={NULL, 0, MAXSIZE, -1};
	mpool_result_t result2;
	result.maxsize = MAXSIZE_LEN;
	result.is_multithread = 1;
	result.this = mpool_create(MAXSIZE, MAXNUM*2, result.is_multithread, NULL);
	memcpy(&result2, &result, sizeof(result));

	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, test_mpool_malloc_thread, &result);
	pthread_create(&tid2, NULL, test_mpool_malloc_thread, &result2);
	pthread_join(tid1, NULL);
	pthread_join(tid2,NULL);

	mpool_delete(result.this, NULL);
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

void * malloc_thread(void *arg) {
	MemoryPool mpool = (MemoryPool)arg;
	int i=0;
	for(i=0;i<MAXNUM/2;i++) {
		mpool_malloc(mpool, MAXSIZE/2);
	}
	pthread_exit(NULL);
	return NULL;
}

int test_mpool_malloc_getnext_check() {
	MemoryPool mpool = mpool_create(MAXSIZE, MAXNUM, 1, NULL);
	void *mem_list[MAXNUM];
	void *ptr;

	int cnt=0;
	FOR_ALL_USEDMEM(mpool, ptr) {
		cnt++;
	}
	if(cnt!=0) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	//only 3
	mem_list[cnt++] = mpool_malloc(mpool, MAXSIZE/2);
	mem_list[cnt++] = mpool_malloc(mpool, MAXSIZE);
	mem_list[cnt++] = mpool_malloc(mpool, MAXSIZE);

	FOR_ALL_USEDMEM(mpool, ptr) {
		cnt--;
		if(mem_list[cnt] != ptr) {
			printf( "###(%d)failed, cnt=%d\n", __LINE__, cnt);
			return -1;
		}
	}
	if(cnt!=0) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	// check after free
	mpool_free(mpool, mem_list[1]);
	ptr = mpool_get_next_usedmem(mpool, NULL);
	if(mem_list[2] != ptr) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	ptr = mpool_get_next_usedmem(mpool, ptr);
	if(mem_list[0] != ptr) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	ptr = mpool_get_next_usedmem(mpool, ptr);
	if(ptr != NULL) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	//check all
	mem_list[1]=mem_list[2];

	for(cnt=2; cnt<MAXNUM; cnt++) {
		mem_list[cnt] = mpool_malloc(mpool, MAXSIZE);
	}

	FOR_ALL_USEDMEM(mpool, ptr) {
		cnt--;
		if(mem_list[cnt] != ptr) {
			printf( "###(%d)failed, cnt=%d\n", __LINE__, cnt);
			return -1;
		}
	}

	if(cnt!=0) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	int cnt2=MAXNUM-1;
	//free check
	for(cnt2=MAXNUM-1; cnt2!=0; cnt2--) {
		mpool_free(mpool, mem_list[cnt2]);
		cnt = cnt2;
		FOR_ALL_USEDMEM(mpool, ptr) {
			cnt--;
			if(mem_list[cnt] != ptr) {
				printf( "###(%d)failed\n", __LINE__);
				printf( "###(%d)failed, cnt=%d, cnt2=%d\n", __LINE__, cnt, cnt2);
				return -1;
			}
		}
		if(cnt!=0) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	//thread check
	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, malloc_thread, mpool);
	pthread_create(&tid2, NULL, malloc_thread, mpool);
	pthread_join(tid1, NULL);
	pthread_join(tid2,NULL);

	void *old_ptr;
	cnt=0;
	FOR_ALL_USEDMEM(mpool, ptr) {
		cnt++;
	}
	if(cnt!=MAXNUM) {
		printf( "###(%d)failed, cnt=%d\n", __LINE__, cnt);
		return -1;	
	}
	return 0;
}

static int constructor_call=0;
void test_constructor(void *ptr) {
	constructor_call++;
	memset(ptr, 1, MAXSIZE);
}

static int destructor_call=0;
void test_destructor(void *ptr) {
	destructor_call++;
}

int test_mpool_malloc_initialize_chech() {
	MemoryPool mpool = mpool_create(MAXSIZE, MAXNUM, 1, test_constructor);
	if(constructor_call != MAXNUM) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	int i=0;
	void *ptr;
	char checkdata[MAXSIZE];
	memset(checkdata, 1, MAXSIZE);
	for(i = 0;i < MAXNUM; i ++) {
		ptr=mpool_malloc(mpool, MAXSIZE);
		if(memcmp(checkdata, ptr, MAXSIZE) != 0) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	mpool_delete(mpool, test_destructor);
	if(destructor_call != MAXNUM) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if(test_mpool_malloc_single_thread()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_malloc_multi_thread()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_malloc_getnext_check()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_malloc_initialize_chech()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	printf( "(%d)Finish to test!!\n", __LINE__);
	return 0;
}
