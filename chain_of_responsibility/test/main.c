#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "chain_of_responsibility.h"

#define ERRLOG(...) printf("####<%s:%d>", __FUNCTION__, __LINE__);printf(__VA_ARGS__);

enum {
	ID_TEST1,
	ID_TEST2,
	ID_TEST3,
	ID_TEST4,
	MAX
};

int ctx_g;
static CorManager reentrant_manager;
static ChainElementPart reentrant_part;

static cor_result_e remove_self(void *arg, void *ctx) {
	(*(int *)arg)++;
	cor_manager_remove_chain_element_part(reentrant_manager, ID_TEST1, reentrant_part);
	return CoR_GONEXT;
}

static cor_result_e count_call(void *arg, void *ctx) {
	(*(int *)arg)++;
	return CoR_GONEXT;
}

static int all_cnt(int *values) {
	int i=0;
	int ret = 0;
	for(i=0;i<MAX;i++) {
		ret += values[i];
	}
	return ret;
}

cor_result_e function_1st(void *arg, void *ctx) {
	int *values = (int *)arg;
	values[0]+=all_cnt(values)+1;
	int *ctx_val = (int *)ctx;
	(*ctx_val)++;
	return CoR_GONEXT;
}

cor_result_e function_2nd(void *arg, void *ctx) {
	int *values = (int *)arg;
	values[1]+=all_cnt(values)+1;
	int *ctx_val = (int *)ctx;
	(*ctx_val)++;
	return CoR_GONEXT;
}

cor_result_e function_3rd(void *arg, void *ctx) {
	int *values = (int *)arg;
	values[2]+=all_cnt(values)+1;
	int *ctx_val = (int *)ctx;
	(*ctx_val)++;
	return CoR_GONEXT;
}

cor_result_e function_3rd_stop(void *arg, void *ctx) {
	int *values = (int *)arg;
	values[2]+=all_cnt(values)+1;
	int *ctx_val = (int *)ctx;
	(*ctx_val)++;
	return CoR_RETURN;
}

cor_result_e function_4th(void *arg, void *ctx) {
	int *values = (int *)arg;
	values[3]+=all_cnt(values)+1;
	int *ctx_val = (int *)ctx;
	(*ctx_val)++;
	return CoR_GONEXT;
}

static int test_failsate() {
	//call clear 1st
	cor_clear();

	//check add function
	if(cor_add_function(ID_TEST1, NULL, NULL)) {
		ERRLOG("can't escape NULL\n");
		return -1;
	}

	///only check not dead
	cor_call(ID_TEST1, NULL);

	///only check not dead
	cor_remove_function(ID_TEST1, NULL);
	return 0;
}

int test_add_call_onename() {
	int values[MAX];
	int name =ID_TEST1;
	chain_func funcs[MAX]={function_1st, function_2nd, function_3rd, function_4th};

	int i=0;
	ctx_g=0;
	for(i=0;i<MAX;i++) {
		if(!cor_add_function(name, funcs[i], &ctx_g)) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	cor_call(name, values);
	if(values[0] != 1 || values[1] != 2 || values[2] != 4 || values[3] != 8 || ctx_g != 4) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	return 0;
}

int test_add_call_with_stop() {
	int values[MAX];
	int name = ID_TEST2;
	chain_func funcs[MAX]={function_1st, function_2nd, function_3rd_stop, function_4th};

	int i=0;
	for(i=0;i<MAX;i++) {
		if(!cor_add_function(name, funcs[i], &ctx_g)) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	ctx_g=0;
	cor_call(name, values);
	if(values[0] != 1 || values[1] != 2 || values[2] != 4 || values[3] != 0 || ctx_g != 3) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d(cnt=%d)]\n", values[0],values[1], values[2], values[3], ctx_g);
		return -1;
	}

	return 0;
}

int test_remove() {
	int values[MAX];
	int name = ID_TEST3;

	chain_func funcs[]={function_1st, function_2nd, function_1st, function_3rd, function_1st, function_4th};
	int i=0;
	for(i=0;i<sizeof(funcs)/sizeof(funcs[0]);i++) {
		if(!cor_add_function(name, funcs[i], &ctx_g)) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	ctx_g=0;
	cor_call(name, values);
	if(values[0] != 21 || values[1] != 2 || values[2] != 8 || values[3] != 32 || ctx_g != 6) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	//remove function_1st
	cor_remove_function(name, function_1st);

	//only function_2nd, function_3rd, function_4th
	memset(values, 0, sizeof(values));
	ctx_g = 0;
	cor_call(name, values);
	if(values[0] != 0 || values[1] != 1 || values[2] != 2 || values[3] != 4 || ctx_g != 3) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}
	return 0;
}

