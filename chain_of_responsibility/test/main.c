#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chain_of_responsibility.h"

#define ERRLOG(...) printf("####<%s:%d>", __FUNCTION__, __LINE__);printf(__VA_ARGS__);

enum {
	ID_TEST1,
	ID_TEST2,
	ID_TEST3
};

int ctx_g;

#define MAX (4)
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
	if(cor_add_function(ID_TEST1, NULL, NULL) != COR_FAILED) {
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
		if(cor_add_function(name, funcs[i], &ctx_g) == COR_FAILED) {
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
		if(cor_add_function(name, funcs[i], &ctx_g) == COR_FAILED) {
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
		if(cor_add_function(name, funcs[i], &ctx_g) == COR_FAILED) {
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

int test_clear() {
	cor_clear();
	int values[MAX];
	int values_ok[MAX];
	int name1 =ID_TEST1;
	int name2 =ID_TEST2;
	int name3 =ID_TEST3;
	memset(values, 0, sizeof(values));
	memset(values_ok, 0, sizeof(values_ok));
	cor_call(name1, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %d\n", name1);
		return -1;
	}

	cor_call(name2, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %d\n", name2);
		return -1;
	}

	cor_call(name3, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %d\n", name3);
		return -1;
	}
	return 0;
}

int main() {
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

	if(test_clear()) {
		ERRLOG("test_clear test case failed!!!\n");
		return -1;
	}

	printf("All test is success!!\n");
	return 0;
}
