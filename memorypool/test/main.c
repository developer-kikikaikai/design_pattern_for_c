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

static int call_cnt_g=0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static void set_cnt() {
	pthread_mutex_lock(&lock);
	call_cnt_g++;
	pthread_mutex_unlock(&lock);
}
static int get_cnt() {
	int cnt=0;
	pthread_mutex_lock(&lock);
	cnt=call_cnt_g;
	pthread_mutex_unlock(&lock);
	return cnt;
}

void * test_mpool_get_thread(void *arg) {
	mpool_result_t * data = (mpool_result_t *)arg;
	char buf[MAXSIZE]={1};
	void *mem_list[MAXNUM];
	int i=0;
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = mpool_get(data->this);
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
	if(mpool_get_usedcnt(data->this) != MAXNUM) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}
	}
	printf("finish!\n");

	if(data->is_multithread) {
		set_cnt();
		while(get_cnt() != 2) {
			sleep(1);
		}
	}
	void * local = NULL;
	local = mpool_get(data->this);
	if(local != NULL) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}
//	printf("0x%x\n", local);
	//no release tmp data
	for(i=0;i<MAXNUM;i++) {
		memset(mem_list[i], 0, MAXSIZE);
	}

	//release data
	mpool_release(data->this, mem_list[10]);
	mpool_release(data->this, mem_list[50]);

	local = mpool_get(data->this);
	void *local2 = mpool_get(data->this);
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
		mpool_release(data->this, mem_list[i]);
		if(!data->is_multithread) {
			if(mpool_get_usedcnt(data->this) != MAXNUM-(i+1)) {
				printf( "###(%d)failed\n", __LINE__);
				goto end;
			}
		}
	}

	/*try to get/release many times*/
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = mpool_get(data->this);
	}
	int index1, index2;
	for(i=0;i<MAXNUM*MAXNUM;i++) {
		index1 = i%10;
		index2 = i%50+50;
		mpool_release(data->this, mem_list[index1]);
		mpool_release(data->this, mem_list[index2]);
		mem_list[index1]=mpool_get(data->this);
		mem_list[index2]=mpool_get(data->this);
		if(!mem_list[index1] || !mem_list[index2]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}

	//exit
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	data->result = 0;
end:
	if(data->is_multithread) {pthread_exit(NULL);}
	return NULL;
}

int test_mpool_get_single_thread() {
	mpool_result_t result={NULL, 0, MAXSIZE_LEN, -1};
	result.this = mpool_create(MAXSIZE, MAXNUM, result.is_multithread, NULL, NULL);

	test_mpool_get_thread(&result);
	mpool_delete(result.this, NULL);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	return 0;
}

int test_mpool_get_multi_thread() {
	mpool_result_t result={NULL, 0, MAXSIZE, -1};
	mpool_result_t result2;
	result.maxsize = MAXSIZE_LEN;
	result.is_multithread = 1;
	result.this = mpool_create(MAXSIZE, MAXNUM*2, result.is_multithread, NULL, NULL);
	memcpy(&result2, &result, sizeof(result));

	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, test_mpool_get_thread, &result);
	pthread_create(&tid2, NULL, test_mpool_get_thread, &result2);
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

void * get_thread(void *arg) {
	MemoryPool mpool = (MemoryPool)arg;
	int i=0;
	for(i=0;i<MAXNUM/2;i++) {
		mpool_get(mpool);
	}
	pthread_exit(NULL);
	return NULL;
}

int test_mpool_get_getnext_check() {
	MemoryPool mpool = mpool_create(MAXSIZE, MAXNUM, 1, NULL, NULL);
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
	mem_list[cnt++] = mpool_get(mpool);
	mem_list[cnt++] = mpool_get(mpool);
	mem_list[cnt++] = mpool_get(mpool);

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

	// check after release
	mpool_release(mpool, mem_list[1]);
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
		mem_list[cnt] = mpool_get(mpool);
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
	//release check
	for(cnt2=MAXNUM-1; cnt2!=0; cnt2--) {
		mpool_release(mpool, mem_list[cnt2]);
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

	pthread_create(&tid1, NULL, get_thread, mpool);
	pthread_create(&tid2, NULL, get_thread, mpool);
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
void test_constructor(void *ptr, void *param) {
	int * data = (int *)param;
	(*data)++;
	memset(ptr, 1, MAXSIZE);
}

static int destructor_call=0;
void test_destructor(void *ptr) {
	destructor_call++;
}

int test_mpool_get_initialize_chech() {
	MemoryPool mpool = mpool_create(MAXSIZE, MAXNUM, 1, test_constructor, &constructor_call);
	if(constructor_call != MAXNUM) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	int i=0;
	void *ptr;
	char checkdata[MAXSIZE];
	memset(checkdata, 1, MAXSIZE);
	for(i = 0;i < MAXNUM; i ++) {
		ptr=mpool_get(mpool);
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
	if(test_mpool_get_single_thread()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_get_multi_thread()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_get_getnext_check()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	if(test_mpool_get_initialize_chech()) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	printf( "(%d)Finish to test!!\n", __LINE__);
	return 0;
}