#define FUNC_SIZE (6)
int test_remove_element() {
	int values[MAX];
	int name = ID_TEST4;
	ChainElementPart elements[FUNC_SIZE];
	chain_func funcs[FUNC_SIZE]={function_1st, function_2nd, function_1st, function_3rd, function_1st, function_4th};

	int i=0;
	for(i=0;i<sizeof(funcs)/sizeof(funcs[0]);i++) {
		elements[i] = cor_add_function(name, funcs[i], &ctx_g);
		if(!elements[i]) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	ctx_g=0;
	cor_call(name, values);
	if(values[0] != 21 || values[1] != 2 || values[2] != 8 || values[3] != 32 || ctx_g != 6) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	//remove function_1st element 1, function_3rd
	cor_remove_chain_element_part(name, elements[2]);
	cor_remove_chain_element_part(name, elements[3]);

	//call function_1stx2, function_2nd and function_4th
	memset(values, 0, sizeof(values));
	ctx_g = 0;
	cor_call(name, values);
	if(values[0] != 5 || values[1] != 2 || values[2] != 0 || values[3] != 8 || ctx_g != 4) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}
	return 0;
}

int test_clear() {
	cor_clear();
	int values[MAX];
	int values_ok[MAX];
	memset(values, 0, sizeof(values));
	memset(values_ok, 0, sizeof(values_ok));
	int i=0;
	for( i = 0; i < MAX; i ++) {
		cor_call(i, values);
		if(memcmp(values, values_ok, sizeof(values)) != 0) {
			ERRLOG("failed to clear %d\n", i);
			return -1;
		}
	}

	return 0;
}

static int test_independent_manager_and_reentrant_callback() {
	CorManager other_manager;
	int count = 0;
	reentrant_manager = cor_manager_new(1);
	other_manager = cor_manager_new(1);
	if(!reentrant_manager || !other_manager) {
		cor_manager_free(reentrant_manager);
		cor_manager_free(other_manager);
		return -1;
	}

	reentrant_part = cor_manager_add_function(reentrant_manager, ID_TEST1, remove_self, NULL);
	if(!reentrant_part || !cor_manager_add_function(reentrant_manager, ID_TEST1, count_call, NULL) ||
	   !cor_manager_add_function(other_manager, ID_TEST1, count_call, NULL)) {
		cor_manager_free(reentrant_manager);
		cor_manager_free(other_manager);
		return -1;
	}

	cor_manager_call(reentrant_manager, ID_TEST1, &count);
	cor_manager_call(reentrant_manager, ID_TEST1, &count);
	if(count != 3) {
		cor_manager_free(reentrant_manager);
		cor_manager_free(other_manager);
		return -1;
	}

	count = 0;
	cor_manager_call(other_manager, ID_TEST1, &count);
	cor_manager_free(reentrant_manager);
	cor_manager_free(other_manager);
	reentrant_manager = NULL;
	return count == 1 ? 0 : -1;
}

#define INIT_THREAD_COUNT 8
#define INIT_ADD_COUNT 50
static void *add_from_thread(void *arg) {
	int i;
	for(i = 0; i < INIT_ADD_COUNT; i++) {
		if(!cor_add_function(ID_TEST1, count_call, NULL)) {
			return (void *)1;
		}
	}
	return NULL;
}

static int test_concurrent_default_initialization() {
	pthread_t threads[INIT_THREAD_COUNT];
	int i;
	int count = 0;
	cor_clear();
	cor_set_threadsafe(1);

	for(i = 0; i < INIT_THREAD_COUNT; i++) {
		if(pthread_create(&threads[i], NULL, add_from_thread, NULL)) {
			return -1;
		}
	}
	for(i = 0; i < INIT_THREAD_COUNT; i++) {
		void *result = NULL;
		pthread_join(threads[i], &result);
		if(result) {
			return -1;
		}
	}

	cor_call(ID_TEST1, &count);
	cor_clear();
	return count == INIT_THREAD_COUNT * INIT_ADD_COUNT ? 0 : -1;
}

int main(int argc, char *argv[]) {
	cor_set_threadsafe(1);

	if(test_failsate()) {
		ERRLOG("test_failsate test case failed!!!\n");
		return -1;
	}

	if(test_add_call_onename()) {
		ERRLOG("test_add_call_onename test case failed!!!\n");
		return -1;
	}

	if(test_add_call_with_stop()) {
		ERRLOG("test_add_call_with_stop test case failed!!!\n");
		return -1;
	}

	if(test_remove()) {
		ERRLOG("test_remove test case failed!!!\n");
		return -1;
	}

	if(test_remove_element()) {
		ERRLOG("test_remove test case failed!!!\n");
		return -1;
	}

	if(test_clear()) {
		ERRLOG("test_clear test case failed!!!\n");
		return -1;
	}

	if(test_independent_manager_and_reentrant_callback()) {
		ERRLOG("test_independent_manager_and_reentrant_callback failed!!!\n");
		return -1;
	}

	if(test_concurrent_default_initialization()) {
		ERRLOG("test_concurrent_default_initialization failed!!!\n");
		return -1;
	}

	printf("All test is success!!\n");
	return 0;
}
