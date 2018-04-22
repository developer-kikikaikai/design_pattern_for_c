#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "flyweight.h"

#define PRINTLOG(...) printf("%s(%d)", __FUNCTION__, __LINE__);printf(__VA_ARGS__);
#define ENTERLOG printf("%s\n", __FUNCTION__);

struct test1 {
	int data1;
	int data2;
};

#define DEFAULT_TEST1_VAL1 (10)
#define DEFAULT_TEST1_VAL2 (10)

void test1_constructor(void *src) {
ENTERLOG
	struct test1 * data = (struct test1 *)src;
	data->data1=DEFAULT_TEST1_VAL1;
	data->data2=DEFAULT_TEST1_VAL2;
}

int test1_get_setval(int val) {
	return val-5;
}

int test1_setter_data1(void *src, size_t srcsize, void *dist) {
ENTERLOG
	struct test1 * data = (struct test1 *)src;
	int *data2=(void *)dist;
	data->data2=test1_get_setval(*data2);
	PRINTLOG("data1:%d. data2:%d\n", data->data1, data->data2);
}

static int call_destructor=0;
void test1_destructor(void *src) {
ENTERLOG
	call_destructor++;
	struct test1 * data = (struct test1 *)src;
	PRINTLOG("data1:%d. data2:%d\n", data->data1, data->data2);
}

struct test2 {
	char str[30];
};

int test2_setter_data1(void *src, size_t srcsize, void *dist) {
ENTERLOG
	struct test2 * data = (struct test2 *)src;
	char * input = (char *) dist;

	snprintf(data->str, sizeof(data->str), "%s", input);
	PRINTLOG("str:%s\n", input);
}

struct test3 {
	unsigned long data1;
	unsigned long data2;
};

int test3_setter_data1(void *src, size_t srcsize, void *dist) {
ENTERLOG
	struct test3 * data1=(struct test3 *)src;
	unsigned long *data = (unsigned long *)dist;
	data1->data1=*data;
}

void * test_set(void *handle) {

ENTERLOG
	int *id=(int *)handle;
	flyweight_set(*id, id, NULL);
	return NULL;
}

int main() {
	//only store 5 registore
	flyweight_set_storagemax(5);

	int ids[6], i=0;
	struct flyweight_init_s test1_methods={test1_constructor, test1_setter_data1, test1_destructor};
	struct flyweight_init_s test2_methods={NULL, test2_setter_data1, NULL};
	ids[i++] = flyweight_register_class(sizeof(struct test1), 0, &test1_methods);
	ids[i++] = flyweight_register_class(sizeof(struct test1), 1, NULL);
	ids[i++] = flyweight_register_class(sizeof(struct test2), 0, &test2_methods);
	ids[i++] = flyweight_register_class(sizeof(struct test2), 1, &test2_methods);
	ids[i++] = flyweight_register_class(sizeof(struct test3), 0, NULL);
	ids[i++] = flyweight_register_class(sizeof(struct test3), 1, NULL);
	for(i=0;i<5;i++) {
		if(ids[i] != i) {
			PRINTLOG("failed to register id\n");
			return 1;
		}
	}
	if(0<ids[i]) {
		PRINTLOG("failed to check max\n");
		return 1;
	}

	//get instance of test1
	struct test1 *get1, *get2;
	get1 = (struct test1 *)flyweight_get(0);
	get2 = (struct test1 *)flyweight_get(0);
	if(get1 == NULL || get1 != get2) {
		PRINTLOG("failed to reuse instance test1\n");
		return 1;
	}

	if(get1->data1!=DEFAULT_TEST1_VAL1 || get1->data2!=DEFAULT_TEST1_VAL2) {
		PRINTLOG("failed to call constructor test1\n");
		return 1;
	}

	int data=10;
	PRINTLOG("set test1\n");
	PRINTLOG("set test1, result=%d\n", flyweight_set(0, &data, NULL));
	if(get1->data1!=DEFAULT_TEST1_VAL1 || get1->data2!=test1_get_setval(data)) {
		PRINTLOG("failed to call setter test1\n");
		return 1;
	}

	get1 = (struct test1 *)flyweight_get(1);
	if(get1 == get2) {
		PRINTLOG("failed to separate instance\n");
		return -1;
	}

	if(get1->data1 || get1->data2) {
		PRINTLOG("Don't initialize data\n");
		return -1;
	}

	flyweight_unregister_class(0);
	if(call_destructor != 1) {
		PRINTLOG("failed to call destructor test1\n");
		return 1;
	}

	get1 = (struct test1 *)flyweight_get(0);
	if(get1 != NULL) {
		PRINTLOG("failed to call unregistor data\n");
		return 1;
	}
	int id = flyweight_register_class(sizeof(struct test3), 1, NULL);
	if(id != 0) {
		PRINTLOG("failed to reuse ID, new id=%d\n", id);
		return 1;
	}

	//check test2 str
	struct test2 * get_test2_1 = (struct test2 *)flyweight_get(3);
	if(get_test2_1 == NULL) {
		PRINTLOG("failed to get test2\n");
		return 1;
	}
	flyweight_set(3, "testdata", NULL);
	if(strcmp(get_test2_1->str, "testdata") != 0 ) {
		PRINTLOG("failed to set test2\n");
		return 1;
	}
	flyweight_unregister_class(3);
	if(0 <= flyweight_set(3, "testdata2", NULL) ) {
		PRINTLOG("failed to unregister test2\n");
		return 1;
	}
	id = flyweight_register_class(sizeof(struct test3), 1, NULL);
	if(id != 3) {
		PRINTLOG("failed to register test3\n");
		return -1;
	}

	struct test3 * get_test3 = flyweight_get(1);
	struct test3 * get_test3_2 = flyweight_get(3);
	struct test3 * get_test3_3 = flyweight_get(4);
	if(get_test3==get_test3_2 || get_test3_2 == get_test3_3 || get_test3 == get_test3_3) {
		PRINTLOG("failed to separate instance\n");
		return -1;
	}
	struct test3 test3_data={100,100};
	flyweight_set(3, &test3_data, NULL);
	get_test3_2 =flyweight_get(3);
	if(memcmp(get_test3_2, &test3_data, sizeof(struct test3)) != 0) {
		PRINTLOG("failed to call default setter\n");
		return -1;
	}

	test3_data.data1=1000;
	flyweight_set(3, &test3_data.data1, test3_setter_data1);
	if(memcmp(get_test3_2, &test3_data, sizeof(struct test3)) != 0) {
		PRINTLOG("failed to call test setter\n");
		return -1;
	}

	printf("flyweight_unregister_class\n");
	//check thread lock
	for(id=0;id<5;id++) {
		flyweight_unregister_class(id);
	}

	id = flyweight_register_class(sizeof(struct test1), 0, &test1_methods);
	flyweight_lock(id);
	pthread_t tid1;
	pthread_create(&tid1, NULL, test_set, &id);
	for(i=0;i<10;i++) {
		get1 = (struct test1 *)flyweight_get(id);
		if(get1->data1!=DEFAULT_TEST1_VAL1 || get1->data2!=DEFAULT_TEST1_VAL2) {
			PRINTLOG("failed to call setter test1\n");
			return 1;
		}
	}
	flyweight_unlock(id);

	pthread_join(tid1, NULL);
	//after unlock id, called test_set
	get1 = (struct test1 *)flyweight_get(id);
	if(get1->data1!=DEFAULT_TEST1_VAL1 || get1->data2!=test1_get_setval(id)) {
		PRINTLOG("failed to lock call setter test1\n");
		return 1;
	}
	PRINTLOG("All is success!!\n");
	return 0;
}
