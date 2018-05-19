#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chain_of_responsibility.h"

#define ERRLOG(...) printf("####<%s:%d>", __FUNCTION__, __LINE__);printf(__VA_ARGS__);

#define MAX (4)
static int all_cnt(int *values) {
	int i=0;
	int ret = 0;
	for(i=0;i<MAX;i++) {
		ret += values[i];
	}
	return ret;
}

cor_result_e function_1st(void *arg) {
	int *values = (int *)arg;
	values[0]+=all_cnt(values)+1;
	return CoR_GONEXT;
}

cor_result_e function_2nd(void *arg) {
	int *values = (int *)arg;
	values[1]+=all_cnt(values)+1;
	return CoR_GONEXT;
}

cor_result_e function_3rd(void *arg) {
	int *values = (int *)arg;
	values[2]+=all_cnt(values)+1;
	return CoR_GONEXT;
}

cor_result_e function_3rd_stop(void *arg) {
	int *values = (int *)arg;
	values[2]+=all_cnt(values)+1;
	return CoR_RETURN;
}

cor_result_e function_4rt(void *arg) {
	int *values = (int *)arg;
	values[3]+=all_cnt(values)+1;
	return CoR_GONEXT;
}

static int test_failsate() {
	//call clear 1st
	cor_clear();

	//check add function
	if(cor_add_function("test", NULL) != COR_FAILED) {
		ERRLOG("can't escape NULL\n");
		return -1;
	}

	if(cor_add_function(NULL, function_1st) != COR_FAILED) {
		ERRLOG("can't escape NULL string\n");
		return -1;
	}

	if(cor_add_function("", function_1st) != COR_FAILED) {
		ERRLOG("can't escape empty\n");
		return -1;
	}

	///only check not dead
	cor_call(NULL, NULL);
	cor_call("", NULL);

	///only check not dead
	cor_remove_function("test", NULL);
	cor_remove_function(NULL, function_1st);
	cor_remove_function("", function_1st);
	return 0;
}

int test_add_call_onename() {
	int values[MAX];
	char *name ="test1";
	chain_func funcs[MAX]={function_1st, function_2nd, function_3rd, function_4rt};

	int i=0;
	for(i=0;i<MAX;i++) {
		if(cor_add_function(name, funcs[i]) == COR_FAILED) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	cor_call(name, values);
	if(values[0] != 1 || values[1] != 2 || values[2] != 4 || values[3] != 8 ) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	return 0;
}

int test_add_call_with_stop() {
	int values[MAX];
	char *name ="test2";
	chain_func funcs[MAX]={function_1st, function_2nd, function_3rd_stop, function_4rt};

	int i=0;
	for(i=0;i<MAX;i++) {
		if(cor_add_function(name, funcs[i]) == COR_FAILED) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	cor_call(name, values);
	if(values[0] != 1 || values[1] != 2 || values[2] != 4 || values[3] != 0 ) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	return 0;
}

int test_remove() {
	int values[MAX];
	char *name ="test3";

	chain_func funcs[]={function_1st, function_2nd, function_1st, function_3rd, function_1st, function_4rt};
	int i=0;
	for(i=0;i<sizeof(funcs)/sizeof(funcs[0]);i++) {
		if(cor_add_function(name, funcs[i]) == COR_FAILED) {
			ERRLOG("add %d func failed\n", i);
			return -1;
		}
	}

	memset(values, 0, sizeof(values));

	//try call!
	cor_call(name, values);
	if(values[0] != 21 || values[1] != 2 || values[2] != 8 || values[3] != 32 ) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}

	//remove function_1st
	cor_remove_function(name, function_1st);

	//only function_2nd, function_3rd, function_4rt
	memset(values, 0, sizeof(values));
	cor_call(name, values);
	if(values[0] != 0 || values[1] != 1 || values[2] != 2 || values[3] != 4 ) {
		ERRLOG("failed to 4 function order, [%d,%d,%d,%d]\n", values[0],values[1], values[2], values[3]);
		return -1;
	}
	return 0;
}

int test_clear() {
	cor_clear();
	int values[MAX];
	int values_ok[MAX];
	char *name1 ="test1";
	char *name2 ="test2";
	char *name3 ="test3";
	memset(values, 0, sizeof(values));
	memset(values_ok, 0, sizeof(values_ok));
	cor_call(name1, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %s\n", name1);
		return -1;
	}

	cor_call(name2, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %s\n", name2);
		return -1;
	}

	cor_call(name3, values);
	if(memcmp(values, values_ok, sizeof(values)) != 0) {
		ERRLOG("failed to clear %s\n", name3);
		return -1;
	}
	return 0;
}

int main() {
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
